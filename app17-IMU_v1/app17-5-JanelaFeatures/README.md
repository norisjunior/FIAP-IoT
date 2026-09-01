# app17-5 - Features da janela

Mesma janela do app17-4 (100 amostras a 100 Hz), mas em vez de imprimir o sinal bruto
calcula e imprime as **features** da janela.

## Wiring

| MPU | ESP32 |
| --- | --- |
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO22 |
| SCL | GPIO23 |

## Features

| Funcao | O que mede |
| --- | --- |
| `calcMean` | media de cada eixo (postura/orientacao) |
| `calcStd` | desvio padrao (quanto o eixo oscila = vibracao) |
| `calcRMS` | energia do eixo |
| `calcRMSMagnitude` | energia dos 3 eixos juntos |

## Aula

- 100 amostras viram 10 numeros: e isso que vai para o MQTT/banco e para o ML.
- Parado: `std` proximo de zero. Chacoalhando: `std` e `rms_mag` sobem.
