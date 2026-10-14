/* *Includes* */
//WiFi
#include <WiFi.h>
#include <WiFiClient.h>
//WebServer
#include <WebServer.h>
//ESP32Sensors
#include "ESP32SensorsAudio.hpp"
#include "ESP32SensorsLED.hpp"

// Pinos
#define BUZZER_PIN 27
#define LED_PIN    26

//WebServer
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_SENHA = "";
const char* HTTPCHARSET = "text/plain; charset=utf-8";
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
// Controle da conexão
const uint32_t INTERVALO_RECONEXAO = 5000;
uint32_t ultimaTentativaWiFi = 0;

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

void setup() {
  Serial.begin(115200);

  //Inicializa todos os sensores
  ESP32Sensors::Audio::inicializar(BUZZER_PIN);
  ESP32Sensors::LED::inicializar(LED_PIN);

  //Conexão WiFi
  // Aguarda o intervalo inicial antes de iniciar o serviço com WiFi conectado.
  delay(INTERVALO_RECONEXAO);
  conectarWiFi();

  // Rotas
  server.on("/", handleRoot);
  server.on("/led/on", handleLedOn);
  server.on("/led/off", handleLedOff);
  server.on("/play", handleAudio);
  server.begin();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

  server.handleClient(); // processa requisições
  delay(10);
}
