#pragma once

#include <Arduino.h>

// Luz dentro da caixa (LDR).
// luz = 4095 - analogRead  ->  quanto maior, mais luz entrou.
//
// Aqui NAO existe limiar de "tampa aberta". O valor que separa caixa fechada de
// caixa aberta muda com o tamanho da caixa, a posicao do sensor e a iluminacao
// da sala. Quem decide isso e o Colab, olhando os dados da propria caixa.
namespace Luz {
  int pinoLdr = 0;

  void inicializar(int pinoSensorLuz) {
    pinoLdr = pinoSensorLuz;
    pinMode(pinoLdr, INPUT);
  }

  int ler() {
    int leitura = analogRead(pinoLdr);
    return 4095 - leitura;
  }
}
