#pragma once

#include <Arduino.h>
#include <EasyUltrasonic.h>

// Distancia da tampa ate a carga (HC-SR04).
//
// Assim como a luz, nao ha limiar aqui: a faixa esperada depende do tamanho da
// caixa e de quanta carga foi colocada. Quem define isso e o Colab.
namespace Carga {
  EasyUltrasonic sensorDistancia;

  void inicializar(int pinoTrig, int pinoEcho) {
    sensorDistancia.attach(pinoTrig, pinoEcho);
  }

  float lerDistanciaCm() {
    return sensorDistancia.getDistanceCM();
  }
}
