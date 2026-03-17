/* *Includes* */
#include <WiFi.h>
#include <WiFiClient.h>
#include "IoTSemaforo.hpp"
#include <PubSubClient.h>
//#include <MqttClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>

/* ---- Config Wi‑Fi ---- */
const char* WIFI_SSID     = "Wokwi-GUEST";   // Rede pública do simulador
const char* WIFI_PASSWORD = "";
WiFiClient wifiClient; // Define client WiFi

/* ---- Config MQTT ---- */
#define MQTT_HOST       "host.wokwi.internal" //IP do Broker test.mosquitto.org
#define MQTT_PORT       1883
#define MQTT_SUB_TOPIC  "semaforo/distancia"
#define MQTT_DEVICEID   "cloud"

PubSubClient mqttClient(wifiClient);
bool wifiConectado = false;
bool mqttConectado = false;

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

/* ---- Funções (protótipos) ---- */

void conectarWiFi();
void conectarMQTT();
void mensagemRecebida(char* topico, byte* msg, unsigned int length);

void setup() {
  Serial.begin(115200);

  //Inicializa os LEDs do semáforo
  semaforo.inicializar();

  //Inicializa WiFi
  //Conecta no WiFi
  conectarWiFi();

  //Configura MQTT e conecta no Broker
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setKeepAlive(60);     // Manter conexão viva por 60s
  mqttClient.setSocketTimeout(60); // Timeout de 60s
  mqttClient.setBufferSize(1024);   // Buffer adequado considerando o payload gerado
  /* Função mensagemRecebida será chamada sempre que um
     novo publish for enviado pelo Broker */
  mqttClient.setCallback(mensagemRecebida);

  Serial.println("Semáforo inteligente iniciado.\nAguardando recebimento de mensagem MQTT.\n");

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

  /* ---- Expiração da janela ---- */
  if ((deteccoes > 0) && ((millis() - tempoInicio) > tempoMax)) {
      deteccoes = 0;                     // zera se passou 60 s
      tempoInicio = 0;
  }

  semaforo.faseVerde();                  // Volta para o verde (mantém sempre verde depois de fechar)

  delay(500);
}



/* ==== WIFI ================================================================ */
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

/* ==== MQTT ================================================================ */
void conectarMQTT() {
  if (!wifiConectado) {
    Serial.println("[MQTT] Sem Wi-Fi.");
    return;
  }

  Serial.print("[MQTT] Conectando ao broker ");
  Serial.print(MQTT_HOST);
  Serial.print(" ... ");

  int tentativas = 0;
  while (!mqttClient.connected() && tentativas < 5) {
    if (mqttClient.connect(MQTT_DEVICEID, MQTT_DEVICEID, MQTT_PASS)) {
      Serial.println("OK!");
      mqttConectado = true;

      // Inscreve no tópico coringa
      if (mqttClient.subscribe(MQTT_SUB_TOPIC)) {
        Serial.print("[MQTT] Inscrito em: ");
        Serial.println(MQTT_SUB_TOPIC);
      } else {
        Serial.println("[MQTT] Falha ao se inscrever no tópico de comandos.");
      }

    } else {
      Serial.print("Falha, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" tentando de novo...");
      tentativas++;
      delay(1000);
    }
  }

  if (!mqttClient.connected()) {
    Serial.println("[MQTT] Não foi possível conectar ao broker.");
  }
}


/* ---- Callback MQTT ---- */
void mensagemRecebida(char* topico, byte* msg, unsigned int length) {
  /* ---- Obtenção do tópico e payload */
  String topic = String(topico);
  String payload;
  for (unsigned int i = 0; i < length; i++) {
    payload += (char)msg[i];
  }
  Serial.printf("Mensagem MQTT recebida [%s]: %s", topic.c_str(), payload.c_str()); Serial.println("\n");

  /* ---- Desserializa JSON e valida distância ---- */
  JsonDocument doc;
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
