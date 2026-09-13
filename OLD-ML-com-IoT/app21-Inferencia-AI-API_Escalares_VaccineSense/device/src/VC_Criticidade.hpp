#pragma once

#include <Arduino.h>

// Criticidade da carga escolhida no potenciometro (0 a 100).
// Nao e uma medicao do mundo: e um ajuste do operador que muda a faixa aceita.
namespace Criticidade {
  int pinoCriticidade = 0;

  void inicializar(int pinoAnalogico) {
    pinoCriticidade = pinoAnalogico;
    pinMode(pinoCriticidade, INPUT);
  }

  int lerNivel() {
    int leitura = analogRead(pinoCriticidade);
    return map(leitura, 0, 4095, 0, 100);
  }
}
