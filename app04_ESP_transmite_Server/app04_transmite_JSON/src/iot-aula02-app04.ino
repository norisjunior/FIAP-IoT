/* *Includes* */
//WiFi
#include <WiFi.h>
#include <WiFiClient.h>
//ESP32Sensors
#ifndef MPU_TYPE
#define MPU_TYPE MPU6050  // Wokwi; troque para MPU6500 no kit com esse modelo
#endif
#include "ESP32SensorsAmbiente.hpp"
#include "ESP32SensorsAccel.hpp"
#include "ESP32SensorsLED.hpp"

// Pinos
#define DHT_PIN   27
#define DHT_MODEL DHT22
#define LED_PIN   26
#define SDA_PIN   18
#define SCL_PIN   19
//JSON
#include <Arduino.h>
#include <ArduinoJson.h>

//WiFi e servidor Python
// Wokwi: rede aberta. No kit físico, informe o SSID e a senha da rede.
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_SENHA = "";
const char* host = "host.wokwi.internal"; // Wokwi no VS Code: o seu computador. ESP32 físico: IP do computador com o servidor Python
const uint16_t port = 5000;

//Dispositivo
const char* device = "ESP32Noris";

// Controle da conexão, como no app14
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
  ESP32Sensors::Ambiente::inicializar(DHT_PIN, DHT_MODEL);
  ESP32Sensors::Accel::inicializar(SDA_PIN, SCL_PIN);
  ESP32Sensors::LED::inicializar(LED_PIN);

  conectarWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

  //Mede temperatura, umidade e índice de calor  
  ESP32Sensors::Ambiente::AMBIENTE salaMaquinas = ESP32Sensors::Ambiente::medirAmbiente();
  
  //Mede trepidação/movimentação do motor (eixos x, y e z)
  ESP32Sensors::Accel::ACCEL motor = ESP32Sensors::Accel::medirAccel();
    
  WiFiClient client;
  if (WiFi.status() == WL_CONNECTED && client.connect(host, port)) {
      
    /* Transmissão dos dados em formato JSON */
    JsonDocument doc;

    doc["device"] = device;
    doc["temp"] = salaMaquinas.temp;
    doc["umid"] = salaMaquinas.umid;
    doc["hic"] = salaMaquinas.ic;
    doc["motor_accel_x"] = motor.x;
    doc["motor_accel_y"] = motor.y;
    doc["motor_accel_z"] = motor.z;

    String output;
    serializeJson(doc, output);
    client.println(output);
    Serial.println("Dados enviados: " + output);
    client.stop();
  } else {
    Serial.println("Falha ao conectar no servidor");
  }

  delay(5000); // Aguarda 5s
}
