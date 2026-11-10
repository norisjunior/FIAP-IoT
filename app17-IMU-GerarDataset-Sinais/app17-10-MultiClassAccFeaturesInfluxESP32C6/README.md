# app17-10-MultiClassAccFeaturesInfluxESP32C6

Coletor multiclasse para a placa **ESP32-C6-DevKitC-1**. É o mesmo firmware da versão ESP32
DevKit: mesmas features, mesmo tópico, mesmo payload e mesmo measurement. Mudam só a placa, os
pinos e o LED onboard, que no C6 é o LED RGB.

Firmware que calcula 8 features de aceleração no próprio dispositivo, em janela fixa de 100
amostras (1 s @ 100 Hz), e publica 1 mensagem MQTT (JSON) por janela. Quatro classes, sempre com
o motor ligado:

- **operando**, **inclinado_frente**, **inclinado_tras**, **anomalia**.

A cada volta completa pela sequência o contador `rodada` incrementa — é o grupo do
`LeaveOneGroupOut` na validação.

## Build

O ESP32-C6 exige o core Arduino-ESP32 3.x. O `platformio.ini` usa o platform da comunidade
`pioarduino` (`board = esp32-c6-devkitc-1`), numa pasta própria (`platforms-pioarduino`), para
não mudar o platform padrão dos outros projetos. Grave e monitore pela porta **UART** da placa.

## Pinos

| Função | GPIO |
|---|---|
| MPU SDA | 5 |
| MPU SCL | 4 |
| Botão COLETA (inicia/para a coleta) | 12 |
| Botão CLASSE (avança a sequência, só com a coleta parada) | 13 |
| LED externo (pisca N vezes = índice da classe atual, 1..4) | 7 |
| LED RGB da DevKitC-1 (cor do estado, ver abaixo) | 8 |

## LED RGB

| Estado | Cor |
|---|---|
| Esperando a coleta | branco |
| Coletando `operando` | verde |
| Coletando `inclinado_frente` | amarelo |
| Coletando `inclinado_tras` | roxo |
| Coletando `anomalia` | vermelho |

As cores estão em `COR_ESPERA` e `COR_CLASSE`, na mesma ordem da `SEQUENCIA`.

## Uso

1. **Wokwi:** ative o bloco `(A)` no topo do `.cpp` e comente `mpu.calibrateAccelGyro(&calib);`.
2. **ESP32-C6 físico:** ative o bloco `(B)` e ajuste WiFi + IP do broker MQTT.
3. Calibrar com o motor na posição inicial/de uso, sem movê-lo.
4. Coleta parada: botão 13 escolhe a classe. Botão 12 inicia a coleta, que para sozinha ao
   completar 30 janelas. Repita para as 4 classes; recomendado 3 rodadas completas.

## Payload

Tópico `FIAPIoT/motor/multiclasse` — 1 JSON por janela:

```json
{
  "device": "IoTDevMultiClasse001",
  "label": "inclinado_frente",
  "rodada": 1,
  "janela": 7,
  "ts_ms": 85123,
  "mean_ax": 0.328, "mean_ay": -0.008, "mean_az": 0.929,
  "std_ax": 0.036,  "std_ay": 0.030,  "std_az": 0.042,
  "std_mag": 0.046,
  "p2p_mag": 0.215
}
```

`rodada` = volta completa pela sequência de classes. `ts_ms` = tempo do ESP32 em ms desde o boot (`millis()`).

Os valores estão em **`g`**. Nivelado, `mean_az` fica perto de `1.0`; inclinado 25°, parte da
gravidade migra para `mean_ax` (`sen 25° ≈ 0,42`).

## Node-RED e treino

O fluxo em `FluxoNodeRED-MultiClass/` é o mesmo da versão ESP32 DevKit (mesmos IDs de nó): se
ele já estiver importado, não precisa importar de novo. Ele grava no measurement
`vibracao_multiclasse`.

O treino também é o mesmo: o notebook `colab/treinamento_multiclasse.ipynb` da versão ESP32
DevKit lê essas janelas do InfluxDB e gera o `modelo_motor_multiclasse.pkl`.
