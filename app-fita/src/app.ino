// ============================================================
//  Fita de LED WS2813B - Cores do Arco-Iris com transição suave
//  ESP32 DevKit V4 (38 pinos) | Pino de dados: GPIO 25
// ============================================================

#include <Adafruit_NeoPixel.h>
#include "arcoiris.hpp"   // nosso arquivo, na mesma pasta src/

#define PINO_LED 25   // pino de dados da fita (com resistor de 220 ohms)
#define NUM_LEDS 30   // quantidade de LEDs da fita

// ----------- OS BOTÕES PARA BRINCAR ESTÃO AQUI -----------
const int BRILHO   = 60;    // 0 a 255  (acima de 100 precisa de fonte externa)
const int DURACAO  = 2000;   // ms que cada transição leva
const int DESCANSO = 1000;   // ms parado na cor pura, antes de trocar

// ----------- CALCULADO SOZINHO A PARTIR DO BRILHO --------
// Com brilho B, a fita só consegue mostrar B níveis diferentes.
// Então não adianta fazer mais passos que isso: o passo extra
// cairia no mesmo nível e o ESP32 desenharia um quadro repetido.
const int PASSOS = (BRILHO > 0) ? BRILHO : 1;   // nunca zero (evita divisão por 0)
const int PAUSA  = DURACAO / PASSOS;            // mantém a duração total

Adafruit_NeoPixel fita(NUM_LEDS, PINO_LED, NEO_GRB + NEO_KHZ800);

// As 7 cores do arco-íris (Vermelho, Verde, Azul  ->  R, G, B)
int cores[7][3] = {
  { 255,   0,   0 },   // 1 - Vermelho
  { 255, 127,   0 },   // 2 - Laranja
  { 255, 255,   0 },   // 3 - Amarelo
  {   0, 255,   0 },   // 4 - Verde
  {   0,   0, 255 },   // 5 - Azul
  {  75,   0, 130 },   // 6 - Anil
  { 148,   0, 211 }    // 7 - Violeta
};

void setup() {
  Serial.begin(115200);
  delay(500);

  fita.begin();
  fita.setBrightness(BRILHO);

  // confere na prática que os passos acompanharam o brilho
  Serial.printf("Brilho: %d  ->  Passos: %d  |  Pausa: %d ms\n",
                BRILHO, PASSOS, PAUSA);
}

void loop() {

  // percorre as 7 cores, fazendo a transição de cada uma para a próxima
  for (int c = 0; c < 7; c++) {

    int prox = (c + 1) % 7;   // depois do violeta (6), volta ao vermelho (0)

    transicao(fita,
              cores[c][0],    cores[c][1],    cores[c][2],      // cor atual
              cores[prox][0], cores[prox][1], cores[prox][2],   // próxima cor
              PASSOS, PAUSA);

    delay(DESCANSO);   // segura um pouco na cor pura
  }
}
