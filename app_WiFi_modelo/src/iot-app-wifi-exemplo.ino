/* *Includes* */
#include <WiFi.h>
#include <WiFiClient.h>
#include "ESP32SensorsDistancia.hpp"
#include <ArduinoJson.h>
// TODO Passo 2: incluir a biblioteca PubSubClient

/* ---- Config Wi‑Fi ---- */
const char* WIFI_SSID     = "Wokwi-GUEST";
const char* WIFI_SENHA = "";
WiFiClient wifiClient; // Define client WiFi

// TODO Passo 3: definir MQTT_BROKER, MQTT_PORTA, MQTT_PUB_TOPIC, MQTT_CLIENT_ID

// TODO Passo 3: criar mqttClient e declarar ultimaTentativaMQTT

// SE FOR SUBSCRIBER:
// TODO Passo 8: criar a função callback (DEVE ser declarada ANTES do setup)
//   - montar a string da mensagem recebida
//   - parsear o JSON com deserializeJson
//   - extrair o valor recebido
//   - decidir o que fazer (ex: se dist < 10, acender o LED)


/* ---- Config sensor ---- */
#define TRIG_PIN 25
#define ECHO_PIN 26

/* ---- Config intervalo temporal ---- */
const uint32_t INTERVALO = 5000; // Intervalo para coleta de medições
uint32_t tempo_anterior = 0;

// Controle da conexão
const uint32_t INTERVALO_RECONEXAO = 5000;
uint32_t ultimaTentativaWiFi = 0;

/* ---- Funções - início ---- */
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

// TODO Passo 4: criar a função conectarMQTT()
// SE FOR SUBSCRIBER: Lembre-se que após o connect bem-sucedido, faça o subscribe

/* ---- Funções - fim ---- */



void setup() {
  Serial.begin(115200);

  //Inicializa sensores (distância nesse exemplo)
  ESP32Sensors::Distancia::inicializar(TRIG_PIN, ECHO_PIN);

  //Inicializa WiFi
  conectarWiFi();

  // TODO Passo 5: apontar o client para o broker (setServer)
  // SE FOR SUBSCRIBER:
  // TODO Passo 7a: registrar a função de callback (setCallback)

}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

  // TODO Passo 9: completar o else if de MQTT e chamar mqttClient.loop()

  if (millis() - tempo_anterior >= INTERVALO) {
    tempo_anterior = millis();

    //Mede a distância
    float dist = ESP32Sensors::Distancia::medirDist();
    
    Serial.println("\nExemplo de impressão \"simples\" com Serial.println:");
    Serial.println("Distância: " + String(dist) + " cm");

    Serial.println("\nExemplo de impressão \"simples\" com Serial.printf:");
    Serial.printf("Distância: %.2f cm", dist); Serial.println("");

    Serial.println("\nCriação de JSON e impressão no Monitor Serial:");
    JsonDocument doc;
    doc["dist"] = dist;
    char buffer[50]; // Crie um buffer do tamanho esperado do que será armazenado
    serializeJson(doc, buffer);
    Serial.println("JSON: ");
    Serial.println(buffer);

    // TODO Passo 6: publicar o buffer no tópico MQTT

  }

  delay(100);
}
