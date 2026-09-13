/* ---- Includes ---- */
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "ESP32SensorsAmbiente.hpp"
#include "ESP32SensorsDistancia.hpp"
#include "ESP32SensorsAccel.hpp"
#include "ESP32SensorsLED.hpp"

/* ---- Config Hardware ---- */
const uint8_t DHT_PIN = 4;
const uint8_t DHT_MODEL = DHT22;
const uint8_t TRIG_PIN = 19;
const uint8_t ECHO_PIN = 18;
const uint8_t SCL_PIN = 23;
const uint8_t SDA_PIN = 22;
const uint8_t LED_PIN = 21;

/* ---- Config Wi-Fi e MQTT ---- */
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
#define MQTT_SERVER "host.wokwi.internal"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "NexoLogEquipe01"
#define MQTT_PUB_TOPIC "FIAPIoT/nexolog/equipe01/dados"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

/* ---- Controle de intervalo ---- */
const unsigned long INTERVALO_COLETA = 2500;
const unsigned long INTERVALO_RECONEXAO = 5000;
unsigned long tempoAnterior = 0;
unsigned long ultimaTentativaWiFi = 0;
unsigned long ultimaTentativaMQTT = 0;

/* ---- Protótipos ---- */
void conectarWiFi();
void conectarMQTT();
bool enviarDadosColetados();

void setup() {
  Serial.begin(115200);
  ESP32Sensors::Ambiente::inicializar(DHT_PIN, DHT_MODEL);
  ESP32Sensors::Distancia::inicializar(TRIG_PIN, ECHO_PIN);
  ESP32Sensors::Accel::inicializar(SCL_PIN, SDA_PIN);
  ESP32Sensors::LED::inicializar(LED_PIN);

  conectarWiFi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setBufferSize(768);
  mqttClient.setKeepAlive(120);

  Serial.println("NexoLog - monitoramento de entregas");
}

void loop() {
  if (millis() - tempoAnterior >= INTERVALO_COLETA) {
    tempoAnterior = millis();
    enviarDadosColetados();
  }

  if (WiFi.status() != WL_CONNECTED) {
    if (millis() - ultimaTentativaWiFi >= INTERVALO_RECONEXAO) {
      ultimaTentativaWiFi = millis();
      WiFi.reconnect();
    }
  } else if (!mqttClient.connected()) {
    if (millis() - ultimaTentativaMQTT >= INTERVALO_RECONEXAO) {
      ultimaTentativaMQTT = millis();
      conectarMQTT();
    }
  } else {
    mqttClient.loop();
  }
}

void conectarWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setSleep(false);
}

void conectarMQTT() {
  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println("[MQTT] Conectado");

  } else {
    Serial.printf("[MQTT] Falha: %d\n", mqttClient.state());
  }
}

bool enviarDadosColetados() {
  ESP32Sensors::Ambiente::AMBIENTE amb = ESP32Sensors::Ambiente::medirAmbiente();
  ESP32Sensors::Distancia::DISTANCIA dist = ESP32Sensors::Distancia::medirDistancia();
  AccelData accel = ESP32Sensors::Accel::medirAccel();
  float movimentacao = ESP32Sensors::Accel::medirMovimentacao(accel);

  Serial.printf("Temp: %.1f C | Umid: %.1f %% | Dist: %.1f cm | Movim: %.2f m/s2\n",
                amb.temp, amb.umid, dist.cm, movimentacao);

  JsonDocument doc;
  doc["device"] = MQTT_CLIENT_ID;
  doc["temp"] = amb.temp;
  doc["umid"] = amb.umid;
  doc["dist"] = dist.cm;
  doc["accel_x"] = accel.accelX;
  doc["accel_y"] = accel.accelY;
  doc["accel_z"] = accel.accelZ;
  doc["movimentacao"] = movimentacao;

  String payload;
  serializeJson(doc, payload);
  if (!mqttClient.connected()) return false;
  bool ok = mqttClient.publish(MQTT_PUB_TOPIC, payload.c_str());
  Serial.println(ok ? "[MQTT] Publicado" : "[MQTT] Falha ao publicar");
  return ok;
}
