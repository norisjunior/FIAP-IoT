/* *Includes* */
#include <Arduino.h>
#include "ESP32Sensors.hpp"

#define FREQUENCY_HZ 50
#define INTERVAL_MS (1000 / (FREQUENCY_HZ + 1))

/* ---- Config Sensores ---- */
const static uint8_t SCL_PIN   = 23;
const static uint8_t SDA_PIN   = 22;
const static uint8_t LED_R_PIN = 21;
const static uint8_t LED_B_PIN = 18;
const static uint8_t LED_G_PIN = 19;

static unsigned long last_interval_ms = 0;

/* ---- MAIN (setup e loop) ---- */
void setup() {
  Serial.begin(115200);

  //Inicializa todos os sensores
  Serial.println("Inicializando sensores...");
  pinMode(LED_R_PIN, OUTPUT);
  pinMode(LED_B_PIN, OUTPUT);
  pinMode(LED_G_PIN, OUTPUT);

  // Liga e apaga LEDs
  for (int i = 0; i < 2; i++) {
    digitalWrite(LED_R_PIN, HIGH);
    digitalWrite(LED_B_PIN, HIGH);
    digitalWrite(LED_G_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_R_PIN, LOW);
    digitalWrite(LED_B_PIN, LOW);
    digitalWrite(LED_G_PIN, LOW);
  }

  ESP32Sensors::beginAll(SDA_PIN, SCL_PIN);

  Serial.println("\nPronto para caturar movimentos!");
}

void loop() {
  if (millis() > last_interval_ms + INTERVAL_MS) {
    last_interval_ms = millis();

    ESP32Sensors::AccelGyro::DADOS dadosMov = ESP32Sensors::AccelGyro::medirAccelGyro();
    
    // Acelerômetro em m/s²
    Serial.print(dadosMov.accel.acceleration.x, 6); Serial.print(',');
    Serial.print(dadosMov.accel.acceleration.y, 6); Serial.print(',');
    Serial.print(dadosMov.accel.acceleration.z, 6); Serial.print(',');
    
    // Giroscópio em graus por segundo (dps)
    Serial.print(dadosMov.gyro.gyro.x, 6); Serial.print(',');
    Serial.print(dadosMov.gyro.gyro.y, 6); Serial.print(',');
    Serial.println(dadosMov.gyro.gyro.z, 6);
  }

  delay(1);
}


