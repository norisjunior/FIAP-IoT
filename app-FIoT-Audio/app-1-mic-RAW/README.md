# app-1-mic-RAW — a onda da voz

Etapa 1, e o começo da trilha. Lê o INMP441 e desenha a onda no Teleplot. Nada mais.

É o momento lúdico da aula: o aluno fala e vê o próprio som reagir na tela.

## Pinos

| INMP441 | ESP32 DevKit v1 |
|---|---|
| VDD | 3V3 (**nunca** 5V) |
| GND | GND |
| L/R | GND |
| SCK | GPIO 26 |
| WS | GPIO 25 |
| SD | GPIO 33 |

## O código

```cpp
#include <Arduino.h>
#include "INMP441.hpp"

int16_t som[256];

void setup() {
  Serial.begin(921600);
  INMP441::iniciar();
}

void loop() {
  INMP441::ler(som, 256);

  // 16 mil amostras por segundo nao cabem na serial: imprime 1 a cada 8
  for (int i = 0; i < 256; i += 8) {
    Serial.printf(">som:%d\n", som[i]);
  }
}
```

`INMP441.hpp` **não se digita** — é uma biblioteca, como a do DHT22. Ele faz pelo microfone o
que o `analogRead()` já faz pelos sensores analógicos: esconde o barramento.

## Uso

```bash
pio run -t upload
```

Abra o **Teleplot** e configure **921600** (não 115200 — é fácil esquecer e achar que travou).

Experimente, nesta ordem: **silêncio · falar · assobiar · bater palma**.

## Analógico × digital

Os `.txt` desta pasta são as versões com KY-038 e MAX9814, do 1º ano:

| | `analogRead(34)` | `INMP441::ler(som, 256)` |
|---|---|---|
| Quem digitaliza | o ADC do ESP32 | o próprio microfone |
| Resolução | 12 bits | 24 bits |
| Taxa | ~500 Hz na prática | 16 000 Hz |
| Ruído no fio | pega | não pega, é digital |

**Não existe `analogRead` para o INMP441.** O ESP32 não mede tensão nenhuma — recebe números
prontos.

## Outra placa

Para o **ESP32-S3 Super Mini**: renomeie `platformio.ini.esp32s3` para `platformio.ini` (guarde
o original como `platformio.ini.esp32dev`). O código não muda; a flag `PLACA_S3` troca os pinos
para SCK 4 · WS 5 · SD 6.

## Problemas

| Sintoma | Causa provável |
|---|---|
| Teleplot mudo ou com lixo | está em 115200 — troque para 921600 |
| só zeros ou linha reta | canal errado: troque `ONLY_LEFT` por `ONLY_RIGHT` no `.hpp` |
| a onda achata no topo | `GANHO` baixo demais no `.hpp` — aumente |
| a onda quase não sai do zero | `GANHO` alto demais — diminua |
| nada acontece | L/R não está no GND, ou SD/WS/SCK trocados |

Sem Wokwi: não existe peça INMP441 no simulador.

---

> **Se alguém perguntar por que o `INMP441.hpp` existe:** o
> [app-0-mic-SOM](../app-0-mic-SOM/) (material de apoio, fora do roteiro) lê o microfone sem
> ele, e o gráfico sai picotado.
