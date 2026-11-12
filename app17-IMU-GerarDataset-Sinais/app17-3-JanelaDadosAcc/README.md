# app17-3 - Janela de dados do acelerometro

Coleta 100 amostras de `ax,ay,az` a 100 Hz e imprime a janela inteira na Serial.

## Wiring

| MPU | ESP32 |
| --- | --- |
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO22 |
| SCL | GPIO23 |

## Aula

- **LED da placa (GPIO2):** aceso = calibrando, mantenha o sensor parado e **nivelado**.
  Apagou = 5 segundos para colocar o sensor na posicao da coleta. Reiniciar com o sensor
  inclinado faz a calibracao "apagar" a inclinacao.
- `TAMANHO_JANELA = 100` amostras, `AMOSTRA_MS = 10` (100 Hz) via `millis()`, sem `delay()`.
- Cheia a janela, imprime as 100 linhas e reinicia o indice.
- Janela = bloco de sinal sobre o qual, no app17-4, se calculam as features.
- E com este app que o professor coleta, ao vivo, as janelas das 4 situacoes (parado,
  inclinado para frente, inclinado para tras, chacoalhando) usadas no
  `app17-4-JanelaFeaturesAcc/colab/contas_da_janela.ipynb`.
