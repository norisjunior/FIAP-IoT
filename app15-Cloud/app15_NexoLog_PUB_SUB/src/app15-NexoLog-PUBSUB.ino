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
const uint8_t LED_ALERTA = 21;

/* ---- Config Wi-Fi e MQTT ---- */
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
#define MQTT_SERVER "host.wokwi.internal"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "NexoLogEquipe01"
#define MQTT_PUB_TOPIC "FIAPIoT/nexolog/equipe01/dados"
#define MQTT_SUB_TOPIC "FIAPIoT/nexolog/equipe01/cmd"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

/* ---- Controle de intervalo ---- */
const unsigned long INTERVALO_COLETA = 1000;   // publica e le o ultrassonico
const unsigned long INTERVALO_DHT = 2100;      // o DHT22 nao responde mais rapido
const unsigned long INTERVALO_MPU = 50;        // 20 amostras por segundo
const unsigned long INTERVALO_RECONEXAO = 5000;
unsigned long tempoAnterior = 0, ultimoDHT = 0, ultimoMPU = 0;
unsigned long proximaTentativaWiFi = 0, proximaTentativaMQTT = 0;

/* ---- Medicoes guardadas entre um envio e outro ---- */
ESP32Sensors::Ambiente::AMBIENTE ambiente = {NAN, NAN, NAN, false};
float distancia = NAN;
AccelData accel = {};
float movimentacaoMax = NAN;

/* ---- Protótipos ---- */
void conectarWiFi();
void conectarMQTT();
bool enviarDadosColetados();
void callbackMQTT(char* topico, byte* conteudo, unsigned int tamanho);

void setup() {
  Serial.begin(115200);
  ESP32Sensors::Ambiente::inicializar(DHT_PIN, DHT_MODEL);
  ESP32Sensors::Distancia::inicializar(TRIG_PIN, ECHO_PIN);
  ESP32Sensors::Accel::inicializar(SCL_PIN, SDA_PIN);
  pinMode(LED_ALERTA, OUTPUT);
  digitalWrite(LED_ALERTA, LOW);

  conectarWiFi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(callbackMQTT);
  mqttClient.setBufferSize(768);
  mqttClient.setKeepAlive(120);

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
  // Uma tentativa a cada INTERVALO_RECONEXAO.
  if (millis() < proximaTentativaWiFi) {
    return;
  }
  proximaTentativaWiFi = millis() + INTERVALO_RECONEXAO;

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setSleep(false);
}

void conectarMQTT() {
  if (millis() < proximaTentativaMQTT) {
    return;
  }
  proximaTentativaMQTT = millis() + INTERVALO_RECONEXAO;

  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println("[MQTT] Conectado");
    mqttClient.subscribe(MQTT_SUB_TOPIC);
  } else {
    Serial.printf("[MQTT] Falha: %d\r\n", mqttClient.state());
  }
}

bool enviarDadosColetados() {
  Serial.printf("%.1f,%.1f,%.1f,%.2f\r\n",
                ambiente.temp, ambiente.umid, distancia, movimentacaoMax);

  JsonDocument doc;
  doc["device"] = MQTT_CLIENT_ID;
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

void callbackMQTT(char* topico, byte* conteudo, unsigned int tamanho) {
  // %.*s imprime so os primeiros "tamanho" caracteres: conteudo nao termina em \0.
  Serial.printf("[MQTT] Recebido: %.*s\r\n", tamanho, (const char*)conteudo);

  JsonDocument doc;
  DeserializationError erro = deserializeJson(doc, (const char*)conteudo, tamanho);
  if (erro) {
    Serial.printf("[CMD] JSON invalido: %s\r\n", erro.c_str());
    return;
  }

  const char* alerta = doc["alerta"];
  if (alerta == nullptr) {
    Serial.println("[CMD] Faltou o campo alerta");
    return;
  }

  // A nuvem ja decidiu. Aqui so obedecemos.
  digitalWrite(LED_ALERTA, strcmp(alerta, "ON") == 0 ? HIGH : LOW);
}
