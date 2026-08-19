/* *Includes* */
#include <WiFi.h>
#include <WiFiClient.h>
#include "IoTSemaforo.hpp"
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
#define MQTT_SUB_TOPIC  "noris/semaforo1/distancia"
#define MQTT_DEVICEID   "NorisSemaforo00001"
#define MQTT_QOS        1
//PubSubClient client(wifiClient); // Define client PubSub (MQTT client)
MqttClient mqttClient(wifiClient);

/* ---- Semáforo - definições ---- */
#define LED_VERMELHO 18
#define LED_AMARELO  17
#define LED_VERDE    16
SemaforoLeds semaforo(LED_VERMELHO, LED_AMARELO, LED_VERDE);

/* ---- Lógica de contagem ---- */
uint8_t deteccoes = 0;
int32_t tempoInicio = 0;
const int tempoMax = 60000; // 60 s
const int PESSOA_DIST = 50;

/* ---- Função: Conectar ao MQTT Broker ---- */
void conectarMQTT() {
  Serial.print("Conectando ao MQTT Broker" + String(MQTT_HOST));
  while (!mqttClient.connected()) {
    if (mqttClient.connect(MQTT_HOST, MQTT_PORT)) {
      Serial.println(" Conectado ao MQTT!");
      mqttClient.subscribe(MQTT_SUB_TOPIC, MQTT_QOS);
      Serial.println("Subscribe realizado no tópico: " + String(MQTT_SUB_TOPIC) + ", QoS " + String(MQTT_QOS));
    } else {
      Serial.print(" Falha, rc=");
      Serial.print(mqttClient.connectError());
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }  
}

/* ---- Callback MQTT ---- */
void mensagemRecebida(int messageSize) {
  /* ---- Obtenção do tópico e payload */
  String topic = mqttClient.messageTopic();
  String payload;
  while (mqttClient.available()) {
    payload += (char)mqttClient.read();
  }
  Serial.printf("Mensagem MQTT recebida [%s]: %s", topic.c_str(), payload.c_str()); Serial.println("\n");

  /* ---- Desserializa JSON e valida distância ---- */
  StaticJsonDocument<200> doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) { // JSON inválido
    delay(500);
    return;
  }

  float dist = doc["dist"].as<float>();
  if (dist >= PESSOA_DIST) {  // não conta caso a distância enviada seja maior que 50cm
    delay(500);
    return;
  }

  /* ---- Conta detecções ---- */
  deteccoes++;
  if (deteccoes == 1) {
    tempoInicio = millis();              //inicia a janela temporal de 60 segundos quando a primeira pessoa chega
  }

  /* ---- Se >= 2 detecções em 60s: fase vermelha ---- */
  if (deteccoes >= 2 && ((millis() - tempoInicio) <= tempoMax)) {
    Serial.println("Gatilho atingido! Preparando fase vermelha do semáforo...");
    delay(500);
    semaforo.faseAmarela();
    delay(2000);
    semaforo.faseVermelha();
    delay(5000);
    deteccoes = 0;                     // reinicia a contagem de detecções
    tempoInicio = millis();            // reinicia a janela temporal
    Serial.println("Semáforo pronto. Aguardando novas medições...\n");
  }
}


void setup() {
  Serial.begin(115200);

  //Inicializa os LEDs do semáforo
  semaforo.inicializar();

  //Inicializa WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");

  //ClientID
  mqttClient.setId(MQTT_DEVICEID);
  //Keep Alive (60 segundos)
  mqttClient.setKeepAliveInterval(60);
  /* Função mensagemRecebida será chamada sempre que um
     novo publish for enviado pelo Broker */
  mqttClient.onMessage(mensagemRecebida);

  Serial.println("Semáforo inteligente iniciado.\nAguardando recebimento de mensagem MQTT.\n");
}

void loop() {
  if (!mqttClient.connected()) {
    conectarMQTT();
  }
  mqttClient.poll();

  /* ---- Expiração da janela ---- */
  if ((deteccoes > 0) && ((millis() - tempoInicio) > tempoMax)) {
      deteccoes = 0;                     // zera se passou 60 s
      tempoInicio = 0;
  }

  semaforo.faseVerde();                  // Volta para o verde (mantém sempre verde depois de fechar)

  delay(500);
}
