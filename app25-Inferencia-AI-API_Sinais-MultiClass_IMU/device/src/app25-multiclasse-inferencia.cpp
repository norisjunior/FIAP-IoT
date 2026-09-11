/* app25-multiclasse-inferencia — o ESP32 mede, pergunta para a nuvem e obedece.

   As features são as mesmas do app17-7 (janela de 100 amostras @ 100 Hz), mas
   aqui o dispositivo não rotula nada: publica a janela em
   FIAPIoT/motor/multiclasse e a classe prevista volta em
   FIAPIoT/motor/multiclasse/cmd. Cada classe acende UMA saída:

       operando          LED azul      (4)
       inclinado_frente  LED amarelo  (21)
       inclinado_tras    LED vermelho (18)
       anomalia          buzzer       (19)

   Repare no que não existe aqui: nenhum if sobre vibração ou inclinação, nenhum
   limiar — e nenhum botão. O app17-7 tinha botões porque era um GERADOR DE
   DATASET, onde um humano rotulava cada janela e a coleta parava em 30. Este é
   um MONITOR de condição: roda sem parar, e quem rotula é o modelo.

   Os pinos 21 e 18 eram justamente os dois BOTÕES do app17-7 — de entrada do
   rótulo humano viraram saída do rótulo do modelo.

   Como a nuvem responde uma vez por segundo, a saída MANTÉM a última decisão
   recebida até a próxima chegar. Não há temporização nenhuma no firmware: a
   saída é a memória. Por isso quem acende os LEDs é a própria receberComando()
   — o loop não precisa cuidar disso.
*/
/*
PARA USAR NO WOKWI:
- Ajustar as credenciais WiFi e o IP do MQTT_SERVER (ou usar as linhas comentadas do Wokwi abaixo)
- Ajustar #define MPU_TYPE:
  - #define MPU_TYPE MPU6050
- Remover/comentar a linha `mpu.calibrateAccelGyro(&calib);` (trava no Wokwi, FIFO ausente)
- As classes de inclinação não têm equivalente fiel no simulador (não há como
  inclinar o MPU6050 do Wokwi); serve para testar o loop MQTT -> API -> MQTT.
*/

#include <Arduino.h>
#include "FastIMU.h"
#include <Wire.h>
#include <math.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

/* ---- Rede: use (A) Wokwi OU (B) ESP32 físico ---- */
// ---- (A) Wokwi (padrão) ----
// const char* WIFI_SSID     = "Wokwi-GUEST";
// const char* WIFI_PASSWORD = "";
// #define MQTT_SERVER "host.wokwi.internal"

// ---- (B) ESP32 físico ----
const char* WIFI_SSID     = "NorisIoT";
const char* WIFI_PASSWORD = "Secure10T";
#define MQTT_SERVER "172.16.10.101"   // IP da máquina com a IoT-platform

WiFiClient wifiClient;

/* ---- MQTT ---- */
#define MQTT_PORT      1883
#define MQTT_PUB_TOPIC "FIAPIoT/motor/multiclasse"       // a janela vai por aqui
#define MQTT_SUB_TOPIC "FIAPIoT/motor/multiclasse/cmd"   // a classe volta por aqui
#define MQTT_CLIENT_ID "IoTDevInferenciaMultiClasse001"
PubSubClient mqttClient(wifiClient);

/* ---- Pinos ---- */
#define SDA_PIN      22
#define SCL_PIN      23
/* Uma saída por classe: a que estiver ligada é a resposta da nuvem.
   Repare que 21 e 18 eram os BOTÕES do app17-7 — os pinos com que um humano
   rotulava agora mostram o rótulo que o modelo escolheu. */
#define LED_AZUL      4   // operando
#define LED_AMARELO  21   // inclinado_frente
#define LED_VERMELHO 18   // inclinado_tras
#define BUZZER       19   // anomalia
#define LED_ONBOARD   2   // LED onboard: aceso = conectado ao broker

/* ---- Sensor (MPU6050 ou MPU6500) ---- */
#define MPU_TYPE MPU6500
MPU_TYPE mpu;

calData calib = { 0 };

/* ---- Amostragem: 100 Hz, janela de 1 s (mesmo padrão do app17-7) ---- */
const int FS_HZ          = 100;
const int AMOSTRA_MS     = 1000 / FS_HZ;      // 10 ms
const int TAMANHO_JANELA = FS_HZ;             // 100 amostras = 1 s

float ax_buf[TAMANHO_JANELA];
float ay_buf[TAMANHO_JANELA];
float az_buf[TAMANHO_JANELA];
float mag_buf[TAMANHO_JANELA];

int indice = 0;
uint32_t tempoAnterior = 0;

/* ---- Protótipos ---- */
void conectarWiFi();
void conectarMQTT();
void receberComando(char* topico, byte* conteudo, unsigned int tamanho);
void apagarTodasAsSaidas();
void publicarJanela(float mx, float my, float mz,
                    float sx, float sy, float sz,
                    float stdMag, float p2p);

/* =========================== Features ===========================
   As 8 que o modelo recebe: mean_* (orientação), std_* e std_mag (vibração)
   e p2p_mag (pior caso da janela). */
float calcMean(float arr[], int n) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += arr[i];
  return soma / n;
}

float calcStd(float arr[], int n, float media) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += (arr[i] - media) * (arr[i] - media);
  return sqrt(soma / n);
}

void calcMagnitude(float axArr[], float ayArr[], float azArr[], float magArr[], int n) {
  for (int i = 0; i < n; i++) {
    magArr[i] = sqrt(axArr[i]*axArr[i] + ayArr[i]*ayArr[i] + azArr[i]*azArr[i]);
  }
}

float calcPtP(float arr[], int n) {
  float mn = arr[0], mx = arr[0];
  for (int i = 1; i < n; i++) {
    if (arr[i] < mn) mn = arr[i];
    if (arr[i] > mx) mx = arr[i];
  }
  return mx - mn;
}

/* ============================== SETUP ============================== */
void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);
  if (mpu.init(calib, 0x68) != 0) {
    Serial.println("Erro: MPU não encontrado");
    while (1);
  }
  mpu.setAccelRange(8);
  // Anti-aliasing p/ amostragem a 100 Hz (Nyquist 50 Hz): o filtro interno do
  // acelerômetro precisa cortar abaixo de 50 Hz. São duas chamadas porque
  // nenhuma delas sozinha cobre os dois chips:
  mpu.setAccelLPF(41);   // MPU6500: escreve ACCEL_CONFIG2 -> 41 Hz.
                         // MPU6050: o FastIMU não implementa, devolve -1 sem fazer nada.
  mpu.setGyroLPF(42);    // MPU6050: o DLPF é COMPARTILHADO, então é ESTA linha que
                         // filtra o acelerômetro (44 Hz). MPU6500: só o giroscópio.

  Serial.println("Deixe o motor na posicao inicial/de uso e nao o movimente durante a calibracao...");
  delay(2000);
  mpu.calibrateAccelGyro(&calib);   // habilite no ESP32 físico; trava no Wokwi (FIFO ausente)
  mpu.init(calib, 0x68);

  Serial.println("MPU iniciado");

  // As quatro saídas de classe, mais o LED da placa.
  pinMode(LED_AZUL,     OUTPUT);
  pinMode(LED_AMARELO,  OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(BUZZER,       OUTPUT);
  pinMode(LED_ONBOARD,  OUTPUT);

  apagarTodasAsSaidas();
  digitalWrite(LED_ONBOARD, LOW);

  // Teste de ligação: acende uma saída por vez, para você conferir se cada
  // componente está no pino certo ANTES de depender do modelo. Se o LED
  // amarelo não acender aqui, o problema é o fio — não a rede neural.
  // Os delay() aqui não atrapalham: o setup roda uma vez, antes do loop.
  Serial.println("Testando as saidas...");
  digitalWrite(LED_AZUL,     HIGH); delay(400); digitalWrite(LED_AZUL,     LOW);
  digitalWrite(LED_AMARELO,  HIGH); delay(400); digitalWrite(LED_AMARELO,  LOW);
  digitalWrite(LED_VERMELHO, HIGH); delay(400); digitalWrite(LED_VERMELHO, LOW);
  tone(BUZZER, 500, 250); noTone(BUZZER);   // buzzer passivo: precisa de frequencia

  conectarWiFi();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(receberComando);   // é aqui que a resposta da nuvem entra
  mqttClient.setKeepAlive(60);
  mqttClient.setSocketTimeout(30);
  mqttClient.setBufferSize(512);

  Serial.println("Sistema pronto. Uma janela por segundo, sem botão nenhum.");
  Serial.printf("  Publica em: %s\r\n", MQTT_PUB_TOPIC);
  Serial.printf("  Escuta em:  %s\r\n", MQTT_SUB_TOPIC);
  Serial.println("  Saidas (uma por classe):");
  Serial.printf("    GPIO %2d  LED azul     = operando\r\n",         LED_AZUL);
  Serial.printf("    GPIO %2d  LED amarelo  = inclinado_frente\r\n", LED_AMARELO);
  Serial.printf("    GPIO %2d  LED vermelho = inclinado_tras\r\n",   LED_VERMELHO);
  Serial.printf("    GPIO %2d  BUZZER       = anomalia\r\n",         BUZZER);
  Serial.println("  Tudo apagado = a nuvem ainda nao respondeu\r\n");
}

/* ============================== LOOP =============================== */
void loop() {
  if (!mqttClient.connected()) {
    conectarMQTT();
  }
  mqttClient.loop();

  digitalWrite(LED_ONBOARD, mqttClient.connected() ? HIGH : LOW);

  // --- Coleta IMU a 100 Hz, direto, sem esperar comando ---
  if (millis() - tempoAnterior >= AMOSTRA_MS) {
    // Avança em passos fixos de AMOSTRA_MS (e não "= millis()"): assim o atraso
    // de um ciclo não empurra o próximo e a taxa não escorrega abaixo de 100 Hz.
    tempoAnterior += AMOSTRA_MS;
    // Se ainda estamos mais de uma amostra atrasados (reconexão MQTT, publish
    // lento), não adianta amostrar em rajada para recuperar: as amostras sairiam
    // sem espaçamento real. Recomeça do agora.
    if (millis() - tempoAnterior >= AMOSTRA_MS) tempoAnterior = millis();

    AccelData accel;
    mpu.update();
    mpu.getAccel(&accel);
    ax_buf[indice] = accel.accelX;
    ay_buf[indice] = accel.accelY;
    az_buf[indice] = accel.accelZ;
    indice++;

    if (indice >= TAMANHO_JANELA) {
      float mx = calcMean(ax_buf, TAMANHO_JANELA);
      float my = calcMean(ay_buf, TAMANHO_JANELA);
      float mz = calcMean(az_buf, TAMANHO_JANELA);
      float sx = calcStd(ax_buf, TAMANHO_JANELA, mx);
      float sy = calcStd(ay_buf, TAMANHO_JANELA, my);
      float sz = calcStd(az_buf, TAMANHO_JANELA, mz);

      calcMagnitude(ax_buf, ay_buf, az_buf, mag_buf, TAMANHO_JANELA);
      float mMag   = calcMean(mag_buf, TAMANHO_JANELA);
      float stdMag = calcStd(mag_buf, TAMANHO_JANELA, mMag);
      float p2p    = calcPtP(mag_buf, TAMANHO_JANELA);

      publicarJanela(mx, my, mz, sx, sy, sz, stdMag, p2p);

      indice = 0;
    }
  }
}

/* ---- Apaga as quatro saídas de classe ----
   Chamada antes de acender a saída nova, para nunca ficarem duas ligadas. */
void apagarTodasAsSaidas() {
  digitalWrite(LED_AZUL,     LOW);
  digitalWrite(LED_AMARELO,  LOW);
  digitalWrite(LED_VERMELHO, LOW);
  digitalWrite(BUZZER,       LOW);
}

/* ---- WiFi ---- */
void conectarWiFi() {
  Serial.printf("Conectando ao WiFi %s", WIFI_SSID);
  // TxPower reduzido: evita brownout/reboot ao ligar o rádio nesta placa.
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_2dBm);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setSleep(false);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println("");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

/* ---- MQTT ---- */
void conectarMQTT() {
  while (!mqttClient.connected()) {
    Serial.printf("Conectando ao MQTT Broker %s...", MQTT_SERVER);
    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      Serial.println(" Conectado!");
      mqttClient.subscribe(MQTT_SUB_TOPIC);
      Serial.printf("Inscrito em: %s\r\n", MQTT_SUB_TOPIC);
    } else {
      Serial.printf(" Falha rc=%d. Tentando em 5s...\r\n", mqttClient.state());
      delay(5000);
    }
  }
}

/* ---- A resposta da nuvem chega aqui ----
   É a única função que mexe nas saídas. Chega o NOME da classe, comparamos
   com os quatro nomes possíveis e acendemos a saída daquele que casar.

   Não há nada de temporal aqui: acendeu, FICA aceso até chegar a próxima
   mensagem — e ela chega a cada segundo. A saída é a memória do dispositivo. */
void receberComando(char* topico, byte* conteudo, unsigned int tamanho) {
  // O payload MQTT não termina em '\0', por isso o String recebe o tamanho junto.
  String classe(conteudo, tamanho);
  classe.trim();

  // Mostra o que chegou ANTES de julgar: se o payload vier errado (um JSON
  // inteiro, por exemplo), é aqui que se vê o que o n8n publicou de fato.
  Serial.println("--- MQTT recebido ---");
  Serial.printf("  topico:  %s\r\n", topico);
  Serial.printf("  tamanho: %u bytes\r\n", tamanho);
  Serial.printf("  payload: \"%s\"\r\n", classe.c_str());

  // Passo 1: apaga tudo. Assim nunca ficam duas saídas ligadas ao mesmo tempo.
  apagarTodasAsSaidas();

  // Passo 2: acende SÓ a saída da classe que chegou.
  if (classe == "operando") {
    digitalWrite(LED_AZUL, HIGH);
    Serial.println("  MODELO:  operando         -> LED azul aceso");

  } else if (classe == "inclinado_frente") {
    digitalWrite(LED_AMARELO, HIGH);
    Serial.println("  MODELO:  inclinado_frente -> LED amarelo aceso");

  } else if (classe == "inclinado_tras") {
    digitalWrite(LED_VERMELHO, HIGH);
    Serial.println("  MODELO:  inclinado_tras   -> LED vermelho aceso");

  } else if (classe == "anomalia") {
    tone(BUZZER, 500, 250);   // bipe de 250 ms; a cada mensagem, um bipe
    Serial.println("  MODELO:  anomalia         -> BUZZER ligado");

  } else {
    // Nenhum nome casou. As saídas ficam todas apagadas, e isso é proposital:
    // apagado avisa que algo está errado, melhor do que manter a última classe
    // e parecer que o sistema continua funcionando.
    Serial.println("  MODELO:  classe DESCONHECIDA - tudo apagado");
    Serial.println("           esperado, sem aspas e sem JSON:");
    Serial.println("           operando | inclinado_frente | inclinado_tras | anomalia");
  }

  Serial.println("---------------------");
}

/* ---- Publica a janela: só o que o modelo precisa ----
   Não vai timestamp. No app17-7 ele existia porque as janelas iam para um
   BANCO, onde o tempo é o eixo e a ordem importa. Aqui a janela vale agora:
   é medida, classificada e respondida em menos de um segundo, e depois não
   serve para mais nada. Sem timestamp no payload, o NTP também sai. */
void publicarJanela(float mx, float my, float mz,
                    float sx, float sy, float sz,
                    float stdMag, float p2p) {
  JsonDocument doc;
  doc["device"]  = MQTT_CLIENT_ID;
  doc["mean_ax"] = serialized(String(mx, 3));
  doc["mean_ay"] = serialized(String(my, 3));
  doc["mean_az"] = serialized(String(mz, 3));
  doc["std_ax"]  = serialized(String(sx, 3));
  doc["std_ay"]  = serialized(String(sy, 3));
  doc["std_az"]  = serialized(String(sz, 3));
  doc["std_mag"] = serialized(String(stdMag, 3));
  doc["p2p_mag"] = serialized(String(p2p, 3));

  String buffer;
  serializeJson(doc, buffer);

  if (!mqttClient.publish(MQTT_PUB_TOPIC, buffer.c_str())) {
    Serial.println("MQTT: falha no envio");
  }
}
