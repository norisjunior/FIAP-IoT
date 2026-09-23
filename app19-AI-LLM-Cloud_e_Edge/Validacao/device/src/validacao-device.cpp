/* validacao-device — o dispositivo decide sozinho E conta o que decidiu.

   Este firmware é a soma das duas metades do app anterior: a inferência que
   roda na flash, com a Random Forest, mais o Wi-Fi e o MQTT que antes só
   existiam na versão com API.

   A diferença de papel é o ponto da aplicação:

     versão com API   o device PERGUNTA e obedece. Sem rede, ele não sabe nada.
     versão da borda  o device DECIDE e não fala com ninguém.
     este             o device DECIDE, acende a saída, e REPORTA o que decidiu.

   Repare no que não existe aqui: subscribe, callback, tópico de comando. O
   device não espera resposta de ninguém — ele já respondeu. A janela sai pelo
   MQTT junto com a predição da borda, e quem quiser conferir que confira.

   Do outro lado, o n8n pega essa mesma janela, pergunta à MLP que roda na
   nuvem, e guarda as duas respostas no PostgreSQL. Se a rede cair, o motor
   continua sendo monitorado: só a conferência para.

       operando          LED azul      (4)
       inclinado_frente  LED amarelo  (21)
       inclinado_tras    LED vermelho (18)
       anomalia          buzzer       (19)
       LED onboard        (2)   aceso = conectado ao broker
*/
/*
PARA USAR NO WOKWI:
- Ajustar as credenciais WiFi e o IP do MQTT_SERVER (ou usar as linhas comentadas do Wokwi abaixo)
- Ajustar #define MPU_TYPE:
  - #define MPU_TYPE MPU6050
- Remover/comentar a linha `mpu.calibrateAccelGyro(&calib);` (trava no Wokwi, FIFO ausente)
- As classes de INCLINAÇÃO saem no simulador: o MPU6050 do Wokwi tem controle
  de aceleração em X, Y e Z. Ajuste até o Serial mostrar mean_az perto de 0,91
  e mean_ax perto de ±0,42, que é o que 25 graus produzem.
- A ANOMALIA não sai. Ela é vibração, e com o controle parado as 100 amostras
  da janela ficam idênticas: std_* e p2p_mag dão zero.
*/

#include <Arduino.h>
#include "FastIMU.h"
#include <Wire.h>
#include <math.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

/* ---- O modelo, em dois arquivos gerados pelo Colab ----
   Os dois SEMPRE do mesmo treino: o scaler guarda a média e o desvio de cada
   feature, e os limiares da floresta estão nessa escala. Misturar o scaler de
   uma execução com a floresta de outra não dá erro de compilação — dá predição
   errada, em silêncio. */
#include "ModeloMotorScaler.hpp"   // Scaler::standardize()
#include "ModeloMotorRF.hpp"       // Eloquent::ML::Port::RandomForest

Eloquent::ML::Port::RandomForest modeloRF;

/* ---- Rede: use (A) Wokwi OU (B) ESP32 físico ---- */
// ---- (A) Wokwi (padrão) ----
// const char* WIFI_SSID     = "Wokwi-GUEST";
// const char* WIFI_PASSWORD = "";
// #define MQTT_SERVER "host.wokwi.internal"

// ---- (B) ESP32 físico ----
const char* WIFI_SSID     = "NorisIoT";
const char* WIFI_PASSWORD = "Secure10T";
#define MQTT_SERVER "172.16.10.101"   // IP da máquina com a plataforma

WiFiClient wifiClient;

/* ---- MQTT: só publicação ----
   Um tópico só, e é de saída. O tópico é NOVO, diferente do usado no app da
   nuvem: se fosse o mesmo, o fluxo daquele app reagiria a estas mensagens
   também e os dois se embolariam. */
#define MQTT_PORT      1883
#define MQTT_PUB_TOPIC "FIAPIoT/motor/validacao"
#define MQTT_CLIENT_ID "IoTDevValidacaoMotor001"
PubSubClient mqttClient(wifiClient);

/* ---- Pinos ---- */
#define SDA_PIN      22
#define SCL_PIN      23
#define LED_AZUL      4   // operando
#define LED_AMARELO  21   // inclinado_frente
#define LED_VERMELHO 18   // inclinado_tras
#define BUZZER       19   // anomalia
#define LED_ONBOARD   2   // aceso = conectado ao broker

/* ---- Sensor (MPU6050 ou MPU6500) ---- */
#define MPU_TYPE MPU6500
MPU_TYPE mpu;

calData calib = { 0 };

/* ---- Amostragem: 100 Hz, janela de 1 s (mesmo padrão da coleta) ---- */
const int FS_HZ          = 100;
const int AMOSTRA_MS     = 1000 / FS_HZ;      // 10 ms
const int TAMANHO_JANELA = FS_HZ;             // 100 amostras = 1 s

float ax_buf[TAMANHO_JANELA];
float ay_buf[TAMANHO_JANELA];
float az_buf[TAMANHO_JANELA];
float mag_buf[TAMANHO_JANELA];

int indice = 0;
uint32_t tempoAnterior = 0;

/* ---- Os nomes das classes, NA ORDEM DO scikit-learn ----
   O predict() devolve um número: 0, 1, 2 ou 3. A ordem é a de classes_, que o
   scikit-learn devolve sempre em ordem ALFABÉTICA. Por isso anomalia é o
   índice 0. Aqui o nome importa duas vezes: acende a saída certa e vai no JSON
   para a nuvem comparar. */
const char* NOMES_CLASSES[4] = { "anomalia", "inclinado_frente",
                                 "inclinado_tras", "operando" };

/* ---- Protótipos ---- */
int  classificarJanela(const float features[8]);
void acionarSaida(int classe);
void apagarTodasAsSaidas();
void publicarJanela(const float features[8], int classe);
void conectarWiFi();
void conectarMQTT();

/* =========================== Features ===========================
   As 8 que o modelo recebe: mean_* (orientação), std_* e std_mag (vibração)
   e p2p_mag (pior caso da janela). Idênticas às da coleta — é esse "idênticas"
   que faz o modelo valer aqui. */
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

  pinMode(LED_AZUL,     OUTPUT);
  pinMode(LED_AMARELO,  OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(BUZZER,       OUTPUT);
  pinMode(LED_ONBOARD,  OUTPUT);

  apagarTodasAsSaidas();
  digitalWrite(LED_ONBOARD, LOW);

  // Teste de ligação: acende uma saída por vez, para você conferir se cada
  // componente está no pino certo ANTES de depender do modelo.
  Serial.println("Testando as saidas...");
  digitalWrite(LED_AZUL,     HIGH); delay(400); digitalWrite(LED_AZUL,     LOW);
  digitalWrite(LED_AMARELO,  HIGH); delay(400); digitalWrite(LED_AMARELO,  LOW);
  digitalWrite(LED_VERMELHO, HIGH); delay(400); digitalWrite(LED_VERMELHO, LOW);
  tone(BUZZER, 500, 250); noTone(BUZZER);   // buzzer passivo: precisa de frequencia

  conectarWiFi();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setKeepAlive(60);
  mqttClient.setSocketTimeout(30);
  mqttClient.setBufferSize(512);
  // Sem setCallback: este firmware não escuta nada. Ele decide e conta.

  Serial.println("Sistema pronto. Decide aqui, e publica o que decidiu.");
  Serial.printf("  Publica em: %s\r\n\r\n", MQTT_PUB_TOPIC);
}

/* ============================== LOOP =============================== */
void loop() {
  if (!mqttClient.connected()) {
    conectarMQTT();
  }
  mqttClient.loop();

  digitalWrite(LED_ONBOARD, mqttClient.connected() ? HIGH : LOW);

  // --- Coleta IMU a 100 Hz ---
  if (millis() - tempoAnterior >= AMOSTRA_MS) {
    // Avança em passos fixos de AMOSTRA_MS (e não "= millis()"): assim o atraso
    // de um ciclo não empurra o próximo e a taxa não escorrega abaixo de 100 Hz.
    tempoAnterior += AMOSTRA_MS;
    // Se ainda estamos mais de uma amostra atrasados, não adianta amostrar em
    // rajada para recuperar: as amostras sairiam sem espaçamento real.
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

      // A ORDEM deste vetor é a ordem das colunas no treino.
      float features[8] = { mx, my, mz, sx, sy, sz, stdMag, p2p };

      // Primeiro decide e age. Publicar vem depois, e de propósito: se o
      // broker estiver fora do ar, o motor continua sendo monitorado.
      int classe = classificarJanela(features);
      acionarSaida(classe);
      publicarJanela(features, classe);

      indice = 0;
    }
  }
}

/* ---- A inferência: duas linhas, e o resto é impressão ----
   Esta função só RESPONDE: devolve o índice da classe e não mexe em pino
   nenhum. Quem acende é a acionarSaida(), chamada pelo loop. */
int classificarJanela(const float features[8]) {
  float padronizado[8];

  uint32_t t0 = micros();
  Scaler::standardize(features, padronizado);  // (valor - media) / desvio
  int classe = modeloRF.predict(padronizado);  // a floresta decide
  uint32_t duracao = micros() - t0;

  Serial.println("--- Janela fechada ---");
  Serial.printf("  mean_ax=%.3f  mean_ay=%.3f  mean_az=%.3f\r\n",
                features[0], features[1], features[2]);
  Serial.printf("  std_ax=%.3f   std_ay=%.3f   std_az=%.3f\r\n",
                features[3], features[4], features[5]);
  Serial.printf("  std_mag=%.3f  p2p_mag=%.3f\r\n", features[6], features[7]);

  if (classe >= 0 && classe < 4) {
    Serial.printf("  PREDIÇÃO:  %d -> %s\r\n", classe, NOMES_CLASSES[classe]);
  } else {
    Serial.printf("  PREDIÇÃO:  %d -> indice fora da faixa\r\n", classe);
  }
  Serial.printf("  Inferencia: %lu us\r\n", duracao);
  Serial.println("----------------------");

  return classe;
}

/* ---- Acende SÓ a saída da classe prevista ---- */
void acionarSaida(int classe) {
  apagarTodasAsSaidas();

  switch (classe) {
    case 0: tone(BUZZER, 500, 250);           break;   // anomalia: bipe de 250 ms
    case 1: digitalWrite(LED_AMARELO,  HIGH); break;   // inclinado_frente
    case 2: digitalWrite(LED_VERMELHO, HIGH); break;   // inclinado_tras
    case 3: digitalWrite(LED_AZUL,     HIGH); break;   // operando
    default: break;                                    // tudo apagado
  }
}

/* ---- Apaga as quatro saídas ---- */
void apagarTodasAsSaidas() {
  digitalWrite(LED_AZUL,     LOW);
  digitalWrite(LED_AMARELO,  LOW);
  digitalWrite(LED_VERMELHO, LOW);
  digitalWrite(BUZZER,       LOW);
}

/* ---- Publica a janela E a predição da borda ----
   As oito features vão para a nuvem rodar o modelo dela em cima dos MESMOS
   números, e predicao_borda vai junto para a comparação ser possível. Sem ela
   a nuvem responderia no vácuo: haveria uma predição só, e nada a conferir.

   Vai o nome da classe, não o índice: do outro lado, a API devolve o nome
   também, e comparar texto com texto dispensa qualquer tabela de tradução. */
void publicarJanela(const float features[8], int classe) {
  JsonDocument doc;
  doc["device"]         = MQTT_CLIENT_ID;
  doc["predicao_borda"] = (classe >= 0 && classe < 4) ? NOMES_CLASSES[classe] : "desconhecida";
  doc["mean_ax"] = serialized(String(features[0], 3));
  doc["mean_ay"] = serialized(String(features[1], 3));
  doc["mean_az"] = serialized(String(features[2], 3));
  doc["std_ax"]  = serialized(String(features[3], 3));
  doc["std_ay"]  = serialized(String(features[4], 3));
  doc["std_az"]  = serialized(String(features[5], 3));
  doc["std_mag"] = serialized(String(features[6], 3));
  doc["p2p_mag"] = serialized(String(features[7], 3));

  String buffer;
  serializeJson(doc, buffer);

  if (!mqttClient.publish(MQTT_PUB_TOPIC, buffer.c_str())) {
    Serial.println("MQTT: falha no envio");
  }
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

/* ---- MQTT ----
   Sem subscribe: não há tópico de comando para assinar. */
void conectarMQTT() {
  while (!mqttClient.connected()) {
    Serial.printf("Conectando ao MQTT Broker %s...", MQTT_SERVER);
    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      Serial.println(" Conectado!");
    } else {
      Serial.printf(" Falha rc=%d. Tentando em 5s...\r\n", mqttClient.state());
      delay(5000);
    }
  }
}
