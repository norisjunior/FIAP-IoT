# A torre do pivô — ESP32-C3 Super Mini Plus (com motor de vibração)

**Placa do professor, fora do conteúdo da aula.** É o firmware de
[`../device-torre-devkit/`](../device-torre-devkit/CONSTRUIR-O-FIRMWARE.md) na montagem do
app18 (`device-esp32c3`): ESP32-C3 Super Mini Plus, MPU6500 e um motor de vibração. Os alunos
usam a versão DevKit v1.

A rede neural (TFLite) decide na placa, acende a saída, publica a janela com a decisão e manda
a ordem para a bomba do pivô. A janela, as 8 features, os dois `.hpp`, os JSON e os tópicos
são **os mesmos** da versão DevKit.

## O que muda em relação à versão DevKit v1

| | DevKit v1 | Esta versão |
|---|---|---|
| Placa | ESP32 DevKit v1 | ESP32-C3 Super Mini Plus (USB nativa) |
| MPU SDA / SCL | 22 / 23 | 20 / 21 |
| `operando` | LED azul (GPIO 4) | LED azul (GPIO 1) |
| `inclinado_frente` | LED amarelo (GPIO 21) | LED verde (GPIO 3) |
| `inclinado_tras` | LED vermelho (GPIO 18) | LED laranja (GPIO 4) |
| `anomalia` | buzzer (GPIO 19) | LED vermelho (GPIO 10) + o mesmo bipe no buzzer passivo (GPIO 7) |
| LED RGB da placa | não tem | a mesma cor do LED aceso |
| Conectado ao broker | LED onboard (GPIO 2) aceso | só no Serial: o RGB repete a classe |
| Motor de vibração | não tem | vibra o tempo todo (IN no GPIO 0, VCC no 5V da placa) |
| `MQTT_CLIENT_ID` | `DEFINA_UM_NOME` | `IoTDevMultiClasse001` |

A montagem e o porquê de cada pino estão no
[README do app18](../../app18-Inferencia_AI_Sinais-MultiClass_IMU/device-esp32c3/README.md).

## Tópicos

- `FIAPIoT/motor/borda/IoTDevMultiClasse001`: a janela e a classe. Os fluxos que guardam e
  conferem (`app20-Inferencia-Borda/n8n/`) servem sem mudança, com `IoTDevMultiClasse001` no
  gatilho **Janela da placa**.
- `FIAPIoT/pivo/IoTDevMultiClasse001/cmd/local`: a ordem para a bomba. É a torre que a
  [bomba](../device-bomba-devkit/) obedece ao ligar (`PIVO_PADRAO`).

## O modelo

Os dois `.hpp` de `src/` são os mesmos da versão DevKit: a rede do dataset de referência
(12/11), gravado numa montagem anterior desta placa, sem o motor de vibração.

## Build

```bash
pio run -t upload
pio device monitor
```

Usa `espressif32@6.12.0` com `board = esp32-c3-devkitm-1`. A placa não tem chip USB-serial:
grave e abra o Monitor Serial pela própria USB-C (`ARDUINO_USB_CDC_ON_BOOT=1`). Se a gravação
não começar, segure **BOOT**, aperte **RESET** e solte **BOOT**.

Flash / RAM: 80,1% / 16,8% (a versão DevKit: 81,0% / 18,7%). A primeira compilação demora uns
7 minutos (a biblioteca do TFLite).

**Wokwi:** ative o bloco `(A)` no topo do `.cpp`, troque `#define MPU_TYPE` para `MPU6050` e
comente `mpu.calibrateAccelGyro(&calib);`. O LED amarelo no GPIO 0 acende no lugar do motor de
vibração.
