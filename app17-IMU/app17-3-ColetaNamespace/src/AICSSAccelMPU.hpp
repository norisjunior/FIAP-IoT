#pragma once

#include <FlixPeriph.h>
// Incluir o #include abaixo se o sensor for o "genérico" identificado como 0x70
// #include <MPU6500.h>
#include <Wire.h>

namespace AICSSAccel {

  MPU6050 mpu(Wire);
  // Trocar pela declaração abaixo se o sensor for o "genérico" identificado como 0x70
  //MPU6500 mpu(Wire);

  struct ACCEL {
    float x;
    float y;
    float z;
  };

  void inicializar(int pinSDA, int pinSCL) {
    Wire.begin(pinSDA, pinSCL);

    if (mpu.begin() < 0) {
      Serial.println("Erro: Sensor MPU não encontrado. Verifique as conexões.");
      while (1) delay(10);
    }

    Serial.println("Sensor MPU inicializado com sucesso!");
  }

  ACCEL lerAceleracao() {
    mpu.read();

    ACCEL resultado;
    mpu.getAccel(resultado.x, resultado.y, resultado.z);
    return resultado;
  }

}
