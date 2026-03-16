/* *Includes* */
#include <WiFi.h>
#include <WiFiClient.h>
#include "ESP32SensorsDistancia.hpp"
#include <PubSubClient.h>
//#include <MqttClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>

/* ---- Config Wi‑Fi ---- */
const char* ssid     = "Wokwi-GUEST";
const char* password = "";
WiFiClient wifiClient; // Define client WiFi

/* ---- Config MQTT ---- */
#define MQTT_HOST       "54.36.178.49" //IP do Broker test.mosquitto.org
#define MQTT_PORT       1883
#define MQTT_PUB_TOPIC  "semaforo/distancia"
String MQTT_DEVICEID = "DistSensor";
PubSubClient client(wifiClient); // Define client PubSub (MQTT client)
//MqttClient mqttClient(wifiClient);

/* ---- Config distância ---- */
#define TRIG_PIN 25
#define ECHO_PIN 26
DistanceSensor sensorDist(TRIG_PIN, ECHO_PIN);
const int PESSOA_DIST = 50;

/* ---- Config intervalo temporal ---- */
int INTERVALO = 1000; // Intervalo entre detecções de pessoas - detecção uma vez por segundo
uint64_t tempo_anterior = 0;

/* ---- Função: Conectar ao MQTT Broker ---- */
void conectarMQTT() {
  Serial.print("Conectando ao MQTT Broker -> " + String(MQTT_HOST));
  while (!client.connected()) {
    MQTT_DEVICEID += WiFi.macAddress();
    if (client.connect(MQTT_DEVICEID.c_str())) {
      Serial.println(" --> Conectado ao MQTT! CLIENTID " + String(MQTT_DEVICEID));
    } else {
      Serial.print(" Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  //Inicializa todos os sensores
  sensorDist.inicializar();

  //Inicializa WiFi
  WiFi.begin(ssid, password);
  Serial.println("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");

  /* ---- Conexão ao MQTT: Definição de servidor ---- */
  client.setServer(MQTT_HOST, MQTT_PORT);
}

void loop() {
  if (!client.connected()) {
    conectarMQTT();
  }
  client.loop();

  if (millis() - tempo_anterior >= INTERVALO) {
    //Mede a distância
    float dist = sensorDist.medirDist();
    Serial.println("Distância: " + String(dist) + " cm.");

    if (dist <= PESSOA_DIST) {
      /* Transmissão dos dados em formato JSON */
      StaticJsonDocument<200> doc;
      doc["dist"] = dist;
      
      char buffer[200];
      serializeJson(doc, buffer);
      Serial.println("Pessoa detectada! Payload: " + String(buffer) + ". Enviando daods (mqtt publish)");
      
      client.publish(MQTT_PUB_TOPIC, buffer);
    }
  }

  delay(100);
}
