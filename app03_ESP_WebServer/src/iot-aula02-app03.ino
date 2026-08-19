//Includes
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include "ESP32SensorsAmbiente.hpp"
#include "ESP32SensorsAccel.hpp"
#include "ESP32SensorsLED.hpp"

// Pinos
#define DHT_PIN   27
#define DHT_MODEL DHT22
#define LED_PIN   14
#define SDA_PIN   18
#define SCL_PIN   19

//WebServer
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
const char* HTTPCHARSET = "text/plain; charset=utf-8";
//const uint8_t WIFI_CHANNEL = 6;
WebServer server(80);

//Coleta medições ambientais (temperatura e umidade)
void handleAmbiente() {
  ESP32Sensors::Ambiente::AMBIENTE leitura = ESP32Sensors::Ambiente::medirAmbiente();

  if (!leitura.valido) {
    server.send(500, HTTPCHARSET, "Erro ao ler ambiente: aguarde 2 segundos ou verifique o sensor DHT.");
    delay(100);
  } else {
    server.send(200, HTTPCHARSET, "Temp: " + String(leitura.temp) + " °C | Umid: " + String(leitura.umid) + "%");
  }
}

void handleAccel() {
  //g é aproximadamente 9.80665 
  //para transformar de m/s² para g (gravidade), basta dividir o resultado por 9.80665
  sensors_event_t a = ESP32Sensors::Accel::medirAccel();
  server.send(200, HTTPCHARSET, "Acelerômetro: x: " + String(a.acceleration.x/9.80665) + "g, y: " + String(a.acceleration.y) + "m/s², z: " + String(a.acceleration.z) + "m/s².");
  delay(100);
}

void setup() {
  Serial.begin(115200);

  //Inicializa todos os sensores
  ESP32Sensors::Ambiente::inicializar(DHT_PIN, DHT_MODEL);
  ESP32Sensors::Accel::inicializar(SDA_PIN, SCL_PIN);
  ESP32Sensors::LED::inicializar(LED_PIN);

  //Conexão WiFi
  //WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
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
  server.on("/ambiente", handleAmbiente);
  server.on("/accel", handleAccel);

  //Página inicial
  server.on("/", []() {
    String page = "*** Bem-vindo ao Servidor ESP32! ***\n\n";
    page += "Rotas disponíveis:\n";
    page += " - /ambiente ------> temperatura em °C\n";
    page += " - /accel ---------> acelerômetro eixos x, y e z em m/s²\n";
    server.send(200, HTTPCHARSET, page);
  });

  server.begin();
  Serial.println("Servidor iniciado! (http://localhost:8180)");
}

void loop() {
  server.handleClient();
  delay(10);
}
