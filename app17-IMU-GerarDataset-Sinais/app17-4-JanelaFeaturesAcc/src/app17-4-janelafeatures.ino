/* app17-4 — as 8 features de cada janela de 100 leituras, impressas na Serial */

#include "FastIMU.h"
#include <Wire.h>
#include <math.h>

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
float mag[TAMANHO_JANELA]; // magnitude

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

// ===== Features =====

// Média
float calcMean(float arr[], int n) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += arr[i];
  return soma / n;
}

// Desvio-padrão
float calcStd(float arr[], int n, float media) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += (arr[i] - media) * (arr[i] - media);
  return sqrt(soma / n);
}

// Magnitude
void calcMagnitude(float axArr[], float ayArr[], float azArr[], float magArr[], int n) {
  for (int i = 0; i < n; i++) {
    magArr[i] = sqrt(axArr[i]*axArr[i] + ayArr[i]*ayArr[i] + azArr[i]*azArr[i]);
  }
}

// Pico a pico
float calcPtP(float arr[], int n) {
  float mn = arr[0], mx = arr[0];
  for (int i = 1; i < n; i++) {
    if (arr[i] < mn) mn = arr[i];
    if (arr[i] > mx) mx = arr[i];
  }
  return mx - mn;
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
      // Média e desvio de cada eixo
      float mx = calcMean(ax, TAMANHO_JANELA);
      float my = calcMean(ay, TAMANHO_JANELA);
      float mz = calcMean(az, TAMANHO_JANELA);
      float sx = calcStd(ax, TAMANHO_JANELA, mx);
      float sy = calcStd(ay, TAMANHO_JANELA, my);
      float sz = calcStd(az, TAMANHO_JANELA, mz);

      // Magnitude: desvio e pico a pico
      calcMagnitude(ax, ay, az, mag, TAMANHO_JANELA);
      float mMag   = calcMean(mag, TAMANHO_JANELA);
      float stdMag = calcStd(mag, TAMANHO_JANELA, mMag);
      float p2pMag = calcPtP(mag, TAMANHO_JANELA);

      // As 8 features
      Serial.println("--- Features da janela (em g) ---");
      Serial.printf("mean_ax=%.3f  mean_ay=%.3f  mean_az=%.3f\n", mx, my, mz);
      Serial.printf("std_ax=%.3f   std_ay=%.3f   std_az=%.3f\n", sx, sy, sz);
      Serial.printf("std_mag=%.3f  p2p_mag=%.3f\n", stdMag, p2pMag);
      indice = 0;
    }
  }
}
