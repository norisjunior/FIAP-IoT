#pragma once
#include <Arduino.h>

namespace ESP32Sensors {
  namespace LDR {
    static uint8_t ldrPin = 0;

    void inicializar(uint8_t pin) {
      ldrPin = pin;
      analogReadResolution(12);
      pinMode(pin, INPUT);
    }

    int ler() {
      return analogRead(ldrPin);
    }
  }
}
