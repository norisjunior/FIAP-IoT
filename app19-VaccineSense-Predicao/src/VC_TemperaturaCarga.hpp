#pragma once

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Temperatura interna da carga (DS18B20).
namespace TemperaturaCarga {
  // Faixa da norma (PNI): +2 a +8 C.
  // Carga critica (criticidade > 50) usa uma faixa mais apertada.
  // Estes limiares alimentam o acumulador de exposicao termica no .ino.
  const float TEMP_MIN_SEGURA = 2.0;
  const float TEMP_MAX_SEGURA = 8.0;

  OneWire oneWire;
  DallasTemperature sensor(&oneWire);

  void inicializar(int pinoDados) {
    oneWire.begin(pinoDados);
    sensor.begin();

    // 10 bits: a conversao cai de ~750 ms para ~190 ms.
    // Necessario porque coletamos a cada 1 segundo.
    sensor.setResolution(10);
  }

  float lerCelsius() {
    sensor.requestTemperatures();
    return sensor.getTempCByIndex(0);
  }

  // -127 = sensor desconectado. 85.0 exato = conversao nao completou.
  bool leituraValida(float temperatura) {
    return temperatura > -100 && temperatura != 85.0 && !isnan(temperatura);
  }

  bool foraDaFaixaSegura(float temperatura, int criticidade) {
    float tempMin = TEMP_MIN_SEGURA;
    float tempMax = TEMP_MAX_SEGURA;

    if (criticidade > 50) {
      tempMin = 4.0;
      tempMax = 6.0;
    }

    return temperatura < tempMin || temperatura > tempMax;
  }
}
