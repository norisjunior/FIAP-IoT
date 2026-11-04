# app36-Analogico-Wokwi — a trilha com microfone analógico

A primeira versão da trilha de áudio, com microfone **analógico** lido por `analogRead()`.
Antes se chamavam `app28-1`, `app28-2` e `app28-3`. A trilha de hoje usa o INMP441
([app36-1](../app36-1-mic-RAW/) e [app36-2](../app36-2-mic-VOZ_FEATURES/)), e esses mesmos
sketches estão guardados lá dentro como `.txt *-ANALOGICO`.

| Pasta | Sketch ativo (`.ino`) | Variantes (`.txt`) |
|---|---|---|
| [app36-A1-mic-RAW](app36-A1-mic-RAW/) | `comparador-mic`: KY038 (GPIO 34) e MAX9814 (GPIO 35) lado a lado | onda crua de cada um |
| [app36-A2-mic-PALMA](app36-A2-mic-PALMA/) | `mic-voz-max9814`: calibra o centro e mostra a voz | `mic-palma-ky038`: palma acende LED (GPIO 27) |
| [app36-A3-mic-VOZ_FEATURES](app36-A3-mic-VOZ_FEATURES/) | `FEATURES-RMS`: janela e RMS | `FEATURES-AMPLITUDE` |

> **A revisar antes de usar.** O `README.md`, o `diagram.json` e o `wokwi.toml` de cada pasta
> foram herdados do app24 (irrigação): o diagrama tem DHT22, LED e um potenciômetro no
> GPIO 34, e nenhum microfone. Os sketches que leem o GPIO 35 (MAX9814) não recebem sinal no
> simulador.
