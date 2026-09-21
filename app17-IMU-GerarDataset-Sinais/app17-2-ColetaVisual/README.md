# app17-2 - Leitura simples do MPU

App base para montar o MPU no ESP32 e ler os tres eixos do acelerometro usando
a biblioteca FlixPeriph.

## Wiring

| MPU6050/GY-521 | ESP32 |
| --- | --- |
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO26 |
| SCL | GPIO27 |

## Aula

- O codigo configura apenas a sensibilidade do acelerometro.
- A saida serial mostra `ax,ay,az`.
- A FlixPeriph retorna aceleracao em `m/s2`. Com o sensor parado, um eixo deve
  ficar proximo de `9.81 m/s2`, por causa da gravidade.
