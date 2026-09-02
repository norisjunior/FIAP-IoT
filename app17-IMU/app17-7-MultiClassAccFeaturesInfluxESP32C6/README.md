# app17-7-MultiClassAccFeaturesInflux (ESP32-C6)

Firmware **ESP32-C6** que calcula 8 features de aceleração no próprio dispositivo, em janela
fixa de **100 amostras (1 s @ 100 Hz)**, e publica 1 mensagem MQTT (JSON) por janela. Evolução
multiclasse do `app17-6` (binário: normal vs anomalia). Quatro classes, sempre com o motor
ligado:

- **operando**, **inclinado_frente**, **inclinado_tras**, **anomalia**.

A cada volta completa pela sequência o contador `rodada` incrementa — é o grupo do
`LeaveOneGroupOut` na validação.

> Mesma lógica e mesmas features do app17-7 original — só muda o alvo (ESP32-C6) e os pinos,
> agora iguais aos do **app17-6**.

## Build

O ESP32-C6 exige o core Arduino-ESP32 3.x. O `platformio.ini` usa o platform da comunidade
[`pioarduino`](https://github.com/pioarduino/platform-espressif32) (`board =
esp32-c6-devkitc-1`); o platform oficial `platformio/espressif32` ainda não suporta o C6 no
framework Arduino. Grave/monitore pela porta **UART** da placa (ponte USB-serial). Para usar
a porta **USB** nativa do C6, ative `-D ARDUINO_USB_CDC_ON_BOOT=1` no `platformio.ini`.

## Pinos (iguais aos do app17-6)

| Função | GPIO |
|---|---|
| MPU SDA | 5 |
| MPU SCL | 4 |
| Botão **COLETA** (inicia/para a coleta) | 12 |
| Botão **CLASSE** (avança a sequência, só com a coleta parada) | 13 |
| LED externo (pisca N vezes = índice da classe atual, 1..4) | 7 |
| LED onboard = LED RGB WS2812 da DevKitC-1 (`RGB_BUILTIN`, aceso = coleta em andamento) | 8 |

Sensor ativo no código: **MPU6050** (`#define MPU_TYPE`). Para o MPU6500, troque a linha no
topo do `.cpp`.

## Uso

1. **Wokwi:** ative o bloco `(A)` no topo do `.cpp`, use `#define MPU_TYPE MPU6050` e comente
   `mpu.calibrateAccelGyro(&calib);` (trava no simulador, FIFO ausente). As classes de
   inclinação não têm equivalente fiel no simulador — serve para testar botões, LED e
   publicação MQTT.
2. **ESP32-C6 físico:** ative o bloco `(B)` e ajuste WiFi + IP do broker MQTT.
3. Calibrar com o motor na posição inicial/de uso, sem movê-lo.
4. Coleta parada: botão 13 escolhe a classe. Botão 12 inicia a coleta, que para sozinha ao
   completar 30 janelas. Repita para as 4 classes; recomendado 3 rodadas completas.

## Payload

Tópico `FIAPIoT/motor/multiclasse`:

```json
{
  "device": "IoTDevMultiClasse001",
  "label": "inclinado_frente",
  "rodada": 1,
  "janela": 7,
  "ts_epoch_ms": 1749760205123,
  "mean_ax": 0.328, "mean_ay": -0.008, "mean_az": 0.929,
  "std_ax": 0.036,  "std_ay": 0.030,  "std_az": 0.042,
  "std_mag": 0.046,
  "p2p_mag": 0.215
}
```

`rótulos`: `operando` / `inclinado_frente` / `inclinado_tras` / `anomalia`. `rodada` = volta
completa pela sequência de classes. `ts_epoch_ms` = epoch em ms (UTC), via NTP + `millis()`.

O mesmo conteúdo sai como CSV no Monitor Serial, com o cabeçalho impresso no boot:

```
ts_epoch_ms,label,rodada,janela,mean_ax,mean_ay,mean_az,std_ax,std_ay,std_az,std_mag,p2p_mag
```

Os valores estão em **`g`**: o FastIMU devolve aceleração em `g` e o firmware publica o número
cru. Nivelado, `mean_az` fica perto de `1.0`; inclinado 25°, parte da gravidade migra para
`mean_ax` (`sen 25° ≈ 0,42`).

## Treino

[`analytics/notebooks/2.6_multiclasse_motor.ipynb`](../analytics/notebooks/2.6_multiclasse_motor.ipynb)
lê estas janelas do InfluxDB (measurement `vibracao_multiclasse`) e gera o
`modelo_motor_multiclasse.pkl`.
