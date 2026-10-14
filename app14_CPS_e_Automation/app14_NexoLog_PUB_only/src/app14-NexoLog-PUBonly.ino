/* ---- Includes ---- */
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "ESP32SensorsAmbiente.hpp"
#include "ESP32SensorsDistancia.hpp"
#include "ESP32SensorsAccel.hpp"

/* ---- Config Hardware ---- */
const uint8_t DHT_PIN = 4;
const uint8_t DHT_MODEL = DHT22;
const uint8_t TRIG_PIN = 19;
const uint8_t ECHO_PIN = 18;
const uint8_t SCL_PIN = 23;
const uint8_t SDA_PIN = 22;

/* ---- Config Wi-Fi e MQTT ---- */
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_SENHA = "";
const char* MQTT_BROKER = "host.wokwi.internal";
const uint16_t MQTT_PORTA = 1883;
// O Client ID identifica a sessão; o DISPOSITIVO_ID preserva o JSON da equipe.
const char* MQTT_CLIENT_ID = "a12345678-app14-01";
const char* DISPOSITIVO_ID = "NexoLogEquipe01";
#define MQTT_PUB_TOPIC "FIAPIoT/nexolog/equipe01/dados"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

/* ---- Controle de intervalo ---- */
const uint32_t INTERVALO_COLETA = 1000;   // publica e le o ultrassonico
const uint32_t INTERVALO_DHT = 2100;      // o DHT22 nao responde mais rapido
const uint32_t INTERVALO_MPU = 50;        // 20 amostras por segundo
const uint32_t INTERVALO_RECONEXAO = 5000;
uint32_t tempoAnterior = 0, ultimoDHT = 0, ultimoMPU = 0;
uint32_t ultimaTentativaWiFi = 0, ultimaTentativaMQTT = 0;

/* ---- Medicoes guardadas entre um envio e outro ---- */
ESP32Sensors::Ambiente::AMBIENTE ambiente = {NAN, NAN, NAN, false};
float distancia = NAN;
AccelData accel = {};
float movimentacaoMax = NAN;

/* ---- Protótipos ---- */
void conectarWiFi();
void conectarMQTT();
bool enviarDadosColetados();

void setup() {
  Serial.begin(115200);
  ESP32Sensors::Ambiente::inicializar(DHT_PIN, DHT_MODEL);
  ESP32Sensors::Distancia::inicializar(TRIG_PIN, ECHO_PIN);
  ESP32Sensors::Accel::inicializar(SCL_PIN, SDA_PIN);

  // O controle de tentativa usa millis(). No boot ele ainda e menor que
  // INTERVALO_RECONEXAO, entao esperamos para a primeira tentativa passar.
  delay(INTERVALO_RECONEXAO);
  conectarWiFi();
  mqttClient.setServer(MQTT_BROKER, MQTT_PORTA);
  mqttClient.setBufferSize(768);
  mqttClient.setKeepAlive(120);
  mqttClient.setSocketTimeout(2); // Espera pela resposta MQTT, em segundos

  // Linha de titulo do CSV. Serial.println ja termina em \r\n .
  Serial.println("temp,umid,dist,movimentacao");
}

void loop() {
  // O DHT22 e lento: guardamos a ultima leitura boa e enviamos ela.
  if (millis() - ultimoDHT >= INTERVALO_DHT) {
    ultimoDHT = millis();
    ESP32Sensors::Ambiente::AMBIENTE leitura = ESP32Sensors::Ambiente::medirAmbiente();
    if (leitura.valido) {
      ambiente = leitura;
    }
  }

  // O MPU e rapido: 20 amostras por segundo, guardamos so a maior.
  if (millis() - ultimoMPU >= INTERVALO_MPU) {
    ultimoMPU = millis();
    accel = ESP32Sensors::Accel::medirAccel();
    movimentacaoMax = fmaxf(movimentacaoMax, ESP32Sensors::Accel::medirMovimentacao(accel));
  }

  // O HC-SR04 e lido junto do envio: a distancia nao muda em milissegundos.
  if (millis() - tempoAnterior >= INTERVALO_COLETA) {
    tempoAnterior = millis();
    distancia = ESP32Sensors::Distancia::medirDistancia();
    enviarDadosColetados();
    movimentacaoMax = NAN;   // recomeca a procurar o pico
  }

  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  } else if (!mqttClient.connected()) {
    conectarMQTT();
  } else {
    mqttClient.loop();
  }
}

void conectarWiFi() {
  // Uma tentativa a cada INTERVALO_RECONEXAO, como no app14.
  if (millis() - ultimaTentativaWiFi < INTERVALO_RECONEXAO) {
    return;
  }
  ultimaTentativaWiFi = millis();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_SENHA);
  WiFi.setSleep(false);

  // Aguarda conectar para mostrar o IP obtido nesta conexão.
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.println("[WiFi] Conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void conectarMQTT() {
  if (millis() - ultimaTentativaMQTT < INTERVALO_RECONEXAO) {
    return;
  }
  ultimaTentativaMQTT = millis();

  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println("[MQTT] Conectado");
  } else {
    Serial.printf("[MQTT] Falha: %d\r\n", mqttClient.state());
  }
}

bool enviarDadosColetados() {
  Serial.printf("%.1f,%.1f,%.1f,%.2f\r\n",
                ambiente.temp, ambiente.umid, distancia, movimentacaoMax);

  JsonDocument doc;
  doc["device"] = DISPOSITIVO_ID;
  doc["temp"] = ambiente.temp;
  doc["umid"] = ambiente.umid;
  doc["dist"] = distancia;
  doc["accel_x"] = accel.accelX;
  doc["accel_y"] = accel.accelY;
  doc["accel_z"] = accel.accelZ;
  doc["movimentacao"] = movimentacaoMax;

  String payload;
  serializeJson(doc, payload);
  if (!mqttClient.connected()) {
    return false;
  }
  bool ok = mqttClient.publish(MQTT_PUB_TOPIC, payload.c_str());
  Serial.println(ok ? "[MQTT] Publicado" : "[MQTT] Falha ao publicar");
  return ok;
}
