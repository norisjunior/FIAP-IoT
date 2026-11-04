// toque.hpp — o botao touch (modulo TTP223B) no GPIO 4: toque curto ou toque longo.
//
// O modulo faz a parte capacitiva sozinho e entrega um sinal pronto: HIGH enquanto o dedo
// esta' no botao. Aqui so' se mede por quanto tempo o dedo ficou.
#pragma once
#include <Arduino.h>

namespace toque {

const int      PINO     = 4;
const uint32_t LONGO_MS = 2000;  // segurando por isso, vira toque longo

enum Gesto { NADA, CURTO, LONGO };

static bool     dedoAntes = false;   // o dedo estava no botao na leitura anterior
static uint32_t inicio = 0;          // millis() em que o dedo encostou
static bool     longoJaFoi = false;  // o toque longo ja' disparou neste toque

inline void iniciar() {
  pinMode(PINO, INPUT_PULLDOWN);  // se o fio soltar, le' LOW em vez de "tocar" sozinho
}

// Chamada a cada quadro. O longo dispara com o dedo ainda no botao, ao completar 2 s, para a
// pessoa saber quando soltar; o curto, quando o dedo sai antes disso.
inline Gesto ler(uint32_t agora) {
  bool dedo = digitalRead(PINO) == HIGH;
  Gesto gesto = NADA;
  if (dedo && !dedoAntes) {                                         // encostou
    inicio = agora;
    longoJaFoi = false;
  } else if (dedo && !longoJaFoi && agora - inicio >= LONGO_MS) {   // segurou 2 s
    longoJaFoi = true;
    gesto = LONGO;
  } else if (!dedo && dedoAntes && !longoJaFoi) {                   // soltou antes dos 2 s
    gesto = CURTO;
  }
  dedoAntes = dedo;
  return gesto;
}

}  // namespace toque
