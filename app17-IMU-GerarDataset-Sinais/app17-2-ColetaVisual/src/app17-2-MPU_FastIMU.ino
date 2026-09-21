#include "FastIMU.h"
#include <Wire.h>

#define SDAPIN 22
#define SCLPIN 23

// Wiring:
// VCC -> 3V3
// GND -> GND
// SDA -> GPIO22
// SCL -> GPIO23

#define MPU_TYPE MPU6500 // troque para MPU6050 se estiver usando o sensor original
MPU_TYPE mpu;

calData calib = { 0 }; // zerada = sem calibrar (ok para fins didaticos)

void setup() {
  Serial.begin(115200);

  Wire.begin(SDAPIN, SCLPIN);

  if (mpu.init(calib, 0x68) != 0) {
    Serial.println("Erro: MPU nao encontrado");
    while (1) {
      delay(10);
    }
  }

  // Sensibilidade do acelerometro, em g:
  // 2  = mais sensivel
  // 4
  // 8
  // 16 = mede aceleracoes maiores
  mpu.setAccelRange(16);

  Serial.println("MPU iniciado");
  Serial.println("ax,ay,az");
}

// Taxa de amostragem: 1000 / AMOSTRA_MS = Hz
// | Aplicacao           | Sinal (Hz) | Taxa minima | AMOSTRA_MS |
// |---------------------|------------|-------------|------------|
// | Orientacao / tilt   | < 5 Hz     | 10 Hz       | 100 ms     |
// | Deteccao de passo   | < 10 Hz    | 20 Hz       | 50 ms      |
// | Gesture recognition | < 25 Hz    | 50 Hz       | 20 ms      |
// | Vibracao / impacto  | < 500 Hz   | 1 kHz       | 1 ms       |
const int AMOSTRA_MS = 20;

uint64_t tempoAnterior = 0;

void loop() {
  if (millis() - tempoAnterior >= AMOSTRA_MS){
    tempoAnterior = millis();

    AccelData accel;
    mpu.update();
    mpu.getAccel(&accel);

    //Serial.printf("%.2f,%.2f,%.2f\n", accel.accelX, accel.accelY, accel.accelZ);
    Serial.printf(">acc_x: %.2f\n>acc_y: %.2f\n>acc_z: %.2f\n", accel.accelX, accel.accelY, accel.accelZ);
  };
}

