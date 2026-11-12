/* app17-5-accrawserial — aceleração RAW (ax,ay,az) a 100 Hz pela Serial, para o coletor_raw.py */

#include <Arduino.h>
#include "FastIMU.h"
#include <Wire.h>

/* ---- Pinos ---- */
#define SDA_PIN 22
#define SCL_PIN 23

#define MPU_TYPE MPU6050 // ou MPU6500 (WHO_AM_I = 0x70)
MPU_TYPE mpu;

calData calib = { 0 };

uint32_t tempoAnterior = 0;
const int AMOSTRA_MS = 10;   // 100 Hz

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);
  if (mpu.init(calib, 0x68) != 0) {
    Serial.println("Erro: MPU nao encontrado");
    while (1);
  }

  Serial.println("Mantenha o sensor parado e nivelado para calibrar...");
  delay(2000);
  // Calibração (no Wokwi, comente esta linha)
  mpu.calibrateAccelGyro(&calib);
  mpu.init(calib, 0x68);
  // Fundo de escala ±8 g
  mpu.setAccelRange(8);

  Serial.println("MPU iniciado");
  Serial.println("Aguardando 5 segundos para iniciar a coleta de medições...");
  delay(5000);

  // Cabeçalho do CSV
  Serial.println("ax,ay,az");
}

void loop() {
  // Coleta a 100 Hz
  if (millis() - tempoAnterior >= AMOSTRA_MS) {
    tempoAnterior += AMOSTRA_MS;
    if (millis() - tempoAnterior >= AMOSTRA_MS) tempoAnterior = millis();

    AccelData accel;
    mpu.update();
    mpu.getAccel(&accel);   // em g

    // ax,ay,az
    Serial.printf("%.3f,%.3f,%.3f\n", accel.accelX, accel.accelY, accel.accelZ);
  }
}
