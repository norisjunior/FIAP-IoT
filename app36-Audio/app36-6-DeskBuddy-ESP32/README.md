# Desk buddy: olhos que acordam com a wake word

Um companheiro de mesa com dois olhos na tela redonda. Ele dorme de olhos fechados,
com um "Zz" subindo. Quando ouve **"Ei Fioti"**, abre os olhos, olha em volta e pisca.
Depois de 6 s sem ouvir a wake word de novo, fecha os olhos devagar e volta a dormir.

É o [app36-4-WakeWord-ESP32](../app36-4-WakeWord-ESP32/) com uma tela. A wake word não muda:
mesmo exemplo do Edge Impulse, mesmos `// AJUSTE`, mesmo modelo `WakeWord-EiFIOT`.

## Hardware

| INMP441 | ESP32-S3 Super Mini | | Tela GC9A01 | ESP32-S3 Super Mini |
|---|---|---|---|---|
| VDD | 3V3 (nunca 5V) | | VCC | 3V3 |
| GND | GND | | GND | GND |
| L/R | GND | | SCL | GPIO 9 |
| SCK | GPIO 5 | | SDA | GPIO 10 |
| WS | GPIO 6 | | DC | GPIO 11 |
| SD | GPIO 7 | | CS | GPIO 12 |
| | | | RST | GPIO 13 |

A tela é uma TFT redonda de 1,28" e 240×240 pixels, com o controlador GC9A01. Ela fala
**SPI**: os pinos se chamam SCL e SDA na plaquinha, mas são o clock e o dado do SPI, não
I2C. Os GPIOs 9 a 13 estão na ordem dos pinos da plaquinha.

O botão touch e o LED RGB da placa entram no [app36-7](../app36-7-DeskBuddy-Toque-ESP32/).

## O que mudou em relação ao app36-4

| Onde | O quê |
|---|---|
| `platformio.ini` | a biblioteca `TFT_eSPI` e a configuração da tela, nas linhas `OLHOS` |
| `src/olhos.hpp` | **novo**: o desenho e a animação dos olhos |
| `src/deskbuddy.ino` | o `wakeword.ino` do app36-4 com três trechos `// OLHOS` |

Os três trechos do sketch:

1. `#include "olhos.hpp"`;
2. no `setup()`, `olhos::iniciar()` e sem o `while (!Serial)`: o desk buddy roda num
   carregador de celular, sem o PC, e esperar o Monitor Serial o deixaria parado para sempre;
3. no `loop()`, `olhos::acordar()` quando a classe `WakeWord` passa do `LIMIAR`.

## Por que a tela roda em outro núcleo

O ESP32-S3 tem dois núcleos. A wake word ocupa o `loop()`, no núcleo 1, e tem prazo: a cada
375 ms chega uma fatia nova de áudio, e o modelo leva ~52 ms com ela. Enviar um quadro para a
tela leva ~25 ms. Se os dois dividissem o mesmo laço, o áudio atrasaria e apareceria
`Error sample buffer overrun`.

Por isso os olhos rodam numa tarefa própria, no núcleo 0. Os dois lados conversam por uma
única variável: o `loop()` anota a hora em que ouviu a wake word, e a tarefa da tela
decide se está acordada comparando essa hora com o relógio.

## Usar

```bash
pio run -t upload
pio device monitor
```

A primeira compilação baixa a `TFT_eSPI` e compila a biblioteca do Edge Impulse: leva uns
3 minutos. A lib descompactada já está em `lib/`; para usar a sua wake word, troque-a como no
passo 2 do README do app36-4.

## Para mexer

No topo do `olhos.hpp`: a cor, o tamanho e o arredondamento dos olhos, a distância entre
eles e o `ACORDADO_MS`, o tempo que ele fica acordado.

## Se não funcionar

| Sintoma | Causa provável |
|---|---|
| tela toda preta ou branca | fio da tela trocado: confira SCL 9, SDA 10, DC 11, CS 12, RST 13 |
| reinicia sozinho logo ao ligar | faltou o `-DUSE_HSPI_PORT=1` no `platformio.ini` |
| olhos amarelos em vez de ciano | a tela usa outra ordem de cores: acrescente `-DTFT_RGB_ORDER=TFT_BGR` |
| olhos certos, mas nunca acordam | é a wake word: teste o app36-4 e veja o ganho (`>> 16`) |
| `ERR: sem memoria para o quadro da tela` | outra parte do programa usou a RAM antes da tela |
