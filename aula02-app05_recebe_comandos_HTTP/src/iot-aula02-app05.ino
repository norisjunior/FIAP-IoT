/* *Includes* */
//WiFi
#include <WiFi.h>
#include <WiFiClient.h>
//WebServer
#include <WebServer.h>
//ESP32Sensors
#include "ESP32Sensors.hpp"

//WebServer
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
const char* HTTPCHARSET = "text/plain; charset=utf-8";
const uint8_t WIFI_CHANNEL = 6;
WebServer server(80);

// -------------- Handlers HTTP
void handleRoot() {
  String page = "*** Bem-vindo ao Servidor ESP32 que recebe comandos HTTP ***\n\n";
  page += "Rotas disponíveis:\n";
  page += " - /led/on ------> Acende o LED\n";
  page += " - /led/off -----> Apaga o LED\n";
  page += " - /play --------> toca um som (exemplo: play?freq=1000&dur=500)\n";
  server.send(200, HTTPCHARSET, page);
}

void handleLedOn() {
  ESP32Sensors::LED::on();
  server.send(200, HTTPCHARSET, "LED ON");
}

void handleLedOff() {
  ESP32Sensors::LED::off();
  server.send(200, HTTPCHARSET, "LED OFF");
}

void handleAudio() {
  // /play?freq=1000&dur=500
  uint16_t f = server.hasArg("freq") ? server.arg("freq").toInt() : 1000;
  uint16_t d = server.hasArg("dur")  ? server.arg("dur").toInt()  : 500;
  ESP32Sensors::Audio::tocar(f, d);
  server.send(200, HTTPCHARSET, "Sinal sonoro tocado");
}


// --------------- Funções setup() e loop()
void setup() {
  Serial.begin(115200);

  //Inicializa todos os sensores
  ESP32Sensors::beginAll();

  //Conexão WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  // Rotas
  server.on("/", handleRoot);
  server.on("/led/on", handleLedOn);
  server.on("/led/off", handleLedOff);
  server.on("/play", handleAudio);
  server.begin();
}

void loop() {
  server.handleClient(); // processa requisições
  delay(10);
}
