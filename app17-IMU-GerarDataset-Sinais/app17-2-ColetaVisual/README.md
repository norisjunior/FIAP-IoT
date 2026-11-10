# app17-2 - Leitura simples do MPU

App base para montar o MPU no ESP32 e ler os tres eixos do acelerometro usando
a biblioteca FastIMU.

## Wiring

| MPU6050/GY-521 | ESP32 |
| --- | --- |
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO22 |
| SCL | GPIO23 |

## Aula

- A FastIMU retorna aceleracao em `g`. Com o sensor parado, um eixo deve ficar
  proximo de `1.0 g`, por causa da gravidade.
- A saida serial usa o formato `>acc_x: 0.01`, que o `plot_accel.py` (app17-0)
  desenha em tempo real.
- Leitura a 100 Hz (`AMOSTRA_MS = 10`), a mesma taxa de todos os app17, e fundo
  de escala de 16 g (`setAccelRange(16)`). Troque para 2 g e chacoalhe: o grafico
  "achata" no maximo, porque o sensor saturou.
- `MPU_TYPE`: o padrao e MPU6050. Se o app17-1 mostrar `WHO_AM_I = 0x70`, troque
  para MPU6500.
