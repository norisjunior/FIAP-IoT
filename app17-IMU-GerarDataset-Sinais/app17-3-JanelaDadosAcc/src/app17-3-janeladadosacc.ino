/* app17-3 — janela de 100 leituras do acelerômetro, impressa na Serial */

#include "FastIMU.h"
#include <Wire.h>

// Ligação: VCC->3V3, GND->GND, SDA->GPIO22, SCL->GPIO23
#define SDAPIN 22
#define SCLPIN 23

#define LED_ONBOARD 2 // aceso = calibrando

#define MPU_TYPE MPU6050 // ou MPU6500 (WHO_AM_I = 0x70)
MPU_TYPE mpu;

calData calib = { 0 };

// Janela de 100 leituras
const int TAMANHO_JANELA = 100;
float ax[TAMANHO_JANELA];
float ay[TAMANHO_JANELA];
float az[TAMANHO_JANELA];

void setup() {
  pinMode(LED_ONBOARD, OUTPUT);
  digitalWrite(LED_ONBOARD, HIGH);

  Serial.begin(115200);

  Wire.begin(SDAPIN, SCLPIN);

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
  digitalWrite(LED_ONBOARD, LOW); // apagou = posicione o sensor

  Serial.println("MPU iniciado");
  Serial.println("Aguardando 5 segundos para iniciar a coleta de medições...");
  delay(5000);
}

const int AMOSTRA_MS = 10; // 100 Hz

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
        Serial.printf("%.3f, %.3f, %.3f\n", ax[i], ay[i], az[i]);
      }
      indice = 0;
    }
  }
}
