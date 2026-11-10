# app17-1 — Qual é o meu sensor?

Rode antes de qualquer outro app17. Ele confirma que o ESP32 enxerga o sensor e descobre
qual chip está na placa.

## Montagem

| MPU | ESP32 |
| --- | --- |
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO22 |
| SCL | GPIO23 |

O LED no GPIO4 acende quando o sensor responde.

## Como ler a saída

1. **Varredura I2C:** lista os endereços encontrados. O MPU aparece em `0x68`.
2. **WHO_AM_I:** o registrador que diz qual é o chip.

   | WHO_AM_I | Chip | Nos outros app17 |
   | --- | --- | --- |
   | `0x68` | MPU6050 (o padrão do kit) | não muda nada |
   | `0x70` | MPU6500 (vendido como "MPU6050 genérico") | trocar para `#define MPU_TYPE MPU6500` |
   | outro | outro chip | seguir a mensagem do Serial |

3. **Teste:** uma leitura em **g**. Parado e nivelado, `az` fica perto de `1.0`.

**"Nenhum dispositivo encontrado":** confira os fios SDA e SCL e a alimentação em 3V3.
