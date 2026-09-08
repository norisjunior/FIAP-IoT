#include <Arduino.h>

#define MIC_PIN_KY  34
#define MIC_PIN_MAX 35

void setup() {
  Serial.begin(115200);
}

void loop() {
  int mic_ky = analogRead(MIC_PIN_KY);
  int mic_max = analogRead(MIC_PIN_MAX);

  Serial.printf(">som_ky: %d\n",mic_ky); Serial.println("");
  Serial.printf(">som_max: %d\n",mic_max); Serial.println("");

  delay(500); // amostragem mais rápida
}