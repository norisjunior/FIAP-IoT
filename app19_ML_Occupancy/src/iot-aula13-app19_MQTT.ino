/* ==== INCLUDES ===================================================== */
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "ESP32Sensors.hpp"   // Ambiente (DHT22), LED, LDR (Lux) e CO2 (ppm)


/* ==== Configurações de Hardware =================================================== */
const uint8_t DHT_PIN   = 26;
const uint8_t DHT_MODEL = DHT22;
const uint8_t LED_PIN   = 21;
const uint8_t LDR_PIN   = 35;
const uint8_t CO2_PIN   = 34;
const uint8_t HR_PIN    = 32;

/* ==== WI-FI =================================================== */
const char* WIFI_SSID     = "Wokwi-GUEST";   // Rede pública do simulador
const char* WIFI_PASSWORD = "";
WiFiClient wifiClient;

/* ==== MQTT =================================================== */
#define MQTT_HOST       "host.wokwi.internal"
#define MQTT_PORT       1883
#define MQTT_PUB_TOPIC  "FIAPIoT/ML_occupancy"
#define MQTT_DEVICEID   "FIAP_IoT_app19_001"
#define MQTT_QOS        0
#define MQTT_RETAIN     false
PubSubClient mqttClient(wifiClient);
bool wifiConectado = false;
bool mqttConectado = false;
unsigned long ultimaTentativaReconexao = 0;
const unsigned long INTERVALO_RECONEXAO = 5000; // 5 segundos entre tentativas

/* ==== CONSTANTES =================================== */
static unsigned long lastMs = 0;
const unsigned long INTERVAL = 30000; // 30s entre envios

/* ==== AUXILIARES =================================================== */
void conectarWiFi() {
  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  uint8_t tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 30) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("OK! IP: ");
    Serial.println(WiFi.localIP());
    wifiConectado = true;
  } else {
    Serial.println("Falha ao conectar Wi-Fi.");
    wifiConectado = false;
  }
}

void conectarMQTT() {
  if (!wifiConectado) return;
  
  Serial.printf("[MQTT] Conectando ao broker %s", MQTT_HOST);
  
  int tentativas = 0;
  while (!mqttClient.connected() && tentativas < 3) {
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

bool buildAndPublishJSON() {
  ESP32Sensors::Ambiente::AMBIENTE  amb = ESP32Sensors::Ambiente::medirAmbiente();
  ESP32Sensors::LDR::DADOS_LDR      luz = ESP32Sensors::LDR::ler();
  ESP32Sensors::CO2::DADOS_CO2      co2 = ESP32Sensors::CO2::ler();
  ESP32Sensors::HR::DADOS_HR        hr  = ESP32Sensors::HR::ler();

  if (!amb.valido || !luz.valido || !co2.valido) {
    Serial.println("Leituras inválidas: sem envio de dados!");
    return false;
  }

  //float hr = humidityRatioMock();

  /* === Log no Serial Monitor === */
  Serial.printf("[DADOS] Temp: %.2f °C | Umid: %.2f %% | Light: %.1f lux | CO2: %.1f ppm | HR: %.6f\n",
                amb.temp, amb.umid, luz.lux, co2.ppm, hr);
  Serial.println("");

  /* === Montagem do JSON === */
  JsonDocument doc;
  doc["dispositivo"]   = MQTT_DEVICEID;
  doc["Temperature"]   = amb.temp;
  doc["Humidity"]      = amb.umid;
  doc["Light"]         = luz.lux;
  doc["CO2"]           = co2.ppm;
  doc["HumidityRatio"] = hr.valor;

  char payload[300];
  serializeJson(doc, payload);

  /* === Publicação MQTT === */
  bool ok = mqttClient.publish(MQTT_PUB_TOPIC, payload, MQTT_RETAIN);
  Serial.println(ok ? "[MQTT] Publicado" : "[MQTT] Falha ao publicar");
  if (ok) {
    Serial.println(String("Enviado: \n") + payload);
    Serial.println("");
  }
  
  return ok;
}

/* ==== SETUP / LOOP ================================================ */
void setup() {
  Serial.begin(115200);
  ESP32Sensors::beginAll(DHT_PIN, DHT_MODEL, LED_PIN, LDR_PIN, CO2_PIN, HR_PIN);
  
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
    delay(100);
    return;
  }

  unsigned long now = millis();

  if (now - lastMs >= INTERVAL) {
    Serial.println("Preparando dados para envio ao MQTT");
    
    ESP32Sensors::LED::on();
    bool ok = buildAndPublishJSON();
    ESP32Sensors::LED::off();
    
    if (!ok) {
      Serial.println("Falha no envio ao MQTT.");
    }
    
    lastMs = now; // reprograma próximo envio
  }
  delay(100);
}