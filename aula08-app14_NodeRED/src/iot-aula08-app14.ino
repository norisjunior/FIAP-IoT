/* *Includes* */
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "ESP32Sensors.hpp"


/* ---- Config Hardware ---- */
const uint8_t DHT_PIN     = 26;
const uint8_t DHT_MODEL   = DHT22;
const uint8_t LED_PIN     = 27;
const uint8_t TRIG_PIN    = 17;
const uint8_t ECHO_PIN    = 16;
const float   DIST_LIMIAR = 100.0;
const uint8_t PIR_PIN     = 25;

/* ---- Config Wi‑Fi ---- */
const char* WIFI_SSID     = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
WiFiClient wifiClient; // Define client WiFi

/* ---- Config MQTT ---- */
#define MQTT_HOST       "broker.emqx.io"
#define MQTT_PORT       1883
#define MQTT_PUB_TOPIC  "FIAPIoT/sala1"
#define MQTT_DEVICEID   "NorisESP32IoT2025001"
//#define MQTT_USERNAME   "NorisESP32IoT2025001"
//#define MQTT_PASSWORD   "@...IoT..."
#define MQTT_QOS        0
#define MQTT_RETAIN     false
unsigned long ultimaTentativaReconexao = 0;
const unsigned long INTERVALO_RECONEXAO = 5000; // 5 segundos entre tentativas
PubSubClient mqttClient(wifiClient);
bool wifiConectado = false;
bool mqttConectado = false;


/* ---- Controle de intervalo de envio ---- */
const int INTERVALO_COLETA = 2500;  // 2,5 s
int proximaLeitura = 0;

/* ---- Protótipos ---- */
void conectarWiFi();
void conectarMQTT();

/* ---- MAIN (setup e loop) ---- */
void setup() {
  Serial.begin(115200);

  //Inicializa todos os sensores
  ESP32Sensors::beginAll(DHT_PIN, DHT_MODEL, TRIG_PIN, ECHO_PIN, DIST_LIMIAR, LED_PIN, PIR_PIN);

  conectarWiFi();
  
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  conectarMQTT();
  
  Serial.println("Sistema inicializado!");
}

void loop() {
  if (wifiConectado && !mqttClient.connected()) {
    conectarMQTT();
  }

  if (mqttConectado) {
    mqttClient.loop();
  } else {
    Serial.println("MQTT desconectado. Pulando ciclo de leitura.");
    return;
  }

  if (millis() < proximaLeitura) return;
  proximaLeitura = millis() + INTERVALO_COLETA;


  /* --- Coleta de dados --- */
  bool mov = ESP32Sensors::Movimento::detectarMovimento();
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
  JsonDocument doc;
  doc["temp"] = amb.temp;
  doc["umid"] = amb.umid;
  doc["ic"] = amb.ic;
  doc["dist"] = dist.cm;
  doc["mov"]  = mov;

  char payload[200];
  serializeJson(doc, payload);

  /* === LED aceso durante a transmissão === */
  ESP32Sensors::LED::on();
  bool ok = mqttClient.publish(MQTT_PUB_TOPIC, payload);
  Serial.println(ok ? "[MQTT] Publicado" : "[MQTT] Falha ao publicar");
  ESP32Sensors::LED::off();

  delay(100);
}

/* ---- Função: Conectar ao WiFi ---- */
void conectarWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando ao WiFi");
  
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[WiFi] Conectado!");
    Serial.printf("[WiFi] IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.println("");
    wifiConectado = true;
  } else {
    Serial.println("\n[WiFi] Falha na conexão - continuando sem MQTT");
    wifiConectado = false;
  }
}

/* ---- Função: Conectar ao MQTT Broker ---- */
void conectarMQTT() {
  if (!wifiConectado) return;
  
  Serial.printf("[MQTT] Conectando ao broker %s", MQTT_HOST);
  
  int tentativas = 0;
  while (!mqttClient.connected() && tentativas < 3) {
    //if (mqttClient.connect(MQTT_DEVICEID, MQTT_USERNAME, MQTT_PASSWORD)) {
    if (mqttClient.connect(MQTT_DEVICEID)) {
      Serial.println(" --> Conectado ao MQTT Broker!");
      mqttConectado = true;
    } else {
      Serial.printf(" Falha, rc=%d", mqttClient.state());
      tentativas++;
      if (tentativas < 3) {
        Serial.println(" Tentando novamente em 3s...");
        delay(3000);
      }
    }
  }
  
  if (!mqttConectado) {
    Serial.println(" --> Falha final na conexão MQTT");
  }
}

