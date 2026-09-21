# app17-4 - Janela de dados do acelerometro

Coleta 100 amostras de `ax,ay,az` a 100 Hz e imprime a janela inteira na Serial.

## Wiring

| MPU | ESP32 |
| --- | --- |
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO22 |
| SCL | GPIO23 |

## Aula

- `TAMANHO_JANELA = 100` amostras, `AMOSTRA_MS = 10` (100 Hz) via `millis()`, sem `delay()`.
- Cheia a janela, imprime as 100 linhas e reinicia o indice.
- Janela = bloco de sinal sobre o qual, no proximo app, se calculam as features.
