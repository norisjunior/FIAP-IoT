# app17-8 - Motor + features + MQTT (base do ML)

Junta tudo: motor, janela de 100 amostras (100 Hz), features do app17-5,
label escolhida por botao e publicacao de 1 JSON por janela no MQTT.
E a **base canonica** de onde derivam os app17-9 e app17-10.

## Pinos

| Funcao | GPIO |
| --- | --- |
| MPU SDA / SCL | 19 / 18 |
| Botao motor (liga/desliga) | 26 |
| Botao anomalia (so com motor desligado) | 25 |
| LED (aceso = anomalia) | 27 |
| Motor IN1 / IN2 | 22 / 23 |

## Configurar antes de rodar

No topo do `.cpp`: `WIFI_SSID`, `WIFI_PASSWORD` e `MQTT_SERVER` (IP do broker).
Topico: `FIAPIoT/motor/features`.

## Labels

| Label | Quando |
| --- | --- |
| `parado` | motor desligado |
| `ligado_normal` | motor ligado, helice equilibrada |
| `ligado_anomalia` | motor ligado, com fita na helice (desbalanceio) |

## Aula

- Serial imprime CSV (`label,mean_*,std_*,rms_*,rms_mag`) e o payload MQTT enviado.
- Colete rodadas separadas por label: e esse dataset que treina o modelo.
