#include <Arduino.h>

#define MIC_PIN 35

int centro = 0;

void calibrarCentro() {
  long soma = 0;
  const int amostras = 500;

  for (int i = 0; i < amostras; i++) {
    soma += analogRead(MIC_PIN);
    delay(2);
  }

  centro = soma / amostras;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Calibrando ponto central das medições de som...");
  delay(1000);
  calibrarCentro();
  Serial.printf("Centro calibrado: %d\n", centro); Serial.println("");
}

void loop() {
  int raw = analogRead(MIC_PIN);
  int amplitude = abs(raw - centro);

  Serial.printf(">raw:%d\n>amp:%d\n", raw, amplitude); Serial.println("");
  delayMicroseconds(500);
}