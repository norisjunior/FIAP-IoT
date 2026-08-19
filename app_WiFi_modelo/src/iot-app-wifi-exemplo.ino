/* *Includes* */
#include <WiFi.h>
#include <WiFiClient.h>
#include "ESP32SensorsDistancia.hpp"
#include <ArduinoJson.h>
// TODO Passo 2: incluir a biblioteca PubSubClient

/* ---- Config Wi‑Fi ---- */
const char* ssid     = "Wokwi-GUEST";
const char* password = "";
WiFiClient wifiClient; // Define client WiFi

// TODO Passo 3: definir MQTT_HOST, MQTT_PORT, MQTT_PUB_TOPIC, MQTT_DEVICEID

// TODO Passo 3: criar o objeto PubSubClient usando wifiClient

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
int INTERVALO = 5000; // Intervalo para coleta de medições
uint64_t tempo_anterior = 0;

/* ---- Funções - início ---- */
void conectarWiFi() {
  Serial.print("Conectando ao WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Conectado!");
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

  // TODO Passo 9: verificar conexão MQTT e chamar client.loop()

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
