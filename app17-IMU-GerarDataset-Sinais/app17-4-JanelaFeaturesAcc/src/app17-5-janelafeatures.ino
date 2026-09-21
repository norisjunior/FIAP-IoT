#include "FastIMU.h"
#include <Wire.h>
#include <math.h>

#define SDAPIN 22
#define SCLPIN 23

// Wiring:
// VCC -> 3V3
// GND -> GND
// SDA -> GPIO22
// SCL -> GPIO23

#define MPU_TYPE MPU6500 // troque para MPU6050 se estiver usando o sensor original
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

float calcMean(float arr[], int n) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += arr[i];
  return soma / n;
}

float calcStd(float arr[], int n) {
  float media = calcMean(arr, n);
  float soma = 0;
  for (int i = 0; i < n; i++) soma += (arr[i] - media) * (arr[i] - media);
  return sqrt(soma / n);
}

float calcRMS(float arr[], int n) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += arr[i] * arr[i];
  return sqrt(soma / n);
}

float calcRMSMagnitude(float ax[], float ay[], float az[], int n) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += ax[i]*ax[i] + ay[i]*ay[i] + az[i]*az[i];
  return sqrt(soma / n);
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
      Serial.println("--- Features ---");
      Serial.printf("mean_ax=%.3f  mean_ay=%.3f  mean_az=%.3f\n", calcMean(ax, TAMANHO_JANELA), calcMean(ay, TAMANHO_JANELA), calcMean(az, TAMANHO_JANELA));
      Serial.printf("std_ax=%.3f   std_ay=%.3f   std_az=%.3f\n",  calcStd(ax,  TAMANHO_JANELA), calcStd(ay,  TAMANHO_JANELA), calcStd(az,  TAMANHO_JANELA));
      Serial.printf("rms_ax=%.3f   rms_ay=%.3f   rms_az=%.3f\n",  calcRMS(ax,  TAMANHO_JANELA), calcRMS(ay,  TAMANHO_JANELA), calcRMS(az,  TAMANHO_JANELA));
      Serial.printf("rms_mag=%.3f\n", calcRMSMagnitude(ax, ay, az, TAMANHO_JANELA));
      indice = 0;
    }
  }
}