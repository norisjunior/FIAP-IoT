/* etapa1 — placa viva: Serial USB e LED */
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>  // Serial USB no core Seeed nRF52

bool led = false;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {}  // espera o monitor por até 3 s
  Serial.println("XIAO nRF52840 Sense Plus: placa viva");
}

void loop() {
  led = !led;
  digitalWrite(LED_BUILTIN, led);  // pisca o LED vermelho
  Serial.print("t = ");
  Serial.print(millis());
  Serial.println(" ms");
  delay(1000);
}
