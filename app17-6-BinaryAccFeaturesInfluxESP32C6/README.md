# app17-6-BinaryAccFeaturesInflux (ESP32-C6)

Firmware **ESP32-C6** que calcula features de aceleração no próprio dispositivo, em janela fixa de
100 amostras (1 s @ 100 Hz), e publica 1 mensagem MQTT (JSON) por janela. Duas classes:

- **NORMAL** = MPU parado.
- **ANOMALIA** = MPU em movimento durante a coleta.

## Build

O ESP32-C6 exige o core Arduino-ESP32 3.x. O `platformio.ini` usa o platform da
comunidade [`pioarduino`](https://github.com/pioarduino/platform-espressif32)
(`board = esp32-c6-devkitc-1`); o platform oficial `platformio/espressif32` ainda
não suporta o C6 no framework Arduino. Grave/monitore pela porta **UART** da placa
(ponte USB-serial). Para usar a porta **USB** nativa, ative
`-D ARDUINO_USB_CDC_ON_BOOT=1` no `platformio.ini`.

## Pinos

| Função | GPIO |
|---|---|
| MPU SDA | 22 |
| MPU SCL | 23 |
| Botão COLETA (inicia/para o envio) | 21 |
| Botão ANOMALIA (seleciona NORMAL/ANOMALIA, só com a coleta parada) | 18 |
| LED externo (aceso = NORMAL, piscando = ANOMALIA) | 4 |
| LED onboard = LED RGB WS2812 da DevKitC-1 (`RGB_BUILTIN`) | 8 |

## Uso

1. **Wokwi:** ative o bloco `(A)` no topo do `.cpp` e comente `mpu.calibrateAccelGyro(&calib);`.
2. **ESP32 físico:** ative o bloco `(B)` e ajuste WiFi + IP do broker MQTT.
3. Compile e rode. Coleta parada: botão 18 escolhe a condição. Botão 21 inicia/para a coleta.

## Payload

Tópico `FIAPIoT/motor/features` — 1 JSON por janela:

```json
{
  "device": "IoTDevJanelaFixa001",
  "label": "ligado_anomalia",
  "ts_epoch_ms": 1749760205123,
  "mean_ax": 0.021, "mean_ay": -0.015, "mean_az": 9.812,
  "std_ax": 1.402,  "std_ay": 1.187,  "std_az": 0.945,
  "rms_ax": 1.403,  "rms_ay": 1.190,  "rms_az": 9.857,
  "rms_mag": 10.12
}
```

`ts_epoch_ms` = epoch em ms (UTC), via NTP + `millis()`. Rótulos: `ligado_normal` / `ligado_anomalia`.
