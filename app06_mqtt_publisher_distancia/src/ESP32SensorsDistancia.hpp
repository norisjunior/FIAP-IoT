#pragma once
#include <Arduino.h>

namespace ESP32Sensors {
  namespace Distancia {

    struct DISTANCIA {
      float cm;
      uint32_t duracaoUs;
      bool valido;
    };

    static uint8_t trigPin = 0;
    static uint8_t echoPin = 0;
    static uint32_t tempoLimite = 30000;

    void inicializar(uint8_t tPin, uint8_t ePin, uint32_t limiteUs = 30000) {
      trigPin = tPin;
      echoPin = ePin;
      tempoLimite = limiteUs;
      pinMode(trigPin, OUTPUT);
      pinMode(echoPin, INPUT);
    }

    DISTANCIA medirDistancia() {
      // Etapa 1: garante que o TRIG esteja em nível baixo.
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);

      // Etapa 2: dispara o sensor com um pulso de 10 microssegundos.
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);

      // Etapa 3: mede o tempo do ECHO. Sem pulso, pulseIn devolve zero.
      uint32_t duracao = pulseIn(echoPin, HIGH, tempoLimite);

      // Etapa 4: calcula a distância. Dividimos por 2 por causa da ida e volta.
      DISTANCIA dados;
      dados.cm = duracao * 0.034f / 2;
      dados.duracaoUs = duracao;
      dados.valido = duracao > 0;
      return dados;
    }
  }
}
