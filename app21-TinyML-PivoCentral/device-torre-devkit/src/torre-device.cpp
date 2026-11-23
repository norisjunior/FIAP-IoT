/* torre-device — a rede neural (TFLite) decide na placa, publica a janela e manda na bomba do pivô */

#include <Arduino.h>
#include "FastIMU.h"
#include <Wire.h>
#include <math.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

/* ---- Modelo (gerado pelo Colab) ---- */
#include "ModeloMotorScaler.hpp"   // Scaler::standardize()
#include "ModeloMotorRede.hpp"     // a rede neural, em bytes (formato TFLite)
#include <MicroTFLite.h>           // o interpretador TFLite da placa

/* ---- A memória de trabalho da rede (tensor arena) ---- */
#define ARENA_SIZE (8 * 1024)
uint8_t tensorArena[ARENA_SIZE];

/* ---- Rede: use (A) Wokwi OU (B) ESP32 físico ---- */
// ---- (A) Wokwi (padrão) ----
// const char* WIFI_SSID     = "Wokwi-GUEST";
// const char* WIFI_PASSWORD = "";
// #define MQTT_SERVER "host.wokwi.internal"

// ---- (B) ESP32 físico ----
const char* WIFI_SSID     = "NorisIoT";
const char* WIFI_PASSWORD = "Secure10T";
#define MQTT_SERVER "172.16.10.101"   // endereço do broker da aula

WiFiClient wifiClient;

/* ---- MQTT (só publicação) ---- */
#define MQTT_PORT        1883
#define MQTT_CLIENT_ID   "DEFINA_UM_NOME"   // o nome da SUA placa (o mesmo do dataset)
#define MQTT_PUB_TOPIC   "FIAPIoT/motor/borda/" MQTT_CLIENT_ID          // a janela e a decisão da placa
#define MQTT_TOPIC_BOMBA "FIAPIoT/pivo/" MQTT_CLIENT_ID "/cmd/local"    // a ordem para a bomba do pivô
PubSubClient mqttClient(wifiClient);
uint32_t ultimaTentativaMQTT = 0;

/* ---- Pinos ---- */
#define SDA_PIN      22
#define SCL_PIN      23
#define LED_AZUL      4   // operando
#define LED_AMARELO  21   // inclinado_frente
#define LED_VERMELHO 18   // inclinado_tras
#define BUZZER       19   // anomalia
#define LED_ONBOARD   2   // aceso = conectado ao broker

/* ---- Sensor (MPU6050 ou MPU6500) ---- */
#define MPU_TYPE MPU6500 // no Wokwi: MPU6050
MPU_TYPE mpu;

calData calib = { 0 };

/* ---- Amostragem: 100 Hz, janela de 1 s ---- */
const int FS_HZ          = 100;
const int AMOSTRA_MS     = 1000 / FS_HZ;      // 10 ms
const int TAMANHO_JANELA = FS_HZ;             // 100 amostras = 1 s

float ax_buf[TAMANHO_JANELA];
float ay_buf[TAMANHO_JANELA];
float az_buf[TAMANHO_JANELA];
float mag_buf[TAMANHO_JANELA];

int indice = 0;
uint32_t tempoAnterior = 0;

/* ---- Nomes das classes, na ordem da saída da rede ---- */
const char* NOMES_CLASSES[4] = { "anomalia", "inclinado_frente",
                                 "inclinado_tras", "operando" };

/* ---- A ordem para a bomba ---- */
int ordemAnterior = -1;            // -1 = ainda não mandou; 0 = desligar; 1 = ligar
uint32_t ultimaOrdem = 0;
const uint32_t REPETIR_ORDEM_MS = 30000;   // repete a mesma ordem a cada 30 s

/* ---- Protótipos ---- */
int  classificarJanela(const float features[8]);
void acionarSaida(int classe);
void apagarTodasAsSaidas();
void publicarJanela(const float features[8], int classe);
void mandarNaBomba(int classe);
void conectarWiFi();
void conectarMQTT();

/* =========================== Features =========================== */
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

  Serial.println("Deixe o motor na posicao inicial/de uso e nao o movimente durante a calibracao...");
  delay(2000);
  // Calibração (no Wokwi, comente esta linha)
  mpu.calibrateAccelGyro(&calib);
  mpu.init(calib, 0x68);
  // Fundo de escala ±8 g
  mpu.setAccelRange(8);

  Serial.println("MPU iniciado");

  // A rede neural: carrega o modelo na memória de trabalho
  if (!ModelInit(modeloMotorRede, tensorArena, ARENA_SIZE)) {
    Serial.println("Erro: a rede neural nao carregou");
    while (1);
  }
  ModelPrintMetadata();   // mostra quanto da arena a rede usou

  pinMode(LED_AZUL,     OUTPUT);
  pinMode(LED_AMARELO,  OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(BUZZER,       OUTPUT);
  pinMode(LED_ONBOARD,  OUTPUT);

  apagarTodasAsSaidas();
  digitalWrite(LED_ONBOARD, LOW);

  // Teste das saídas
  Serial.println("Testando as saidas...");
  digitalWrite(LED_AZUL,     HIGH); delay(400); digitalWrite(LED_AZUL,     LOW);
  digitalWrite(LED_AMARELO,  HIGH); delay(400); digitalWrite(LED_AMARELO,  LOW);
  digitalWrite(LED_VERMELHO, HIGH); delay(400); digitalWrite(LED_VERMELHO, LOW);
  tone(BUZZER, 500, 250); noTone(BUZZER);   // buzzer passivo

  conectarWiFi();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setKeepAlive(60);
  mqttClient.setSocketTimeout(30);
  mqttClient.setBufferSize(512);

  Serial.println("Sistema pronto. A rede decide aqui e manda na bomba.");
  Serial.printf("  Publica em: %s\r\n", MQTT_PUB_TOPIC);
  Serial.printf("  Ordem para a bomba em: %s\r\n\r\n", MQTT_TOPIC_BOMBA);
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
    tempoAnterior += AMOSTRA_MS;
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

      // Features na ordem das colunas do treino
      float features[8] = { mx, my, mz, sx, sy, sz, stdMag, p2p };

      // Classifica, aciona a saída, publica e manda na bomba
      int classe = classificarJanela(features);
      acionarSaida(classe);
      publicarJanela(features, classe);
      mandarNaBomba(classe);

      indice = 0;
    }
  }
}

/* ---- Inferência: padroniza, roda a rede, fica com a maior probabilidade ---- */
int classificarJanela(const float features[8]) {
  float padronizado[8];
  float probabilidade[4];

  uint32_t t0 = micros();
  Scaler::standardize(features, padronizado);
  for (int i = 0; i < 8; i++) {
    ModelSetInput(padronizado[i], i);
  }
  if (!ModelRunInference()) {
    Serial.println("Erro: a inferencia falhou");
    return -1;
  }
  int classe = 0;
  for (int c = 0; c < 4; c++) {
    probabilidade[c] = ModelGetOutput(c);
    if (probabilidade[c] > probabilidade[classe]) classe = c;
  }
  uint32_t duracao = micros() - t0;

  Serial.println("--- Janela fechada ---");
  Serial.printf("  mean_ax=%.3f  mean_ay=%.3f  mean_az=%.3f\r\n",
                features[0], features[1], features[2]);
  Serial.printf("  std_ax=%.3f   std_ay=%.3f   std_az=%.3f\r\n",
                features[3], features[4], features[5]);
  Serial.printf("  std_mag=%.3f  p2p_mag=%.3f\r\n", features[6], features[7]);
  Serial.printf("  PREDIÇÃO:  %d -> %s (%.0f%%)\r\n",
                classe, NOMES_CLASSES[classe], probabilidade[classe] * 100);
  Serial.printf("  Inferencia: %lu us\r\n", duracao);
  Serial.println("----------------------");

  return classe;
}

/* ---- Acende a saída da classe ---- */
void acionarSaida(int classe) {
  apagarTodasAsSaidas();

  switch (classe) {
    case 0: tone(BUZZER, 500, 250);           break;   // anomalia
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

/* ---- Publica a janela e a predição da borda (a nuvem confere) ---- */
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
    Serial.println("MQTT: sem conexao - a decisao ficou so na placa");
  }
}

/* ---- A ordem para a bomba: só a torre operando irriga ---- */
void mandarNaBomba(int classe) {
  if (classe < 0) return;                    // a inferência falhou: não manda nada
  int ordem = (classe == 3) ? 1 : 0;         // 3 = operando

  // Só fala com a bomba quando muda de ideia (e repete a cada 30 s)
  if (ordem == ordemAnterior && millis() - ultimaOrdem < REPETIR_ORDEM_MS) return;

  JsonDocument doc;
  doc["dispositivo"] = MQTT_CLIENT_ID;
  doc["bomba"]       = (ordem == 1);
  doc["motivo"]      = NOMES_CLASSES[classe];

  String buffer;
  serializeJson(doc, buffer);

  if (mqttClient.publish(MQTT_TOPIC_BOMBA, buffer.c_str())) {
    if (ordem != ordemAnterior) {
      Serial.printf(">> Ordem para a bomba: %s (%s)\r\n",
                    ordem == 1 ? "LIGAR" : "DESLIGAR", NOMES_CLASSES[classe]);
    }
    ordemAnterior = ordem;
    ultimaOrdem   = millis();
  }
}

/* ---- WiFi: tenta por 10 s e segue ---- */
void conectarWiFi() {
  Serial.printf("Conectando ao WiFi %s", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_2dBm);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setSleep(false);
  uint32_t inicio = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - inicio < 10000) {
    delay(500);
    Serial.print('.');
  }
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Sem WiFi: a placa decide sozinha assim mesmo.");
  }
}

/* ---- MQTT: uma tentativa a cada 5 s, sem parar a leitura ---- */
void conectarMQTT() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (millis() - ultimaTentativaMQTT < 5000) return;
  ultimaTentativaMQTT = millis();

  Serial.printf("Conectando ao MQTT Broker %s...", MQTT_SERVER);
  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println(" Conectado!");
  } else {
    Serial.printf(" Falha rc=%d. Nova tentativa em 5 s\r\n", mqttClient.state());
  }
}
