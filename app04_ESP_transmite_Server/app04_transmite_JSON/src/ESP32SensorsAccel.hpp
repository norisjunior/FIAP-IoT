#pragma once
#include <Wire.h>
#include <FastIMU.h>

// A biblioteca é a mesma para o MPU6050 e para o MPU6500.
#ifndef MPU_TYPE
#define MPU_TYPE MPU6050
#endif

namespace ESP32Sensors {
  namespace Accel {

    struct ACCEL {
      float x;
      float y;
      float z;
    };

    MPU_TYPE mpu;
    calData calib = {0};  // Sem calibração

    void inicializar(uint8_t sda, uint8_t scl) {
      Wire.begin(sda, scl);
      if (mpu.init(calib, 0x68) != 0) {
        Serial.println("Erro: MPU não encontrado. Verifique modelo e conexões.");
        while (1) delay(10);
      }

      // Preserva a configuração do app04: ±16 g, filtro ~21 Hz e taxa de 1 kHz.
      mpu.setAccelRange(16);
      mpu.setGyroRange(500);
      mpu.setGyroLPF(21);  // No MPU6050, o filtro também atua no acelerômetro
      mpu.setAccelLPF(21); // No MPU6500, o acelerômetro tem filtro próprio
      mpu.setAccelODR(1000);
    }

    ACCEL medirAccel() {
      AccelData leitura;
      mpu.update();
      mpu.getAccel(&leitura);  // A API da FastIMU exige o endereço da variável

      // FastIMU entrega em g. O JSON continua em m/s², como antes.
      ACCEL dados;
      dados.x = leitura.accelX * 9.80665f;
      dados.y = leitura.accelY * 9.80665f;
      dados.z = leitura.accelZ * 9.80665f;
      return dados;
    }
  }
}
