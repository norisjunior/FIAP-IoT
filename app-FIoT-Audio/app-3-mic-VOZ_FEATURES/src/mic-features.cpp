// Etapa 3 — uma janela de 256 ms vira 4 numeros (features).
// Mesmas funcoes do app17 (acelerometro), agora sobre audio.

#include <Arduino.h>
#include "INMP441.hpp"

#define N 4096   // 4096 amostras / 16000 Hz = 256 ms

int16_t som[N];


float calcMean(int16_t arr[], int n) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += arr[i];
  return soma / n;
}

float calcStd(int16_t arr[], int n, float media) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += (arr[i] - media) * (arr[i] - media);
  return sqrt(soma / n);
}

float calcPtP(int16_t arr[], int n) {
  int mn = arr[0], mx = arr[0];
  for (int i = 1; i < n; i++) {
    if (arr[i] < mn) mn = arr[i];
    if (arr[i] > mx) mx = arr[i];
  }
  return mx - mn;
}

// quantas vezes o sinal passa pelo zero por segundo: som agudo passa muito,
// som grave passa pouco
float calcZCR(int16_t arr[], int n, float media) {
  int cruzamentos = 0;
  for (int i = 1; i < n; i++) {
    if ((arr[i] - media < 0) != (arr[i - 1] - media < 0)) cruzamentos++;
  }
  return (float)cruzamentos * INMP441::TAXA / n;
}


void setup() {
  Serial.begin(115200);
  INMP441::iniciar();
}

void loop() {
  INMP441::ler(som, N);

  float media  = calcMean(som, N);
  float desvio = calcStd(som, N, media);
  float p2p    = calcPtP(som, N);
  float zcr    = calcZCR(som, N, media);

  Serial.printf(">media:%d\n",  (int)media);
  Serial.printf(">desvio:%d\n", (int)desvio);
  Serial.printf(">p2p:%d\n",    (int)p2p);
  Serial.printf(">zcr:%d\n",    (int)zcr);
}
