// olhos.hpp — os olhos do desk buddy na tela redonda GC9A01 (240x240).
//
// Rodam numa tarefa propria, no nucleo 0. A wake word continua no loop(), no nucleo 1,
// e nunca espera a tela: se esperasse, o audio de 375 ms nao seria lido a tempo.
// O loop() so' chama olhos::acordar() quando ouve a wake word.
//
// TOQUE / LED: a mesma tarefa le' o botao touch e acende o LED da placa. Ela volta a cada
// quadro; o loop() so' volta a cada 375 ms e perderia um toque rapido.
#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "toque.hpp"  // TOQUE
#include "led.hpp"    // LED

namespace olhos {

// Aparencia e tempo: mexa aqui
const uint16_t COR         = TFT_CYAN;
const uint16_t COR_MUDO    = TFT_RED;  // TOQUE: a cor dos olhos no modo mudo
const uint16_t COR_FELIZ   = TFT_MAGENTA;  // TOQUE: a cor do "^ ^" depois de um toque
const int      LARGURA     = 62;    // de cada olho, em pixels
const int      ALTURA      = 84;
const int      RAIO        = 18;    // arredondamento dos cantos
const int      DISTANCIA   = 88;    // entre os centros dos dois olhos
const uint32_t ACORDADO_MS = 6000;  // quanto tempo fica acordado depois da wake word
const uint32_t FELIZ_MS    = 1500;  // TOQUE: quanto tempo fica feliz depois de um toque

static TFT_eSPI tft;
static TFT_eSprite quadro(&tft);         // desenha fora da tela e envia o quadro inteiro de uma vez
static volatile uint32_t chamadoEm = 0;  // millis() da ultima wake word; 0 = nunca ouviu
static volatile bool mudo = false;       // TOQUE: o toque longo liga e desliga; o loop() ignora a wake word
static uint32_t felizAte = 0;            // TOQUE: millis() ate' quando os olhos ficam felizes

// Estado da animacao: so' a tarefa da tela mexe nele
static float abertura = 0;               // 0 = fechado, 1 = aberto
static float olharX = 0, olharY = 0;     // para onde os olhos estao virados, em pixels
static float alvoX = 0, alvoY = 0;
static uint32_t proximoOlhar = 0, proximaPiscada = 0, fimPiscada = 0;

// Chamada pelo loop() quando a wake word passa do limiar
inline void acordar() {
  chamadoEm = millis();
}

// LED: o LED tambem precisa saber se esta' acordado, por isso virou funcao.
// Le' o chamadoEm antes do relogio: o loop() pode muda-lo a qualquer momento, do outro nucleo.
inline bool acordado() {
  uint32_t ultimo = chamadoEm;
  return ultimo != 0 && millis() - ultimo < ACORDADO_MS;
}

// Anda uma fracao do caminho a cada quadro: rapido no comeco, suave no fim
inline float aproxima(float atual, float alvo, float passo) {
  return atual + (alvo - atual) * passo;
}

inline void desenhaOlho(int cx, int cy, float aberto, uint16_t cor) {  // TOQUE: cor
  int h = ALTURA * aberto;
  if (h < 8) quadro.fillRoundRect(cx - LARGURA / 2, cy - 3, LARGURA, 6, 3, cor);  // fechado: um traco
  else       quadro.fillRoundRect(cx - LARGURA / 2, cy - h / 2, LARGURA, h, min(RAIO, h / 2), cor);
}

// TOQUE: olho feliz, um "^": duas linhas grossas, de pontas redondas, que se encontram no alto.
// Sem a cor de fundo, a drawWideLine le' o pixel de baixo para suavizar a borda: assim nao
// fica uma emenda escura onde as duas linhas se cruzam.
inline void desenhaOlhoFeliz(int cx, int cy) {
  const int meio = LARGURA / 2 - 6;  // da ponta ao centro, na horizontal
  const int alto = 16;               // da ponta ao bico, na vertical (metade)
  const int grossura = 14;
  quadro.drawWideLine(cx - meio, cy + alto, cx, cy - alto, grossura, COR_FELIZ);
  quadro.drawWideLine(cx, cy - alto, cx + meio, cy + alto, grossura, COR_FELIZ);
}

inline void desenhaZzz(uint32_t agora) {
  float p = (agora % 3000) / 3000.0;  // sobe e recomeca a cada 3 s
  quadro.setTextColor(COR);
  quadro.setTextSize(2);
  quadro.drawString("z", 150 + p * 14, 78 - p * 30);
  quadro.setTextSize(3);
  quadro.drawString("Z", 166 + p * 14, 56 - p * 30);
}

inline void desenhaQuadro(uint32_t agora) {
  bool estaAcordado = acordado();

  if (mudo) {                          // TOQUE: mudo: fecha e fica parado, sem respirar
    abertura = aproxima(abertura, 0, 0.3);
    olharX = aproxima(olharX, 0, 0.3);
    olharY = aproxima(olharY, 0, 0.3);
  } else if (estaAcordado) {
    if (agora > proximoOlhar) {        // olha para outro lugar de vez em quando
      alvoX = random(-24, 25);
      alvoY = random(-16, 17);
      proximoOlhar = agora + random(700, 2500);
    }
    if (agora > proximaPiscada) {      // pisca a cada 2 a 5 s
      fimPiscada = agora + 120;
      proximaPiscada = agora + random(2000, 5000);
    }
    abertura = aproxima(abertura, agora < fimPiscada ? 0 : 1, 0.45);
    olharX = aproxima(olharX, alvoX, 0.35);
    olharY = aproxima(olharY, alvoY, 0.35);
  } else {                             // dormindo: fecha devagar, olha para baixo e respira
    abertura = aproxima(abertura, 0, 0.12);
    olharX = aproxima(olharX, 0, 0.1);
    olharY = aproxima(olharY, 12 + 3 * sin(agora / 600.0), 0.1);
  }

  int esquerdo = 120 - DISTANCIA / 2 + olharX;
  int direito  = 120 + DISTANCIA / 2 + olharX;
  int altura   = 120 + olharY;

  quadro.fillSprite(TFT_BLACK);
  if (agora < felizAte) {              // TOQUE
    desenhaOlhoFeliz(esquerdo, altura);
    desenhaOlhoFeliz(direito, altura);
  } else {
    uint16_t cor = mudo ? COR_MUDO : COR;  // TOQUE
    desenhaOlho(esquerdo, altura, abertura, cor);
    desenhaOlho(direito, altura, abertura, cor);
  }
  if (!mudo && !estaAcordado && abertura < 0.1) desenhaZzz(agora);
  quadro.pushSprite(0, 0);
}

// TOQUE: o curto acorda (e, se ja' esta' acordado, deixa feliz); o longo liga e desliga o mudo
inline void trataToque(uint32_t agora) {
  toque::Gesto gesto = toque::ler(agora);
  if (gesto == toque::LONGO) {
    mudo = !mudo;
    chamadoEm = 0;                     // entrando ou saindo do mudo, comeca dormindo
  } else if (gesto == toque::CURTO && !mudo) {
    if (acordado()) felizAte = agora + FELIZ_MS;
    acordar();
  }
}

inline void tarefa(void *) {
  while (true) {
    uint32_t agora = millis();
    trataToque(agora);                                                             // TOQUE
    led::mostrar(mudo ? led::MUDO : acordado() ? led::ACORDADO : led::ESPERANDO);  // LED
    desenhaQuadro(agora);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

inline void iniciar() {
  toque::iniciar();  // TOQUE
  tft.init();
  tft.fillScreen(TFT_BLACK);
  quadro.setColorDepth(8);  // 240x240 em 8 bits = 57 KB; em 16 bits seriam 115 KB
  if (quadro.createSprite(240, 240) == nullptr) {
    Serial.println("ERR: sem memoria para o quadro da tela");
    return;
  }
  xTaskCreatePinnedToCore(tarefa, "olhos", 4096, NULL, 1, NULL, 0);  // nucleo 0
}

}  // namespace olhos
