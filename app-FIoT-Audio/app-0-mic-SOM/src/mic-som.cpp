// Etapa 0 — o microfone produz numeros.
// Usa a biblioteca I2S que ja vem no core do ESP32.

#include <Arduino.h>
#include <I2S.h>

void setup() {
  Serial.begin(115200);

  I2S.setAllPins(26, 25, 33, -1, 33);      // SCK, WS, SD
  I2S.begin(I2S_PHILIPS_MODE, 16000, 32);  // 16000 amostras/s, 32 bits
}

void loop() {
  int som = I2S.read();

  Serial.printf(">som:%d\n", som);
}
