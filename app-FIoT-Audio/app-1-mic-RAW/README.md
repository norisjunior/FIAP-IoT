# app-1-mic-RAW — ler em bloco, sem perder amostra

Etapa 1. Conserta o defeito da [etapa 0](../app-0-mic-SOM/): lê 256 amostras de uma vez, em
vez de uma por chamada. O sinal para de sair picotado e a onda aparece inteira.

## Pinos

Iguais aos da etapa 0 (VDD 3V3 · GND · L/R no GND · SCK 26 · WS 25 · SD 33).

## O que muda em relação à etapa 0

| | Etapa 0 | Etapa 1 |
|---|---|---|
| Leitura | `I2S.read()` — 1 amostra | `INMP441::ler(som, 256)` — 256 de uma vez |
| Enquanto o ESP32 imprime | o microfone continua produzindo e **as amostras se perdem** | o DMA guarda; nada se perde |
| No gráfico | picotado, com buracos | a onda contínua |
| Valor | cru, na casa dos milhões | encolhido para caber em 16 bits |

```cpp
void loop() {
  INMP441::ler(som, 256);   // le as 256 de uma vez, sem perder nenhuma

  for (int i = 0; i < 256; i += 32) {
    Serial.printf(">som:%d\n", som[i]);
  }
}
```

## O `INMP441.hpp`

Não se digita em aula. Ele faz pelo INMP441 o que o `analogRead()` já faz pelos sensores
analógicos: esconde o barramento.

A diferença para o `<I2S.h>` da etapa 0 é que aqui a leitura é **em bloco**, com DMA — que é o
único jeito de acompanhar 16 000 amostras por segundo enquanto o programa faz outra coisa.

> **`analogRead()` também é um monte de código — só já está escondido no framework.** Abra o
> `.hpp` uma vez, na comparação analógico × digital, e feche. O aluno não escreve o driver do
> ADC; também não escreve o do I2S.

## Uso

```bash
pio run -t upload
```

Teleplot a 115200. Experimente: **silêncio · falar · assobiar · bater palma**.

Compare com a etapa 0 rodando o mesmo teste: é o mesmo microfone, o mesmo som, e o gráfico
muda completamente.

## Por que o `for` pula de 32 em 32

16 000 amostras/s × 16 bits = 32 kB/s. A 115200 baud cabem ~11 kB/s de texto. O código lê
**todas** as amostras, mas imprime 1 a cada 32.

Note a diferença para a etapa 0: lá as amostras eram **perdidas**; aqui são **lidas e
descartadas na hora de imprimir**. O sinal que aparece é uma amostragem honesta do sinal
inteiro — não um pedaço com furos.

É o problema da Aula 13 (100 Hz já derruba o Node-RED), agora 160× pior.

## Ajustar o ganho

`GANHO` no `INMP441.hpp` (padrão 11) é quantos bits jogar fora do valor de 24 bits — o mesmo
papel do 2G/4G/8G/16G no acelerômetro (Aula 12).

| Sintoma | O que fazer |
|---|---|
| a onda achata em 32767 | **aumente** o número |
| falando normal a onda quase não sai do zero | **diminua** o número |

## Analógico × I2S

Os `.txt` desta pasta são as versões com KY-038 e MAX9814:

| | KY-038 / MAX9814 | INMP441 |
|---|---|---|
| Leitura | `analogRead(pino)` | `i2s_read()` — um bloco |
| Resolução | 12 bits do ADC do ESP32 | 24 bits, dentro do microfone |
| Taxa | ~500 Hz na prática | 16 000 Hz |
| Ruído | do ADC e da alimentação | digital, não pega ruído no fio |

## Problemas

| Sintoma | Causa provável |
|---|---|
| só zeros ou lixo constante | canal errado — troque `ONLY_LEFT` por `ONLY_RIGHT` no `.hpp` |
| nada acontece | L/R não está no GND, ou SD/WS/SCK trocados |
| `ESP_I2S.h not found` | é a API do core 3.x; este projeto usa a legada |

Sem Wokwi: não existe peça INMP441 no simulador.
