// led.hpp — o LED RGB da placa (um WS2812 no GPIO 48) mostra o estado, como a luz da Alexa:
//   amarelo  = esperando a wake word
//   verde    = acordado, esperando o proximo comando
//   vermelho = modo mudo: a wake word e' ignorada
#pragma once
#include <Arduino.h>

namespace led {

const int PINO = 48;

enum Estado { ESPERANDO, ACORDADO, MUDO };

static int aceso = -1;  // o estado que esta' no LED agora; -1 = nenhum ainda

// O WS2812 guarda a cor sozinho: so' escreve quando o estado muda.
// Brilho baixo de proposito (40 de 255): no maximo o LED ofusca e gasta mais bateria.
inline void mostrar(Estado estado) {
  if (estado == aceso) return;
  aceso = estado;
  switch (estado) {
    case ESPERANDO: neopixelWrite(PINO, 40, 25, 0); break;  // amarelo
    case ACORDADO:  neopixelWrite(PINO, 0, 40, 0);  break;  // verde
    case MUDO:      neopixelWrite(PINO, 40, 0, 0);  break;  // vermelho
  }
}

}  // namespace led
