#pragma once

#include <Arduino.h>
#include <DHT.h>

// Temperatura e umidade do ambiente em volta da caixa (DHT22).
namespace AmbienteExterno {
  // Unico limiar que sobrou no firmware: ele acende o LED.
  // Um sensor responde sozinho -> um if resolve, nao precisa de ML.
  const float LIMIAR_AMBIENTE_HOSTIL = 30.0;

  DHT dht(0, DHT22);

  void inicializar(int pinoDados) {
    dht = DHT(pinoDados, DHT22);
    dht.begin();
  }

  float lerTemperatura() {
    return dht.readTemperature();
  }

  float lerUmidade() {
    return dht.readHumidity();
  }

  bool leituraValida(float temperatura, float umidade) {
    return !isnan(temperatura) && !isnan(umidade);
  }

  bool ambienteHostil(float temperatura) {
    return temperatura > LIMIAR_AMBIENTE_HOSTIL;
  }
}
