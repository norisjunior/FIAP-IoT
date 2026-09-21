#include "FastIMU.h"
#include <Wire.h>

#define SDAPIN 22
#define SCLPIN 23

// Wiring:
// VCC -> 3V3
// GND -> GND
// SDA -> GPIO22
// SCL -> GPIO23

#define MPU_TYPE MPU6050 // troque para MPU6500 se estiver usando o sensor generico
MPU_TYPE mpu;

calData calib = { 0 }; // preenchida pela calibracao no setup()

// Arrays para armazenar a janela de 100 leituras
const int TAMANHO_JANELA = 100;
float ax[TAMANHO_JANELA];
float ay[TAMANHO_JANELA];
float az[TAMANHO_JANELA];

void setup() {
  Serial.begin(115200);

  Wire.begin(SDAPIN, SCLPIN);

  if (mpu.init(calib, 0x68) != 0) {
    Serial.println("Erro: MPU nao encontrado");
    while (1);
  }

  // Sensibilidade do acelerometro, em g:
  // 2  = mais sensivel
  // 4
  // 8
  // 16 = mede acelerações maiores
  mpu.setAccelRange(8);

  Serial.println("Mantenha o sensor parado e nivelado para calibrar...");
  delay(2000);
  mpu.calibrateAccelGyro(&calib);
  mpu.init(calib, 0x68);

  Serial.println("MPU iniciado");
}

// Taxa de amostragem: 1000 / AMOSTRA_MS = Hz
const int AMOSTRA_MS = 10; // 10ms = 100Hz

int indice = 0;
uint32_t tempoAnterior = 0;

void loop() {
  if (millis() - tempoAnterior >= AMOSTRA_MS) {
    tempoAnterior = millis();

    AccelData accel;
    mpu.update();
    mpu.getAccel(&accel);
    ax[indice] = accel.accelX;
    ay[indice] = accel.accelY;
    az[indice] = accel.accelZ;
    indice++;

    if (indice >= TAMANHO_JANELA) {
      Serial.println("Janela coletada:");
      for (int i = 0; i < TAMANHO_JANELA; i++) {
        Serial.print(ax[i]); Serial.print(", ");
        Serial.print(ay[i]); Serial.print(", ");
        Serial.println(az[i]);
      }
      indice = 0;
    }
  }
}