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
const char* WIFI_SSID = "NorisIoT";
const char* WIFI_PASS = "Secure10T";

/* ---- MQTT ---- */
#define BROKER_IP    "172.16.10.101"             // IP do notebook do professor (Ethernet)
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
      /* rc negativo = a conexao TCP nem chegou ao broker (IP/porta/firewall).
         rc positivo = o broker respondeu e recusou (protocolo, id, credencial). */
      Serial.printf(" falhou, rc=%d. Tentando de novo em 3 s.\n", mqtt.state());
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  ESP32Sensors::Distancia::inicializar(TRIG_PIN, ECHO_PIN);

  Serial.print("Conectando ao WiFi");
  WiFi.mode(WIFI_STA);                            // so cliente: nada de AP
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println(" conectado!");
  Serial.print("WiFi IP: ");
  Serial.println(WiFi.localIP());

  mqtt.setServer(BROKER_IP, 1883);
}

void loop() {
  if (!mqtt.connected()) conectarMQTT();
  mqtt.loop();

  float dist = ESP32Sensors::Distancia::medirDistancia();
  String msg = String(dist, 1);                   // texto puro. Ex.: "37.5"

  /* true = retained: o broker guarda esta mensagem. Quem assinar depois
     recebe o último valor na hora, sem esperar a próxima publicação. */
  bool ok = mqtt.publish(TOPICO_DIST, msg.c_str(), true);
  Serial.println(ok ? "[TX] " TOPICO_DIST " -> " + msg + " cm"
                    : String("[TX] falhou ao publicar"));

  delay(200);
}
