#pragma once
#include <Wire.h>
#include <FastIMU.h>
#include <math.h>

// Wokwi: MPU6050. Para a placa MPU6500, altere o modelo abaixo.
#ifndef MPU_TYPE
#define MPU_TYPE MPU6050
#endif

namespace ESP32Sensors {
  namespace Accel {
    MPU_TYPE mpu;
    calData calib = {0};
    static bool disponivel = false;

    void inicializar(uint8_t clPin, uint8_t daPin) {
      Wire.begin(daPin, clPin);
      disponivel = mpu.init(calib, 0x68) == 0;
      if (!disponivel) {
        Serial.println("Erro: MPU não encontrado. Verifique modelo e conexões.");
        return;
      }
      mpu.setAccelRange(16);
    }

    AccelData medirAccel() {
      AccelData accel = {};
      if (!disponivel) {
        accel.accelX = accel.accelY = accel.accelZ = NAN;
        return accel;
      }
      mpu.update();
      mpu.getAccel(&accel);
      // FastIMU fornece g; mantemos o payload em m/s².
      accel.accelX *= 9.80665f;
      accel.accelY *= 9.80665f;
      accel.accelZ *= 9.80665f;
      return accel;
    }

    float medirInclinacao(AccelData accel) {
      float x = accel.accelX;
      float y = accel.accelY;
      float z = accel.accelZ;
      if (!isfinite(x) || !isfinite(y) || !isfinite(z)) return NAN;
      if (sqrtf(x*x + y*y + z*z) < 1.0f) return NAN;
      // Caixa parada, com o eixo Z apontando para cima: 0 graus.
      return atan2f(sqrtf(x*x + y*y), z) * 180.0f / PI;
    }
  }
}
