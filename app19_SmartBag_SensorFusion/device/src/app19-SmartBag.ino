/* ---- Includes ---- */
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "ESP32SensorsAmbiente.hpp"
#include "ESP32SensorsDistancia.hpp"
#include "ESP32SensorsAccel.hpp"
#include "ESP32SensorsLDR.hpp"
#include "ESP32SensorsLED.hpp"

/* ---- Config Hardware ---- */
const uint8_t DHT_PIN = 4;
const uint8_t DHT_MODEL = DHT22;
const uint8_t TRIG_PIN = 19;
const uint8_t ECHO_PIN = 18;
const uint8_t SCL_PIN = 23;
const uint8_t SDA_PIN = 22;
const uint8_t LED_PIN = 21;
const uint8_t LDR_PIN = 35;
const uint8_t BTN_COLETA = 27;
const uint8_t BTN_SITUACAO = 26;

/* ---- Config Wi-Fi e MQTT ---- */
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
#define MQTT_SERVER "host.wokwi.internal"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "SmartBagEquipe01"
#define MQTT_PUB_TOPIC "FIAPIoT/smartbag/equipe01/dados"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

/* ---- Controle de intervalo ---- */
const unsigned long INTERVALO_COLETA = 1000;
const unsigned long INTERVALO_MPU = 50;
const unsigned long INTERVALO_DHT = 2500;
const unsigned long INTERVALO_RECONEXAO = 5000;
unsigned long tempoAnterior = 0, ultimoMPU = 0, ultimoDHT = 0;
unsigned long ultimaTentativaWiFi = 0, ultimaTentativaMQTT = 0;

/* ---- Coleta ---- */
ESP32Sensors::Ambiente::AMBIENTE ambiente = {NAN, NAN, NAN, false};
float distBase = NAN, movMax = NAN, inclMax = NAN;
bool coletando = false;
String situacao = "";
uint32_t rodada = 1;
uint8_t indiceSituacao = 0;
bool baselinePendente = true;
const char* SITUACOES[] = {
  "parada_fechada", "transporte_normal", "buraco", "aberta_parada",
  "aberta_movimento", "tombamento", "problema_termico"
};
const uint8_t TOTAL_SITUACOES = sizeof(SITUACOES) / sizeof(SITUACOES[0]);

/* ---- Botoes ---- */
int ultimoBotaoColeta = HIGH, ultimoBotaoSituacao = HIGH;
unsigned long ultimoDebounceColeta = 0, ultimoDebounceSituacao = 0;
const unsigned long debounceMs = 300;

void calibrarDistancia();
void conectarWiFi();
void conectarMQTT();
void enviarDadosColetados();

void setup() {
  Serial.begin(115200);
  pinMode(BTN_COLETA, INPUT_PULLUP);
  pinMode(BTN_SITUACAO, INPUT_PULLUP);
  ESP32Sensors::Ambiente::inicializar(DHT_PIN, DHT_MODEL);
  ESP32Sensors::Distancia::inicializar(TRIG_PIN, ECHO_PIN);
  ESP32Sensors::Accel::inicializar(SCL_PIN, SDA_PIN);
  ESP32Sensors::LDR::inicializar(LDR_PIN);
  ESP32Sensors::LED::inicializar(LED_PIN);

  conectarWiFi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setBufferSize(1024);
  mqttClient.setKeepAlive(120);

  situacao = SITUACOES[0];
  Serial.println("Rodada 1: feche a bag antes de iniciar a coleta.");
  Serial.println("COLETA: inicia/para. SITUACAO: avanca com a coleta parada.");

  // Linha de titulo do CSV. Serial.println ja termina em \r\n .
  Serial.println("rodada,situacao,temperatura,umidade,delta_distancia,luz,mov_max,incl_max");
}

void loop() {
  // Botao COLETA: inicia/para a situacao selecionada.
  int leituraColeta = digitalRead(BTN_COLETA);
  if (ultimoBotaoColeta == HIGH && leituraColeta == LOW &&
      millis() - ultimoDebounceColeta > debounceMs) {
    if (!coletando) {
      if (baselinePendente) calibrarDistancia();
      movMax = inclMax = NAN;
      tempoAnterior = ultimoMPU = millis();
    }
    coletando = !coletando;
    if (coletando) ESP32Sensors::LED::on();
    else ESP32Sensors::LED::off();
    Serial.printf("[COLETA] %s | rodada %lu | %s\r\n",
                  coletando ? "INICIADA" : "PARADA", (unsigned long)rodada, situacao.c_str());
    ultimoDebounceColeta = millis();
  }
  ultimoBotaoColeta = leituraColeta;

  // Botao SITUACAO: muda o ensaio, sem iniciar a coleta.
  int leituraSituacao = digitalRead(BTN_SITUACAO);
  if (ultimoBotaoSituacao == HIGH && leituraSituacao == LOW &&
      millis() - ultimoDebounceSituacao > debounceMs) {
    if (!coletando) {
      indiceSituacao++;
      if (indiceSituacao >= TOTAL_SITUACOES) {
        indiceSituacao = 0;
        rodada++;
        distBase = NAN;
        baselinePendente = true;
        Serial.println("Nova rodada: feche a bag antes de iniciar a coleta.");
      }
      situacao = SITUACOES[indiceSituacao];
      Serial.printf("[SITUACAO] %s | rodada %lu\r\n", situacao.c_str(), (unsigned long)rodada);
    } else {
      Serial.println("Pare a coleta antes de trocar a situacao.");
    }
    ultimoDebounceSituacao = millis();
  }
  ultimoBotaoSituacao = leituraSituacao;

  // O DHT22 e lento: guardamos a ultima leitura boa e enviamos ela.
  unsigned long agora = millis();
  if (agora - ultimoDHT >= INTERVALO_DHT) {
    ultimoDHT = agora;
    ESP32Sensors::Ambiente::AMBIENTE leitura = ESP32Sensors::Ambiente::medirAmbiente();
    if (leitura.valido) ambiente = leitura;
  }

  // O MPU e rapido: 20 amostras por segundo, guardamos so os maiores.
  agora = millis();
  if (coletando && agora - ultimoMPU >= INTERVALO_MPU) {
    ultimoMPU = agora;
    AccelData accel = ESP32Sensors::Accel::medirAccel();
    movMax = fmaxf(movMax, ESP32Sensors::Accel::medirMovimentacao(accel));
    inclMax = fmaxf(inclMax, ESP32Sensors::Accel::medirInclinacao(accel));
  }

  // O HC-SR04 e lido junto do envio: a distancia nao muda em milissegundos.
  if (coletando && millis() - tempoAnterior >= INTERVALO_COLETA) {
    enviarDadosColetados();
    movMax = inclMax = NAN;   // recomeca a procurar os picos
    tempoAnterior = millis();
  }

  if (WiFi.status() != WL_CONNECTED) {
    if (millis() - ultimaTentativaWiFi >= INTERVALO_RECONEXAO) {
      ultimaTentativaWiFi = millis();
      WiFi.reconnect();
    }
  } else if (!mqttClient.connected()) {
    if (millis() - ultimaTentativaMQTT >= INTERVALO_RECONEXAO) {
      ultimaTentativaMQTT = millis();
      conectarMQTT();
    }
  } else {
    mqttClient.loop();
  }
}

void calibrarDistancia() {
  float soma = 0;
  int validas = 0;
  unsigned long inicio = millis();
  while (validas < 5 && millis() - inicio < 5000) {
    float dist = ESP32Sensors::Distancia::medirDistancia();
    if (isfinite(dist)) { soma += dist; validas++; }
    delay(60);
  }
  distBase = validas == 5 ? soma / validas : NAN;
  Serial.printf("[BASELINE] %.2f cm\r\n", distBase);
  baselinePendente = !isfinite(distBase);
  if (baselinePendente) Serial.println("[BASELINE] Sem medida. Feche a bag e tente na proxima coleta.");
}

void conectarWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setSleep(false);
}

void conectarMQTT() {
  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println("[MQTT] Conectado");
  } else {
    Serial.printf("[MQTT] Falha: %d\r\n", mqttClient.state());
  }
}

void enviarDadosColetados() {
  float dist = ESP32Sensors::Distancia::medirDistancia();
  int luz = ESP32Sensors::LDR::ler();

  // Mesma ordem da linha de titulo impressa no setup().
  Serial.printf("%lu,%s,%.1f,%.1f,%.2f,%d,%.2f,%.1f\r\n",
                (unsigned long)rodada, situacao.c_str(), ambiente.temp, ambiente.umid,
                dist - distBase, luz, movMax, inclMax);

  JsonDocument doc;
  doc["device"] = MQTT_CLIENT_ID;
  doc["rodada"] = rodada;
  doc["situacao"] = situacao;
  doc["temperatura"] = ambiente.temp;
  doc["umidade"] = ambiente.umid;
  doc["delta_distancia"] = dist - distBase;
  doc["luz"] = luz;
  doc["mov_max"] = movMax;
  doc["incl_max"] = inclMax;

  String payload;
  serializeJson(doc, payload);
  if (!mqttClient.connected() || !mqttClient.publish(MQTT_PUB_TOPIC, payload.c_str())) {
    Serial.println("[MQTT] Amostra nao enviada");
  }
}
