/* ---- Bibliotecas ---- */
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "client_smart_objects.h"

#include <DHT.h>

/* ---- Config Wi‑Fi ---- */
const char* ssid     = "Wokwi-GUEST";
const char* password = "";

/* ---- DHT e LEDs ---- */
const uint8_t LED_PINS[3] = {15, 16, 17};
const uint8_t DHT_PIN = 19;
DHT dht(DHT_PIN, DHT22);

void setup()
{
  Serial.begin(115200);

  //Inicializa WiFi
  WiFi.begin(ssid, password);
  Serial.println("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");

  delay(5000);

  LwM2m.init();
}

void loop()
{
  LwM2m.lifecycle();
}
