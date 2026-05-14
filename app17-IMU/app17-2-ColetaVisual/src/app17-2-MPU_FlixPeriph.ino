#include <FlixPeriph.h>
// Descomente se estiver usando MPU6500 vendido como MPU6050 generico.
#include <MPU6500.h>
#include <Wire.h>

#define SDAPIN 22
#define SCLPIN 23

// Wiring:
// VCC -> 3V3
// GND -> GND
// SDA -> GPIO22
// SCL -> GPIO23

// Troque para MPU6500 se estiver usando o sensor generico.
MPU6500 mpu(Wire);
//MPU6050 mpu(Wire);

void setup() {
  Serial.begin(115200);

  Wire.begin(SDAPIN, SCLPIN);

  if (!mpu.begin()) {
    Serial.println("Erro: MPU nao encontrado");
    while (1) {
      delay(10);
    }
  }

  // Sensibilidade do acelerometro:
  // ACCEL_RANGE_2G  = mais sensivel
  // ACCEL_RANGE_4G
  // ACCEL_RANGE_8G
  // ACCEL_RANGE_16G = mede aceleracoes maiores
  mpu.setAccelRange(IMUInterface::ACCEL_RANGE_16G);

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

    float ax, ay, az;
    mpu.read();
    mpu.getAccel(ax, ay, az);

    //Serial.printf("%.2f,%.2f,%.2f\n", ax, ay, az);
    Serial.printf(">acc_x: %.2f\n>acc_y: %.2f\n>acc_z: %.2f\n", ax, ay, az);
  };
}

