// led.hpp — o LED RGB da placa (um WS2812 no GPIO 48) mostra o estado, como a luz da Alexa:
//   amarelo  = esperando a wake word
//   verde    = acordado, esperando o proximo comando
//   vermelho = em deep sleep: nao escuta (SONO: aceso no minimo, com o ESP32 dormindo)
#pragma once
#include <Arduino.h>
#include "driver/gpio.h"  // SONO

namespace led {

const int     PINO        = 48;
const uint8_t BRILHO_SONO = 1;  // SONO: o minimo que ainda acende (de 255)

enum Estado { ESPERANDO, ACORDADO };

static int aceso = -1;  // o estado que esta' no LED agora; -1 = nenhum ainda

// SONO: ao acordar do deep sleep, o pino continua travado; solta antes de mudar a cor
inline void iniciar() {
  gpio_hold_dis((gpio_num_t)PINO);
  gpio_deep_sleep_hold_dis();
}

// O WS2812 guarda a cor sozinho: so' escreve quando o estado muda.
// Brilho baixo de proposito (40 de 255): no maximo o LED ofusca e gasta mais bateria.
inline void mostrar(Estado estado) {
  if (estado == aceso) return;
  aceso = estado;
  switch (estado) {
    case ESPERANDO: neopixelWrite(PINO, 40, 25, 0); break;  // amarelo
    case ACORDADO:  neopixelWrite(PINO, 0, 40, 0);  break;  // verde
  }
}

// SONO: o WS2812 continua aceso enquanto tiver energia, mesmo com o ESP32 dormindo. So' o fio
// de dados precisa ficar parado em LOW: solto, um ruido nele poderia trocar a cor.
// Vermelho puro acende so' um dos tres LEDs de dentro dele; cinza ou violeta acenderiam mais.
inline void dormir() {
  neopixelWrite(PINO, BRILHO_SONO, 0, 0);
  delay(1);                        // termina de enviar a cor
  gpio_hold_en((gpio_num_t)PINO);  // trava o pino como esta' agora: em LOW
  gpio_deep_sleep_hold_en();       // e mantem a trava durante o deep sleep
}

}  // namespace led
