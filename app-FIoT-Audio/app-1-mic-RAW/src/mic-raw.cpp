// Etapa 1 — le o microfone em BLOCO, para nao perder amostra.
// Na etapa 0, uma amostra por vez deixava buracos no sinal.

#include <Arduino.h>
#include "INMP441.hpp"

int16_t som[256];

void setup() {
  Serial.begin(115200);
  INMP441::iniciar();
}

void loop() {
  INMP441::ler(som, 256);   // le as 256 de uma vez, sem perder nenhuma

  // 16 mil amostras por segundo nao cabem na serial: imprime 1 a cada 32
  for (int i = 0; i < 256; i += 32) {
    Serial.printf(">som:%d\n", som[i]);
  }
}
