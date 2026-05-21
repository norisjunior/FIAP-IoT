#include <Arduino.h>
#include <FlixPeriph.h>
#include <MPU6500.h>
#include <Wire.h>
#include <math.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

/* ---- Config Wi-Fi ---- */
const char* WIFI_SSID     = "IoTNJ";
const char* WIFI_PASSWORD = "Th1ng$IoT";
WiFiClient wifiClient;

/* ---- Config MQTT ---- */
#define MQTT_SERVER    "10.58.131.155"
#define MQTT_PORT      1883
#define MQTT_PUB_TOPIC "FIAPIoT/motor/features"
#define MQTT_CLIENT_ID "IoTDeviceNorisMotorML001"
PubSubClient mqttClient(wifiClient);

/* ---- Pinos ---- */
#define BTN_MOTOR    26
#define BTN_ANOMALIA 25
#define LED_PIN      27
#define MOTOR_IN1    22
#define MOTOR_IN2    23
#define SDA_PIN      19
#define SCL_PIN      18

MPU6500 mpu(Wire);

bool motorLigado   = false;
bool anomaliaAtiva = false;

int ultimoBotaoMotor    = HIGH;
int ultimoBotaoAnomalia = HIGH;

unsigned long ultimoDebounceMotor    = 0;
unsigned long ultimoDebounceAnomalia = 0;
const unsigned long debounceMs = 300;

const int PWM_MOTOR = 100;

const int TAMANHO_JANELA = 100;
float ax_buf[TAMANHO_JANELA];
float ay_buf[TAMANHO_JANELA];
float az_buf[TAMANHO_JANELA];

int indice = 0;
uint32_t tempoAnterior = 0;
const int AMOSTRA_MS = 10; // 100 Hz

/* ---- Protótipos ---- */
void conectarWiFi();
void conectarMQTT();
void publicarFeatures(const char* label,
                      float mx, float my, float mz,
                      float sx, float sy, float sz,
                      float rx, float ry, float rz, float rmag);

/* ---- Motor ---- */
void ligarMotor() {
  analogWrite(MOTOR_IN1, 180);
  digitalWrite(MOTOR_IN2, LOW);
  delay(150);
  analogWrite(MOTOR_IN1, PWM_MOTOR);
  digitalWrite(MOTOR_IN2, LOW);
}

void desligarMotor() {
  analogWrite(MOTOR_IN1, 0);
  digitalWrite(MOTOR_IN2, LOW);
}

/* ---- Features ---- */
float calcMean(float arr[], int n) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += arr[i];
  return soma / n;
}

float calcStd(float arr[], int n) {
  float media = calcMean(arr, n);
  float soma = 0;
  for (int i = 0; i < n; i++) soma += (arr[i] - media) * (arr[i] - media);
  return sqrt(soma / n);
}

float calcRMS(float arr[], int n) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += arr[i] * arr[i];
  return sqrt(soma / n);
}

float calcRMSMagnitude(float axArr[], float ayArr[], float azArr[], int n) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += axArr[i]*axArr[i] + ayArr[i]*ayArr[i] + azArr[i]*azArr[i];
  return sqrt(soma / n);
}

/* ============================== SETUP ============================== */
void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);
  if (!mpu.begin()) {
    Serial.println("Erro: MPU nao encontrado");
    while (1);
  }
  mpu.setAccelRange(IMUInterface::ACCEL_RANGE_8G);
  Serial.println("MPU iniciado");

  pinMode(BTN_MOTOR,    INPUT_PULLUP);
  pinMode(BTN_ANOMALIA, INPUT_PULLUP);
  pinMode(LED_PIN,      OUTPUT);
  pinMode(MOTOR_IN1,    OUTPUT);
  pinMode(MOTOR_IN2,    OUTPUT);

  desligarMotor();
  digitalWrite(LED_PIN, LOW);

  conectarWiFi();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setKeepAlive(60);
  mqttClient.setSocketTimeout(30);
  mqttClient.setBufferSize(512);

  Serial.println("Sistema pronto.");
  Serial.println("  Botao 26: liga/desliga motor");
  Serial.println("  Botao 25: ativa/desativa anomalia (so com motor desligado)");
  Serial.printf("  Topico MQTT: %s\n\n", MQTT_PUB_TOPIC);
  Serial.println("label,mean_ax,mean_ay,mean_az,std_ax,std_ay,std_az,rms_ax,rms_ay,rms_az,rms_mag");
}

/* ============================== LOOP =============================== */
void loop() {
  if (!mqttClient.connected()) {
    conectarMQTT();
  }
  mqttClient.loop();

  // --- Botao motor (toggle) ---
  int leituraMotor = digitalRead(BTN_MOTOR);
  if (ultimoBotaoMotor == HIGH && leituraMotor == LOW) {
    if (millis() - ultimoDebounceMotor > debounceMs) {
      motorLigado = !motorLigado;
      if (motorLigado) {
        ligarMotor();
        digitalWrite(LED_PIN, anomaliaAtiva ? HIGH : LOW);
        Serial.println("Motor LIGADO");
      } else {
        desligarMotor();
        digitalWrite(LED_PIN, LOW);
        Serial.println("Motor DESLIGADO");
      }
      ultimoDebounceMotor = millis();
    }
  }
  ultimoBotaoMotor = leituraMotor;

  // --- Botao anomalia (toggle, so quando motor desligado) ---
  int leituraAnomalia = digitalRead(BTN_ANOMALIA);
  if (ultimoBotaoAnomalia == HIGH && leituraAnomalia == LOW) {
    if (millis() - ultimoDebounceAnomalia > debounceMs) {
      if (!motorLigado) {
        anomaliaAtiva = !anomaliaAtiva;
        Serial.printf("Anomalia: %s (aplique/remova a fita e ligue o motor)\n",
                      anomaliaAtiva ? "ATIVA" : "NORMAL");
      }
      ultimoDebounceAnomalia = millis();
    }
  }
  ultimoBotaoAnomalia = leituraAnomalia;

  // --- Coleta IMU a 100 Hz ---
  if (millis() - tempoAnterior >= AMOSTRA_MS) {
    tempoAnterior = millis();
    mpu.read();
    mpu.getAccel(ax_buf[indice], ay_buf[indice], az_buf[indice]);
    indice++;

    if (indice >= TAMANHO_JANELA) {
      const char* label;
      if (!motorLigado)       label = "parado";
      else if (anomaliaAtiva) label = "ligado_anomalia";
      else                    label = "ligado_normal";

      float mx   = calcMean(ax_buf, TAMANHO_JANELA);
      float my   = calcMean(ay_buf, TAMANHO_JANELA);
      float mz   = calcMean(az_buf, TAMANHO_JANELA);
      float sx   = calcStd(ax_buf,  TAMANHO_JANELA);
      float sy   = calcStd(ay_buf,  TAMANHO_JANELA);
      float sz   = calcStd(az_buf,  TAMANHO_JANELA);
      float rx   = calcRMS(ax_buf,  TAMANHO_JANELA);
      float ry   = calcRMS(ay_buf,  TAMANHO_JANELA);
      float rz   = calcRMS(az_buf,  TAMANHO_JANELA);
      float rmag = calcRMSMagnitude(ax_buf, ay_buf, az_buf, TAMANHO_JANELA);

      Serial.printf("%s,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
                    label, mx, my, mz, sx, sy, sz, rx, ry, rz, rmag);

      publicarFeatures(label, mx, my, mz, sx, sy, sz, rx, ry, rz, rmag);

      indice = 0;
    }
  }
}

/* =========================== FUNÇÕES =============================== */
void conectarWiFi() {
  Serial.printf("Conectando ao WiFi %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println("");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void conectarMQTT() {
  while (!mqttClient.connected()) {
    Serial.printf("Conectando ao MQTT Broker %s...", MQTT_SERVER);
    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      Serial.println(" Conectado!");
    } else {
      Serial.printf(" Falha rc=%d. Tentando em 5s...\n", mqttClient.state());
      delay(5000);
    }
  }
}

void publicarFeatures(const char* label,
                      float mx, float my, float mz,
                      float sx, float sy, float sz,
                      float rx, float ry, float rz, float rmag) {
  JsonDocument doc;
  doc["device"]  = MQTT_CLIENT_ID;
  doc["label"]   = label;
  doc["mean_ax"] = serialized(String(mx, 3));
  doc["mean_ay"] = serialized(String(my, 3));
  doc["mean_az"] = serialized(String(mz, 3));
  doc["std_ax"]  = serialized(String(sx, 3));
  doc["std_ay"]  = serialized(String(sy, 3));
  doc["std_az"]  = serialized(String(sz, 3));
  doc["rms_ax"]  = serialized(String(rx, 3));
  doc["rms_ay"]  = serialized(String(ry, 3));
  doc["rms_az"]  = serialized(String(rz, 3));
  doc["rms_mag"] = serialized(String(rmag, 3));

  String buffer;
  serializeJson(doc, buffer);

  Serial.print("PAYLOAD MQTT: ");
  Serial.println(buffer.c_str());

  bool ok = mqttClient.publish(MQTT_PUB_TOPIC, buffer.c_str());
  Serial.println(ok ? "MQTT: enviado com sucesso" : "MQTT: falha no envio");
}
