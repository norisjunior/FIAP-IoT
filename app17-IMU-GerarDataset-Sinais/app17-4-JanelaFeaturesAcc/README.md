# app17-4 - Features da janela

Mesma janela do app17-3 (100 amostras a 100 Hz), mas em vez de imprimir o sinal bruto
calcula e imprime as **features** da janela: as mesmas 8 que o app17-7 publica.

## Wiring

| MPU | ESP32 |
| --- | --- |
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO22 |
| SCL | GPIO23 |

## Features

Cada feature responde a uma pergunta sobre a janela. Valores em `g`.

| Feature | Funcao | Pergunta |
| --- | --- | --- |
| `mean_ax`, `mean_ay`, `mean_az` | `calcMean` | para onde a gravidade aponta? (inclinacao) |
| `std_ax`, `std_ay`, `std_az` | `calcStd` | quanto cada eixo treme? |
| `std_mag` | `calcMagnitude` + `calcStd` | quanto treme no total, em qualquer posicao? |
| `p2p_mag` | `calcMagnitude` + `calcPtP` | amplitude total (pico a pico): qual foi o maior tranco da janela? |

`calcStd` divide por `n` (e nao por `n-1`).

## Aula

- **LED da placa (GPIO2):** aceso = calibrando, mantenha o sensor parado e **nivelado**.
  Apagou = 5 segundos para colocar o sensor na posicao. Reiniciar com o sensor inclinado
  faz a calibracao "apagar" a inclinacao.
- 100 amostras viram 8 numeros: e isso que vai para o MQTT/banco e para o ML (app17-7).
- Parado: `std_*` e `std_mag` perto de zero. Chacoalhando: `std_*`, `std_mag` e `p2p_mag` sobem.
- Inclinando devagar: o `std` quase nao muda, mas a gravidade "migra" de `mean_az`
  para `mean_ax` ou `mean_ay`.
- As mesmas contas feitas a mao, com 5 leituras reais de cada situacao (coleta do
  app17-3): [`colab/contas_da_janela.ipynb`](colab/contas_da_janela.ipynb). A saida
  original do Serial de cada coleta esta em [`coleta_professor/`](coleta_professor/).
- A `coleta_professor/` e de 06/11/2026, antes da correcao do fundo de escala: o
  `setAccelRange(8)` vinha antes do segundo `init()` e a coleta rodou em +-16 g. Em `g` os
  valores sao praticamente os mesmos; o maior pico (4,7 g, no chacoalho) cabe em +-8 g. Nao precisa
  recoletar.
