/* *Includes* */
//WiFi
#include <WiFi.h>
#include <WiFiClient.h>
//WebServer
#include <WebServer.h>
//ESP32Sensors
#include "ESP32Sensors.hpp"
//JSON
#include <Arduino.h>
#include <ArduinoJson.h>

//WebServer
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* host = "host.wokwi.internal"; // IP do servidor Python
const uint16_t port = 5000;

//Dispositivo
const char* device = "ESP32Noris";

void setup() {
  Serial.begin(115200);

  //Inicializa todos os sensores
  ESP32Sensors::beginAll();

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
}

void loop() {
  //Mede temperatura, umidade e índice de calor  
  ESP32Sensors::Ambiente::AMBIENTE salaMaquinas = ESP32Sensors::Ambiente::medirAmbiente();
  
  //Mede trepidação/movimentação do motor (eixos x, y e z)
  sensors_event_t motor = ESP32Sensors::Accel::medirAccel();
    
  WiFiClient client;
  if (client.connect(host, port)) {
      
    /* Transmissão dos dados em formato JSON */
    JsonDocument doc;

    doc["device"] = device;
    doc["temp"] = salaMaquinas.temp;
    doc["umid"] = salaMaquinas.umid;
    doc["hic"] = salaMaquinas.hic;
    doc["motor_accel_x"] = motor.acceleration.x;
    doc["motor_accel_y"] = motor.acceleration.y;
    doc["motor_accel_z"] = motor.acceleration.z;

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
