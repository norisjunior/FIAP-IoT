// ============================================================
//  arcoiris.hpp
//  Funções auxiliares para a fita de LED
// ============================================================

#pragma once                    // evita que este arquivo seja incluído 2x

#include <Adafruit_NeoPixel.h>  // o .hpp precisa conhecer o tipo "Adafruit_NeoPixel"

// ------------------------------------------------------------
//  Muda a fita suavemente da cor 1 para a cor 2.
//
//  A ideia: dividir o caminho entre as duas cores em vários
//  "passos" e, a cada passo, andar um pouquinho em R, G e B.
//
//  fita   -> a fita de LED (passada por referência, com o &)
//  passos -> em quantas partes dividir a transição
//  pausa  -> milissegundos entre cada passo
// ------------------------------------------------------------
inline void transicao(Adafruit_NeoPixel &fita,
                      int r1, int g1, int b1,
                      int r2, int g2, int b2,
                      int passos, int pausa) {

  for (int passo = 0; passo <= passos; passo++) {

    // regra de três: quanto já andamos do caminho entre as 2 cores?
    int r = r1 + (r2 - r1) * passo / passos;
    int g = g1 + (g2 - g1) * passo / passos;
    int b = b1 + (b2 - b1) * passo / passos;

    // pinta todos os LEDs com a cor intermediária deste passo
    for (int i = 0; i < fita.numPixels(); i++) {
      fita.setPixelColor(i, fita.Color(r, g, b));
    }

    fita.show();
    delay(pausa);
  }
}
