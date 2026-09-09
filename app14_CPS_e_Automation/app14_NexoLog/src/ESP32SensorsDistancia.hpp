#pragma once
#include <Arduino.h>

namespace ESP32Sensors {
  namespace Distancia {
    static uint8_t trigPin = 0;
    static uint8_t echoPin = 0;

    struct DISTANCIA {
      float cm;
      bool valido;
    };

    void inicializar(uint8_t tPin, uint8_t ePin) {
      trigPin = tPin;
      echoPin = ePin;
      pinMode(trigPin, OUTPUT);
      pinMode(echoPin, INPUT);
    }

    DISTANCIA medirDistancia() {
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);

      unsigned long duracao = pulseIn(echoPin, HIGH, 30000);
      return {duracao ? duracao * 0.034f / 2 : NAN, duracao > 0};
    }
  }
}
