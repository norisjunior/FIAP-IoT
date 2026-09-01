# app17-7-MultiClassAccFeaturesInflux

Firmware ESP32 que calcula features de aceleração no próprio dispositivo, em janela fixa de
100 amostras (1 s @ 100 Hz), e publica 1 mensagem MQTT (JSON) por janela. Evolução multiclasse
do `app17-6` (binário: normal vs anomalia). Quatro classes, sempre com o motor ligado:

- **operando**, **inclinado_frente**, **inclinado_tras**, **anomalia**.

## Pinos

| Função | GPIO |
|---|---|
| MPU SDA | 22 |
| MPU SCL | 23 |
| Botão COLETA (inicia/para a coleta) | 21 |
| Botão CLASSE (avança a sequência, só com a coleta parada) | 18 |
| LED externo (pisca N vezes = índice da classe atual, 1..4) | 4 |
| LED onboard (aceso = coleta em andamento) | 2 |

## Uso

1. **Wokwi:** ative o bloco `(A)` no topo do `.cpp`, use `#define MPU_TYPE MPU6050` e comente
   `mpu.calibrateAccelGyro(&calib);`. As classes de inclinação não têm equivalente fiel no
   simulador — serve para testar botões, LED e publicação MQTT.
2. **ESP32 físico:** ative o bloco `(B)` e ajuste WiFi + IP do broker MQTT.
3. Calibrar com o motor na posição inicial/de uso, sem movê-lo.
4. Coleta parada: botão 18 escolhe a classe. Botão 21 inicia a coleta, que para sozinha ao
   completar 30 janelas. Repita para as 4 classes; recomendado 3 rodadas completas.

## Payload

Tópico `FIAPIoT/motor/multiclasse` — 1 JSON por janela:

```json
{
  "device": "IoTDevMultiClasse001",
  "label": "inclinado_frente",
  "rodada": 1,
  "janela": 7,
  "ts_epoch_ms": 1749760205123,
  "mean_ax": 3.214, "mean_ay": -0.082, "mean_az": 9.114,
  "std_ax": 0.351,  "std_ay": 0.298,  "std_az": 0.412,
  "std_mag": 0.447,
  "p2p_mag": 2.108
}
```

`rótulos`: `operando` / `inclinado_frente` / `inclinado_tras` / `anomalia`. `rodada` = volta
completa pela sequência de classes. `ts_epoch_ms` = epoch em ms (UTC), via NTP + `millis()`.
