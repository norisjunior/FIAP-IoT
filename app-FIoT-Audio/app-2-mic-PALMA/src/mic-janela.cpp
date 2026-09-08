// Etapa 2 — uma janela de 32 ms vira um numero: o RMS.

#include <Arduino.h>
#include "INMP441.hpp"

#define N 512   // 512 amostras / 16000 Hz = 32 ms

int16_t som[N];

void setup() {
  Serial.begin(115200);
  INMP441::iniciar();
}

void loop() {
  INMP441::ler(som, N);

  // o microfone tem um deslocamento fixo: tira a media primeiro
  float media = 0;
  for (int i = 0; i < N; i++) {
    media += som[i];
  }
  media = media / N;

  // RMS: eleva ao quadrado, tira a media, tira a raiz
  float soma = 0;
  for (int i = 0; i < N; i++) {
    float v = som[i] - media;
    soma += v * v;
  }
  int rms = sqrt(soma / N);

  Serial.printf(">rms:%d\n", rms);
}
