/* ==========================================================================
   Physical Computing, Embedded AI, Robotics & Cognitive IoT

   Aplicação 06 - Dispositivo do PROFESSOR (MQTT publisher)

   O HC-SR04 mede a distância e o ESP32 PUBLICA o número no broker.
   Só isso: quem decide o que fazer com o número é cada dispositivo que
   assina o tópico (Aplicação 07).
   ========================================================================== */

#include <WiFi.h>
#include <PubSubClient.h>
#include "ESP32SensorsDistancia.hpp"

/* ---- Wi-Fi (roteador da sala, 2,4 GHz) ---- */
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";

/* ---- MQTT ---- */
#define BROKER_IP    "host.wokwi.internal"             // IP do notebook do professor
#define TOPICO_DIST  "fiap/iot/2026/prof/dist"

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

/* ---- Sensor de distância ---- */
#define TRIG_PIN 22
#define ECHO_PIN 23

void conectarMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Conectando ao broker...");
    if (mqtt.connect("professor")) {
      Serial.println(" conectado!");
    } else {
      Serial.println(" falhou. Tentando de novo em 3 s.");
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  ESP32Sensors::Distancia::inicializar(TRIG_PIN, ECHO_PIN);

  Serial.print("Conectando ao WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println(" conectado!");

  mqtt.setServer(BROKER_IP, 1883);
}

void loop() {
  if (!mqtt.connected()) conectarMQTT();
  mqtt.loop();

  float dist = ESP32Sensors::Distancia::medirDistancia();
  String msg = String(dist, 1);                   // texto puro. Ex.: "37.5"

  /* true = retained: o broker guarda esta mensagem. Quem assinar depois
     recebe o último valor na hora, sem esperar a próxima publicação. */
  mqtt.publish(TOPICO_DIST, msg.c_str(), true);
  Serial.println("[TX] " TOPICO_DIST " -> " + msg + " cm");

  delay(200);
}
