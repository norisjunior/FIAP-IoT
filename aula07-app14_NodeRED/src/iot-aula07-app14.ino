/* *Includes* */
#include <WiFi.h>
#include <WiFiClient.h>
#include <MqttClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "ESP32Sensors.hpp"

/* ---- Config Wi‑Fi ---- */
const char* WIFI_SSID     = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
WiFiClient wifiClient; // Define client WiFi

/* ---- Config MQTT ---- */
#define MQTT_HOST       "broker.emqx.io"
#define MQTT_PORT       1883
#define MQTT_PUB_TOPIC  "FIAPIoT/sala1"
#define MQTT_DEVICEID   "FIAPIoTDevice001"
#define MQTT_QOS        0
#define MQTT_RETAIN     false
MqttClient mqttClient(wifiClient);

/* ---- Controle de intervalo de envio ---- */
const int INTERVALO_COLETA = 2500;  // 2,5 s
int proximaLeitura = 0;

/* ---- Protótipos ---- */
void conectarWiFi();
void conectarMQTT();
bool publicarMsg();

/* ---- MAIN (setup e loop) ---- */
void setup() {
  Serial.begin(115200);

  //Inicializa todos os sensores
  ESP32Sensors::beginAll();

  conectarWiFi();
  conectarMQTT();
}

void loop() {
  mqttClient.poll();

  if (!mqttClient.connected()) {
    conectarMQTT();
  }

  if (millis() < proximaLeitura) return;   // Aguarda próximo ciclo
  proximaLeitura = millis() + INTERVALO_COLETA;

  /* --- Coleta de dados --- */
  bool mov = ESP32Sensors::PIR::detectarMovimento();
  ESP32Sensors::Distancia::DISTANCIA dist = ESP32Sensors::Distancia::medirDistancia();
  ESP32Sensors::Ambiente::AMBIENTE amb = ESP32Sensors::Ambiente::medirAmbiente();
  
  if (!amb.valido) {
    Serial.println("[ERRO] Leitura DHT inválida.");
    return;                                   // Pula ciclo se houver erro
  }

  /* --- Log no Serial Monitor --- */
  Serial.printf("[DADOS] Temp: %.2f °C | Umid: %.2f %% | HI: %.2f °C | Dist: %.2f cm | PIR: %s\n",
                amb.temp, amb.umid, amb.ic, dist.cm, mov ? "MOVIMENTO" : "VAZIO");
  Serial.println("");

  /* === Montagem do JSON === */
  StaticJsonDocument<128> doc;
  doc["temp"] = amb.temp;
  doc["umid"] = amb.umid;
  doc["ic"] = amb.ic;
  doc["dist"] = dist.cm;
  doc["mov"]  = mov;

  char payload[128];
  serializeJson(doc, payload);

  /* === LED aceso durante a transmissão === */
  ESP32Sensors::LED::on();
  bool ok = publicarMsg(MQTT_PUB_TOPIC, payload);
  ESP32Sensors::LED::off();

  Serial.println(ok ? "[MQTT] Publicado" : "[MQTT] Falha ao publicar");

  delay(100);
}

/* ---- Função: Conectar ao WiFi ---- */
void conectarWiFi() {
  Serial.printf("Conectando ao WiFi %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

/* ---- Função: Conectar ao MQTT Broker ---- */
void conectarMQTT() {
  // Se já está conectado, não faz nada
  if (mqttClient.connected()) {
    return;
  }

  Serial.print("Conectando ao MQTT Broker" + String(MQTT_HOST));
  // Conexão ao MQTT: Definição de ID e keep alive
  mqttClient.setId(MQTT_DEVICEID);
  mqttClient.setKeepAliveInterval(60);

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

/* ---- Publicação simplificada (QoS 0, retain opcional) ---- */
bool publicarMsg(const char* topico, const char* dados) {
  mqttClient.beginMessage(topico, strlen(dados), MQTT_RETAIN, MQTT_QOS);
  mqttClient.print(dados);
  return mqttClient.endMessage();
}