# Smart Ring TinyML — XIAO nRF52840 Sense Plus

Anel inteligente experimental que reconhece gestos do dedo com um modelo TinyML rodando na
própria placa e envia o gesto reconhecido por Bluetooth Low Energy.

```
IMU (6 eixos) → janela temporal → pré-processamento → modelo TinyML → classe → BLE → ação
```

O treino acontece fora (Edge Impulse); a inferência, sempre na placa, sem internet.

## Etapas

Cada etapa é uma pasta com um projeto PlatformIO próprio. Só se avança depois de validar a anterior.

| Pasta | Etapa | Estado |
|---|---|---|
| [etapa1-IMU](etapa1-IMU/) | 1 — Hardware e ambiente | em andamento (teste 1.1) |
| — | 2 — Definição e coleta dos gestos | — |
| — | 3 — Treinamento (Edge Impulse) | — |
| — | 4 — Inferência embarcada | — |
| — | 5 — Comunicação BLE | — |
| — | 6 — Bateria e protótipo V1 no dedo | — |
| — | 7 — Case 3D e otimização | — |

## Hardware

| Item | Especificação |
|---|---|
| Placa | Seeed Studio XIAO nRF52840 **Sense Plus** |
| SoC | nRF52840 — Cortex-M4F a 64 MHz, 256 KB RAM, 1 MB Flash (+ 2 MB QSPI), BLE |
| IMU | LSM6DS3TR-C, I²C interno (`Wire1`: SDA P0.07, SCL P0.27), endereço 0x6A, alimentada pelo P1.08, INT1 no P0.11 |
| Bateria | LiPo 300 mAh — **ainda não conectada**; tensão, polaridade e conector validados só na Etapa 6 |
| Fixação | velcro no dedo (protótipo V1) |

### Sense × Sense Plus

Para este projeto a parte que importa é igual: mesmo SoC, mesma IMU nos mesmos pinos, mesmo LED
RGB e microfone PDM. A Plus acrescenta 9 pinos no verso (D11–D19: I²S, UART2, SPI1, NFC e a
leitura da bateria em D16/P0.31) e mudou o pad da bateria de lugar. Com isso a numeração Arduino
dos pinos mudou (`PINS_COUNT` 33 → 39, `PIN_VBAT` 32 → 35): código que usa nomes (`LED_BUILTIN`,
`PIN_VBAT`) funciona nas duas; código com número de pino fixo, não. Por isso o projeto usa a board
da **Sense Plus**, e não a da Sense.

## Ambiente e decisões

### PlatformIO: plataforma da Seeed

```ini
platform = https://github.com/Seeed-Studio/platform-seeedboards.git#1.1.1
board = seeed-xiao-afruitnrf52-nrf52840-sense-plus
framework = arduino
platform_packages =
    framework-arduinoadafruitnrf52 @ https://files.seeedstudio.com/arduino/core/nRF52/Seeed_nRF52_Boards-1.1.13.tar.bz2
```

- A plataforma `platform-seeedboards` é a da própria Seeed; fica fixada na tag `1.1.1`.
- **O core é fixado à parte, na 1.1.13** — a mesma versão que a Arduino IDE instala hoje para a
  Sense Plus. Sem a linha `platform_packages`, a plataforma baixa um arquivo de URL sem versão
  (de 06/2025, rotulado "1.1.1") que já tem o variant da Plus, mas sem a correção de D17/D19 de
  05/2026. Com a URL versionada o build fica reproduzível.
- Ela é uma plataforma à parte (nome `SeeedStudio`): não interfere no `espressif32@6.12.0`.
- Atenção: os pacotes do PlatformIO são compartilhados. Um projeto **sem** a linha
  `platform_packages` faz o PlatformIO trocar o core instalado pelo arquivo antigo (e vice-versa).
  Todas as etapas deste projeto usam a mesma linha.
- No nRF52 não existe "core 2.x / 3.x" como no ESP32: as opções são os dois cores da Seeed abaixo,
  cada um com a sua numeração.

### Core Arduino: Seeed nRF52 (Adafruit-based), não o mbed

A Seeed oferece dois cores para a XIAO nRF52840. No PlatformIO:

| | Adafruit-based — **escolhido** | mbed |
|---|---|---|
| Board | `seeed-xiao-afruitnrf52-nrf52840-sense-plus` | `seeed-xiao-mbed-nrf52840-sense-plus` |
| Variant | o da Sense Plus | **o da Sense original** (o JSON da board aponta para `SEEED_XIAO_NRF52840_SENSE`) |
| Versão | Seeed nRF52 Boards **1.1.13** (05/2026) | ArduinoCore-mbed 2.9.2 no PIO (2023); 2.9.3 na IDE |
| Sense Plus na Arduino IDE | listada desde a 1.1.9 | **não listada**, nem na 2.9.3 |
| BLE | Bluefruit (SoftDevice S140 7.3.0), com perfis HID | ArduinoBLE |
| Seeed recomenda para | BLE e baixo consumo | Machine Learning, IMU e PDM |

Motivos da escolha: é o único core que a Seeed declara compatível com a Sense Plus (o mbed não a
lista nem na Arduino IDE), o BLE é maduro (e o HID permite, na Etapa 5, o anel agir como teclado
BLE sem app próprio) e o consumo é menor, o que pesa com 300 mAh. Fora do Arduino, o Zephyr tem a
Sense Plus como alvo oficial, mas foge do requisito de framework Arduino.

**Risco assumido:** Edge Impulse + core Adafruit-based não é combinação oficial (a Edge Impulse
trata a XIAO como placa da comunidade). Mitigação: no primeiro export do modelo (Etapa 3),
compilar o exemplo da biblioteca EI neste ambiente **antes** de iniciar a Etapa 4. Plano B: trocar
para a board mbed — muda a linha `board =` e o código BLE; o código da IMU não muda.

### Biblioteca da IMU: Seeed_Arduino_LSM6DS3 2.0.7

- É a das referências da Seeed (XIAOEI, TFLite).
- Ela liga a IMU (P1.08) e usa o `Wire1` sozinha, **mas só reconhece a Sense Plus a partir da
  versão 2.0.5** (macro `TARGET_SEEED_XIAO_NRF52840_SENSE_PLUS`). Versões anteriores compilam e
  procuram a IMU no I²C externo (D4/D5). Por isso a versão fica fixada.
- Alternativas descartadas: FastIMU e Adafruit_LSM6DS exigiriam ligar o P1.08 e escolher o `Wire1`
  à mão.

### Pontos anotados para etapas futuras

- **Etapa 6 (bateria):** carregador BQ25101, corrente de 50 mA ou 100 mA escolhida pelo P0.13;
  a wiki da Seeed manda manter o P0.14 em LOW (com ele em HIGH o P0.31 pode passar do limite de
  3,6 V); na Plus o pad da bateria mudou de lugar — conferir na placa física antes de soldar.

## Referências

- Seeed Wiki — XIAO nRF52840 Series: https://wiki.seeedstudio.com/XIAO_BLE/
- Seeed Wiki — Motion Recognition com Edge Impulse: https://wiki.seeedstudio.com/XIAOEI/
- Seeed Wiki — TensorFlow Lite Getting Started: https://wiki.seeedstudio.com/XIAO-BLE-Sense-TFLite-Getting-Started/
- Seeed Wiki — XIAO nRF52840 com PlatformIO: https://wiki.seeedstudio.com/xiao_nrf52840_with_platform_io/
- Plataforma PlatformIO da Seeed: https://github.com/Seeed-Studio/platform-seeedboards
- Core Seeed nRF52 (Adafruit-based): https://github.com/Seeed-Studio/Adafruit_nRF52_Arduino
- Biblioteca da IMU: https://github.com/Seeed-Studio/Seeed_Arduino_LSM6DS3
- Edge Impulse — Seeed XIAO nRF52840 Sense: https://docs.edgeimpulse.com/docs/edge-ai-hardware/mcu/seeed-xiao-nrf52840-sense
