/* Aplicacao 06 - PUBLICADOR MQTT

   O ESP32 mede a distancia e publica o valor no broker.
   Fluxo da aula: SENSOR -> PUBLISH -> BROKER
*/
#include <NewPing.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Altere somente estes dados para a rede da sala.
const char* WIFI_SSID = "NorisIoT";
const char* WIFI_SENHA = "Secure10T";
// const char* WIFI_SSID = "Wokwi-GUEST";
// const char* WIFI_SENHA = "";

const char* BROKER_IP = "172.16.10.101";

const char* TOPICO = "fiap/iot/distancia";

const int TRIG_PIN = 22;
const int ECHO_PIN = 23;

NewPing sensorDist(TRIG_PIN, ECHO_PIN, 400);
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);
const char* CLIENT_ID = "ESP32Noris001Dist";

uint64_t tempoAgora = 0;
uint64_t INTERVALO_COLETA = 500;

void conectarWiFi() {
  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_SENHA);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println(" conectado!");
}

void conectarMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Conectando ao broker MQTT...");

    if (mqtt.connect(CLIENT_ID)) {
      Serial.println(" conectado!");
    } else {
      Serial.println(" falhou. Nova tentativa em 2 segundos.");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  conectarWiFi();
  mqtt.setServer(BROKER_IP, 1883);
  mqtt.setKeepAlive(120);
}

void loop() {
  if (!mqtt.connected()) {
    conectarMQTT();
  }
  mqtt.loop();

  if (millis() - tempoAgora >= INTERVALO_COLETA) {

    tempoAgora = millis();
    float distancia = sensorDist.ping_cm();
    if (distancia == 0) distancia = 400;  // sem eco = muito longe
    String mensagem = String(distancia, 0);

    mqtt.publish(TOPICO, mensagem.c_str());
    Serial.println("Distancia publicada: " + mensagem + " cm");
  }

}
