// olhos.hpp — os olhos do desk buddy na tela redonda GC9A01 (240x240).
//
// Rodam numa tarefa propria, no nucleo 0. A wake word continua no loop(), no nucleo 1,
// e nunca espera a tela: se esperasse, o audio de 375 ms nao seria lido a tempo.
// O loop() so' chama olhos::acordar() quando ouve a wake word.
#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>

namespace olhos {

// Aparencia e tempo: mexa aqui
const uint16_t COR         = TFT_CYAN;
const int      LARGURA     = 62;    // de cada olho, em pixels
const int      ALTURA      = 84;
const int      RAIO        = 18;    // arredondamento dos cantos
const int      DISTANCIA   = 88;    // entre os centros dos dois olhos
const uint32_t ACORDADO_MS = 6000;  // quanto tempo fica acordado depois da wake word

static TFT_eSPI tft;
static TFT_eSprite quadro(&tft);         // desenha fora da tela e envia o quadro inteiro de uma vez
static volatile uint32_t chamadoEm = 0;  // millis() da ultima wake word; 0 = nunca ouviu

// Estado da animacao: so' a tarefa da tela mexe nele
static float abertura = 0;               // 0 = fechado, 1 = aberto
static float olharX = 0, olharY = 0;     // para onde os olhos estao virados, em pixels
static float alvoX = 0, alvoY = 0;
static uint32_t proximoOlhar = 0, proximaPiscada = 0, fimPiscada = 0;

// Chamada pelo loop() quando a wake word passa do limiar
inline void acordar() {
  chamadoEm = millis();
}

// Anda uma fracao do caminho a cada quadro: rapido no comeco, suave no fim
inline float aproxima(float atual, float alvo, float passo) {
  return atual + (alvo - atual) * passo;
}

inline void desenhaOlho(int cx, int cy, float aberto) {
  int h = ALTURA * aberto;
  if (h < 8) quadro.fillRoundRect(cx - LARGURA / 2, cy - 3, LARGURA, 6, 3, COR);  // fechado: um traco
  else       quadro.fillRoundRect(cx - LARGURA / 2, cy - h / 2, LARGURA, h, min(RAIO, h / 2), COR);
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
  bool acordado = chamadoEm != 0 && agora - chamadoEm < ACORDADO_MS;

  if (acordado) {
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

  quadro.fillSprite(TFT_BLACK);
  desenhaOlho(120 - DISTANCIA / 2 + olharX, 120 + olharY, abertura);
  desenhaOlho(120 + DISTANCIA / 2 + olharX, 120 + olharY, abertura);
  if (!acordado && abertura < 0.1) desenhaZzz(agora);
  quadro.pushSprite(0, 0);
}

inline void tarefa(void *) {
  while (true) {
    desenhaQuadro(millis());
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

inline void iniciar() {
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
