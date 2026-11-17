# Inferência na nuvem — ESP32-C3 Super Mini Plus (com microservo)

**Placa do professor, fora do conteúdo da aula.** É o firmware de `../device-devkitv1/` na
mesma montagem que gravou o dataset de referência: ESP32-C3 Super Mini Plus, MPU6500 e um
microservo que vai e volta. Os alunos usam a versão DevKit v1.

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
| `anomalia` | buzzer (GPIO 19) | LED vermelho (GPIO 10) |
| LED RGB da placa | não tem | a mesma cor do LED aceso |
| Conectado ao broker | LED onboard (GPIO 2) aceso | LED RGB branco até a primeira classe |
| Servo | não tem | vai e volta o tempo todo (GPIO 0, 5V da placa) |
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
| LED RGB da placa | 8 |
| Servo (sinal) | 0 |

## LED RGB

Repete a cor do LED aceso:

| Estado | LED | RGB |
|---|---|---|
| Sem broker | apagados | apagado |
| Conectado, a nuvem ainda não respondeu | apagados | branco |
| `operando` | azul (GPIO 1) | azul |
| `inclinado_frente` | verde (GPIO 3) | verde |
| `inclinado_tras` | laranja (GPIO 4) | laranja |
| `anomalia` | vermelho (GPIO 10) | vermelho |
| Chegou algo que não é uma das 4 classes | apagados | branco (o Serial diz o quê) |

Ao ligar, depois da calibração, a placa testa as saídas: azul, verde, laranja e vermelho, cada
LED junto com a mesma cor no RGB.

## Servo

Medir igual: o dataset de referência foi gravado com o servo indo e voltando nas quatro
classes, então a inferência também roda com ele em movimento — meio segundo em 60°, meio em
120°. Ele começa depois da calibração (calibre com a placa parada) e não para mais.

## n8n

O fluxo é o mesmo de `../n8n/`: troque `DEFINA_UM_NOME` por `IoTDevMultiClasse001` nos dois
nós de MQTT. Os tópicos ficam:

- a janela: `FIAPIoT/motor/multiclasse/IoTDevMultiClasse001`
- a classe: `FIAPIoT/motor/multiclasse/IoTDevMultiClasse001/cmd`

A coleta (`FIAPIoT/motor/multiclasse`, sem o nome) não recebe estas janelas: a inferência não
entra no dataset.

O `.pkl` de referência de `../api/` foi treinado com esta montagem: incline **uns 45°** para
a frente e para trás.

## Build

```bash
pio run -t upload
pio device monitor
```

Usa `espressif32@6.12.0` com `board = esp32-c3-devkitm-1`. A placa não tem chip USB-serial:
grave e abra o Monitor Serial pela própria USB-C (`ARDUINO_USB_CDC_ON_BOOT=1`). Se a gravação
não começar, segure **BOOT**, aperte **RESET** e solte **BOOT**.

**Wokwi:** ative o bloco `(A)` no topo do `.cpp`, troque `#define MPU_TYPE` para `MPU6050` e
comente `mpu.calibrateAccelGyro(&calib);` (o simulador não tem a FIFO do MPU).
