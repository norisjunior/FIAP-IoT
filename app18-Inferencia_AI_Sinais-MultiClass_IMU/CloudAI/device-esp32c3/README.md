# Inferência na nuvem — ESP32-C3 Super Mini Plus (com motor de vibração)

**Placa do professor, fora do conteúdo da aula.** É o firmware de `../device-devkitv1/` na
mesma montagem da placa pronta de coleta: ESP32-C3 Super Mini Plus, MPU6500 e um motor de
vibração. Os alunos usam a versão DevKit v1.

Publica a janela, escuta a classe e acende a saída dela. Rede, MQTT, janela, as 8 features
e o payload são **os mesmos** da versão DevKit: o mesmo fluxo n8n e a mesma API servem às duas.

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
| Conectado ao broker | LED onboard (GPIO 2) aceso | LED RGB branco até a primeira classe |
| Motor de vibração | não tem | vibra o tempo todo (IN no GPIO 0, VCC no 5V da placa) |
| `MQTT_CLIENT_ID` | `DEFINA_UM_NOME` | `IoTDevMultiClasse001` (o `device` do dataset de referência) |

Os LEDs verde e laranja entram nos pinos dos botões da coleta, 3 (COLETA) e 4 (CLASSE), como
na versão DevKit, onde os botões 21 e 18 viraram LEDs.

## Pinos

| Função | GPIO |
|---|---|
| MPU SDA | 20 |
| MPU SCL | 21 |
| LED azul + resistor 220 Ω (`operando`) | 1 |
| LED verde + resistor 220 Ω (`inclinado_frente`) | 3 |
| LED laranja + resistor 220 Ω (`inclinado_tras`) | 4 |
| LED vermelho + resistor 220 Ω (`anomalia`) | 10 |
| Buzzer passivo (`anomalia`) | 7 |
| LED RGB da placa | 8 |
| Motor de vibração (pino IN do módulo) | 0 |

## LED RGB

Repete a cor do LED aceso:

| Estado | LED | RGB |
|---|---|---|
| Sem broker | apagados | apagado |
| Conectado, a nuvem ainda não respondeu | apagados | branco |
| `operando` | azul (GPIO 1) | azul |
| `inclinado_frente` | verde (GPIO 3) | verde |
| `inclinado_tras` | laranja (GPIO 4) | laranja |
| `anomalia` | vermelho (GPIO 10) + bipe | vermelho |
| Chegou algo que não é uma das 4 classes | apagados | branco (o Serial diz o quê) |

Ao ligar, depois da calibração, a placa testa as saídas: azul, verde, laranja e vermelho, cada
LED junto com a mesma cor no RGB, e o vermelho com um bipe.

## Buzzer

Cada janela classificada como `anomalia` toca um bipe de 250 ms a 500 Hz
(`tone(BUZZER, 500, 250)`), o mesmo da versão DevKit. Como chega uma classe por segundo, uma
anomalia que dura vários segundos bipa uma vez por segundo. O bipe para sozinho quando a
classe muda.

O buzzer fica no GPIO 7, longe dos pinos de boot do C3 (2, 8 e 9).

## Motor de vibração

Medir igual: a coleta grava as quatro classes com o motor vibrando, então a inferência também
roda com ele ligado. Ele fica desligado na calibração (calibre com a placa parada), liga depois
do teste das saídas e não desliga mais, nem enquanto a placa reconecta ao Wi-Fi ou ao broker.

## n8n

O fluxo é o mesmo de `../n8n/`: troque `DEFINA_UM_NOME` por `IoTDevMultiClasse001` nos dois
nós de MQTT. Os tópicos ficam:

- a janela: `FIAPIoT/motor/multiclasse/IoTDevMultiClasse001`
- a classe: `FIAPIoT/motor/multiclasse/IoTDevMultiClasse001/cmd`

A coleta (`FIAPIoT/motor/multiclasse`, sem o nome) não recebe estas janelas: a inferência não
entra no dataset.

O `.pkl` de referência de `../api/` foi treinado com o dataset de 12/11, gravado numa montagem
anterior desta placa, antes do motor de vibração. Com o motor de vibração, colete de novo e use
o `.pkl` desse treino. Nos dois casos, incline **uns 45°** para a frente e para trás.

## Build

```bash
pio run -t upload
pio device monitor
```

Usa `espressif32@6.12.0` com `board = esp32-c3-devkitm-1`. A placa não tem chip USB-serial:
grave e abra o Monitor Serial pela própria USB-C (`ARDUINO_USB_CDC_ON_BOOT=1`). Se a gravação
não começar, segure **BOOT**, aperte **RESET** e solte **BOOT**.

**Wokwi:** ative o bloco `(A)` no topo do `.cpp`, troque `#define MPU_TYPE` para `MPU6050` e
comente `mpu.calibrateAccelGyro(&calib);` (o simulador não tem a FIFO do MPU). O LED amarelo no
GPIO 0 acende no lugar do motor de vibração.
