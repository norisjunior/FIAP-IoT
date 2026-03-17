/* *Includes* */
#include <WiFi.h>
#include <WiFiClient.h>
#include "ESP32SensorsDistancia.hpp"
#include <PubSubClient.h>
//#include <MqttClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>

/* ---- Config Wi‑Fi ---- */
const char* WIFI_SSID     = "Wokwi-GUEST";   // Rede pública do simulador
const char* WIFI_PASSWORD = "";
WiFiClient wifiClient;

/* ---- Config MQTT ---- */
#define MQTT_HOST       "host.wokwi.internal" //IP do Broker test.mosquitto.org
#define MQTT_PORT       1883
#define MQTT_PUB_TOPIC  "semaforo/distancia"
#define MQTT_DEVICEID   "smartdevice"

PubSubClient mqttClient(wifiClient);
bool wifiConectado = false;
bool mqttConectado = false;


/* ---- Config distância ---- */
#define TRIG_PIN 25
#define ECHO_PIN 26
DistanceSensor sensorDist(TRIG_PIN, ECHO_PIN);
const int PESSOA_DIST = 50;

/* ---- Config intervalo temporal ---- */
int INTERVALO = 2000; // Intervalo entre detecções de pessoas - detecção uma vez a cada 2 segundos
uint64_t tempo_anterior = 0;

//Protótipos de funções a utilizar
void conectarWiFi();
void conectarMQTT();


void setup() {
  Serial.begin(115200);

  //Inicializa todos os sensores
  sensorDist.inicializar();

  //Conecta no WiFi
  conectarWiFi();

  //Configura MQTT e conecta no Broker
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setKeepAlive(60);     // Manter conexão viva por 60s
  mqttClient.setSocketTimeout(60); // Timeout de 60s
  mqttClient.setBufferSize(1024);   // Buffer adequado considerando o payload gerado

  Serial.println("\nSistema de envio de distância inicializado!");
  Serial.printf("Envio de dados a cada %d segundos...", (INTERVALO/1000)); Serial.println("");

  Serial.println("\n>> Sistema pronto!\n");


}

void loop() {

  if (wifiConectado && !mqttClient.connected()) {
    conectarMQTT();
  }

  if (mqttConectado) {
    mqttClient.loop();
  } else {
    Serial.println("MQTT desconectado. Pulando ciclo de leitura.");
    delay(1000);
    return;
  }

  if (millis() - tempo_anterior >= INTERVALO) {
    tempo_anterior = millis();
    //Mede a distância
    float dist = sensorDist.medirDist();
    Serial.println("Distância: " + String(dist) + " cm.");

    if (dist <= PESSOA_DIST) {
      /* Transmissão dos dados em formato JSON */
      JsonDocument doc;
      doc["dist"] = dist;

      char buffer[1024];
      serializeJson(doc, buffer);
      Serial.println("Pessoa detectada! Payload: " + String(buffer) + ". Enviando daods (mqtt publish)");

      mqttClient.publish(MQTT_PUB_TOPIC, buffer);
    }
  }

  delay(100);
}


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
