/* app25-multiclasse-inferencia — o ESP32 mede, pergunta para a nuvem e obedece.

   As features são as mesmas do app17-7 (janela de 100 amostras @ 100 Hz), mas
   aqui o dispositivo não rotula nada: publica a janela em
   FIAPIoT/motor/multiclasse e a classe prevista volta em
   FIAPIoT/motor/multiclasse/cmd. O LED pisca o índice do que a NUVEM respondeu.

   Repare no que não existe aqui: nenhum if sobre vibração ou inclinação, nenhum
   limiar — e nenhum botão. O app17-7 tinha botões porque era um GERADOR DE
   DATASET, onde um humano rotulava cada janela e a coleta parava em 30. Este é
   um MONITOR de condição: roda sem parar, e quem rotula é o modelo.

   A função atualizarLedClasse() é a mesma do app17-7. Só mudou de onde vem o
   índice: antes era o botão 18, agora é a resposta da nuvem.
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
#include <time.h>
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

/* ---- Relógio (NTP) ---- */
const char* NTP_SERVER_1 = "pool.ntp.org";
const char* NTP_SERVER_2 = "time.google.com";
uint64_t epochBaseMs        = 0;
uint32_t millisNaSync       = 0;
bool     relogioSincronizado = false;

/* ---- Pinos ---- */
#define SDA_PIN      22
#define SCL_PIN      23
#define LED_PIN       4   // LED externo: pisca N vezes = índice da classe prevista
#define LED_ONBOARD   2   // LED onboard: aceso = conectado ao broker

/* ---- Sensor (MPU6050 ou MPU6500) ---- */
#define MPU_TYPE MPU6500
MPU_TYPE mpu;

calData calib = { 0 };

/* ---- As classes, na mesma ordem do app17-7 ----
   A ordem aqui não precisa bater com a do modelo (modelo.classes_ é
   alfabética): a nuvem manda o NOME, e nós procuramos o nome nesta lista só
   para saber quantas piscadas dar. */
const char* SEQUENCIA[] = { "operando", "inclinado_frente", "inclinado_tras", "anomalia" };
const int   N_CLASSES = 4;

/* ---- A resposta da nuvem ----
   -1 = ainda não respondeu (LED apagado). */
int classePrevista = -1;

/* ---- LED de classe: N piscadas curtas + pausa longa, repetindo ---- */
const uint32_t LED_ON_MS    = 150;
const uint32_t LED_OFF_MS   = 200;
const uint32_t LED_PAUSA_MS = 1200;

int      ledPiscadasFeitas = 0;
bool     ledClasseAceso    = false;
uint32_t ledUltimaMudanca  = 0;

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
void sincronizarRelogio();
uint64_t agoraEpochMs();
void conectarMQTT();
void receberComando(char* topico, byte* conteudo, unsigned int tamanho);
void atualizarLedClasse();
void publicarJanela(uint64_t ts_epoch_ms,
                    float mx, float my, float mz,
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

  pinMode(LED_PIN,     OUTPUT);
  pinMode(LED_ONBOARD, OUTPUT);
  digitalWrite(LED_PIN,     LOW);
  digitalWrite(LED_ONBOARD, LOW);
  ledUltimaMudanca = millis();

  conectarWiFi();
  sincronizarRelogio();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(receberComando);   // é aqui que a resposta da nuvem entra
  mqttClient.setKeepAlive(60);
  mqttClient.setSocketTimeout(30);
  mqttClient.setBufferSize(512);

  Serial.println("Sistema pronto. Uma janela por segundo, sem botão nenhum.");
  Serial.printf("  Publica em: %s\r\n", MQTT_PUB_TOPIC);
  Serial.printf("  Escuta em:  %s\r\n", MQTT_SUB_TOPIC);
  Serial.println("  LED externo: pisca N vezes = índice da classe prevista (1..4)");
  Serial.println("    1=operando  2=inclinado_frente  3=inclinado_tras  4=anomalia");
  Serial.println("  LED apagado = a nuvem ainda não respondeu\r\n");
}

/* ============================== LOOP =============================== */
void loop() {
  if (!mqttClient.connected()) {
    conectarMQTT();
  }
  mqttClient.loop();

  digitalWrite(LED_ONBOARD, mqttClient.connected() ? HIGH : LOW);
  atualizarLedClasse();

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
      uint64_t ts_epoch_ms = agoraEpochMs();

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

      publicarJanela(ts_epoch_ms, mx, my, mz, sx, sy, sz, stdMag, p2p);

      indice = 0;
    }
  }
}

/* ---- LED de classe: N piscadas curtas + pausa longa, repetindo sempre ----
   Idêntica à do app17-7. A única diferença é o índice: vem de classePrevista
   (a nuvem), não de indiceClasse (o botão). */
void atualizarLedClasse() {
  if (classePrevista < 0) {          // ainda sem resposta
    digitalWrite(LED_PIN, LOW);
    ledClasseAceso = false;
    ledPiscadasFeitas = 0;
    return;
  }

  int totalPiscadas = classePrevista + 1;
  uint32_t agora = millis();

  if (ledClasseAceso) {
    if (agora - ledUltimaMudanca >= LED_ON_MS) {
      digitalWrite(LED_PIN, LOW);
      ledClasseAceso = false;
      ledUltimaMudanca = agora;
      ledPiscadasFeitas++;
    }
  } else {
    uint32_t intervalo = (ledPiscadasFeitas >= totalPiscadas) ? LED_PAUSA_MS : LED_OFF_MS;
    if (agora - ledUltimaMudanca >= intervalo) {
      if (ledPiscadasFeitas >= totalPiscadas) ledPiscadasFeitas = 0;
      digitalWrite(LED_PIN, HIGH);
      ledClasseAceso = true;
      ledUltimaMudanca = agora;
    }
  }
}

/* ---- WiFi ---- */
void conectarWiFi() {
  Serial.printf("Conectando ao WiFi %s", WIFI_SSID);
  // TxPower reduzido: evita brownout/reboot ao ligar o rádio nesta placa.
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_2dBm);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println("");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

/* ---- Relógio (NTP) ---- */
void sincronizarRelogio() {
  configTime(0, 0, NTP_SERVER_1, NTP_SERVER_2);   // UTC (offset 0)
  Serial.print("Sincronizando relógio via NTP");
  struct tm tm;
  for (int i = 0; i < 16 && !relogioSincronizado; i++) {
    if (getLocalTime(&tm, 500)) relogioSincronizado = true;
    else { Serial.print('.'); delay(500); }
  }
  if (relogioSincronizado) {
    epochBaseMs  = (uint64_t)time(nullptr) * 1000ULL;
    millisNaSync = millis();
    Serial.printf(" OK (%04d-%02d-%02d %02d:%02d:%02d UTC)\r\n",
                  tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                  tm.tm_hour, tm.tm_min, tm.tm_sec);
  } else {
    Serial.println(" FALHOU. ts_epoch_ms vira uptime.");
  }
}

uint64_t agoraEpochMs() {
  return epochBaseMs + (uint64_t)(millis() - millisNaSync);
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
   Vem o NOME da classe; procuramos na SEQUENCIA para saber quantas piscadas. */
void receberComando(char* topico, byte* conteudo, unsigned int tamanho) {
  // O payload MQTT não termina em '\0', por isso o String recebe o tamanho junto.
  String classe(conteudo, tamanho);
  classe.trim();

  int novaClasse = -1;
  for (int i = 0; i < N_CLASSES; i++) {
    if (classe == SEQUENCIA[i]) novaClasse = i;
  }

  if (novaClasse < 0) {
    Serial.printf("MODELO: classe desconhecida (%s)\r\n", classe.c_str());
    return;
  }

  // Reinicia o padrão de piscadas quando a classe muda, para a contagem não
  // sair pela metade.
  if (novaClasse != classePrevista) {
    ledPiscadasFeitas = 0;
    ledClasseAceso    = false;
    digitalWrite(LED_PIN, LOW);
    ledUltimaMudanca  = millis();
  }
  classePrevista = novaClasse;

  Serial.printf("MODELO: %s (%d piscadas)\r\n", classe.c_str(), classePrevista + 1);
}

/* ---- Publica a janela: só o que o modelo precisa ---- */
void publicarJanela(uint64_t ts_epoch_ms,
                    float mx, float my, float mz,
                    float sx, float sy, float sz,
                    float stdMag, float p2p) {
  JsonDocument doc;
  doc["device"]      = MQTT_CLIENT_ID;
  doc["ts_epoch_ms"] = ts_epoch_ms;
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
