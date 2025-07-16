/* *Includes* */
#include <WiFi.h>
#include <WiFiClient.h>
#include "ESP32SensorsDistancia.hpp"
//#include <PubSubClient.h>
#include <MqttClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>

/* ---- Config Wi‑Fi ---- */
const char* ssid     = "Wokwi-GUEST";
const char* password = "";
WiFiClient wifiClient; // Define client WiFi

/* ---- Config MQTT ---- */
#define MQTT_HOST       "broker.emqx.io"
#define MQTT_PORT       1883
#define MQTT_PUB_TOPIC  "semaforo1/distancia"
#define MQTT_DEVICEID   "DistSensorSemaforo00001"
#define MQTT_QOS        1
#define MQTT_RETAIN     false
//PubSubClient client(wifiClient); // Define client PubSub (MQTT client)
MqttClient mqttClient(wifiClient);

/* ---- Config distância ---- */
#define TRIG_PIN 25
#define ECHO_PIN 26
DistanceSensor sensorDist(TRIG_PIN, ECHO_PIN);
const int PESSOA_DIST = 50;

/* ---- Função: Conectar ao MQTT Broker ---- */
void conectarMQTT() {
  Serial.print("Conectando ao MQTT Broker" + String(MQTT_HOST));
  while (!mqttClient.connected()) {
    if (mqttClient.connect(MQTT_HOST, MQTT_PORT)) {
      Serial.println("--> Conectado ao MQTT!");
    } else {
      Serial.print(" Falha, rc=");
      Serial.print(mqttClient.connectError());
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

  /* ---- Conexão ao MQTT: Definição de ID e keep alive ---- */
  mqttClient.setId(MQTT_DEVICEID);
  mqttClient.setKeepAliveInterval(60);
}

void loop() {
  if (!mqttClient.connected()) {
    conectarMQTT();
  }
  mqttClient.poll();

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
    
    mqttClient.beginMessage(MQTT_PUB_TOPIC, strlen(buffer), MQTT_RETAIN, MQTT_QOS);
    mqttClient.print(buffer);
    mqttClient.endMessage();

    delay(5000); //intervalo antes de nova detecção
  }
  
  delay(1000);
}
