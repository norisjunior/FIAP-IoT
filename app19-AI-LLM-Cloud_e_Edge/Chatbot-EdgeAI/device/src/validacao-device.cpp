/* validacao-device — o ESP32 classifica a janela na borda e publica a predição */

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

/* ---- MQTT (só publicação) ---- */
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

/* ---- Nomes das classes, na ordem de classes_ ---- */
const char* NOMES_CLASSES[4] = { "anomalia", "inclinado_frente",
                                 "inclinado_tras", "operando" };

/* ---- Protótipos ---- */
int  classificarJanela(const float features[8]);
void acionarSaida(int classe);
void apagarTodasAsSaidas();
void publicarJanela(const float features[8], int classe);
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

      // Classifica, aciona a saída e publica
      int classe = classificarJanela(features);
      acionarSaida(classe);
      publicarJanela(features, classe);

      indice = 0;
    }
  }
}

/* ---- Inferência ---- */
int classificarJanela(const float features[8]) {
  float padronizado[8];

  uint32_t t0 = micros();
  Scaler::standardize(features, padronizado);
  int classe = modeloRF.predict(padronizado);
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

/* ---- Publica a janela e a predição da borda ---- */
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
    } else {
      Serial.printf(" Falha rc=%d. Tentando em 5s...\r\n", mqttClient.state());
      delay(5000);
    }
  }
}
