# app17-6-BinaryAccFeaturesInflux

Firmware ESP32 que calcula features de aceleração no próprio dispositivo, em janela fixa de
100 amostras (1 s @ 100 Hz), e publica 1 mensagem MQTT (JSON) por janela. Duas classes:

- **NORMAL** = MPU parado.
- **ANOMALIA** = MPU em movimento durante a coleta.

## Pinos

| Função | GPIO |
|---|---|
| MPU SDA | 22 |
| MPU SCL | 23 |
| Botão COLETA (inicia/para o envio) | 21 |
| Botão ANOMALIA (seleciona NORMAL/ANOMALIA, só com a coleta parada) | 18 |
| LED externo (aceso = NORMAL, piscando = ANOMALIA) | 4 |
| LED onboard | 2 |

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
