# Aplicação 06 — publicador MQTT

Este é o ESP32 "publisher". Ele mede a distância com o HC-SR04 e publica o
valor, em centímetros, no tópico `fiap/iot/distancia`.

```text
HC-SR04 → ESP32 → publish → broker MQTT
```

## Ligações

| HC-SR04 | ESP32 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| TRIG | GPIO 22 |
| ECHO | GPIO 23 |

## Antes de gravar

No início do arquivo `src/app06-publisher-distancia.ino`, ajuste:

- `WIFI_SSID`
- `WIFI_SENHA`
- `BROKER_IP`

Quando o sensor não recebe eco, o programa publica `400 cm`, e não `0 cm`.
Assim, nenhum LED acende por engano quando não há objeto diante do sensor.

Para subir o código para o dispositivo: PlatformIO -> Upload

Para visualizar as mensagens no Monitor Serial:
F1 -> PlatformIO New Terminal:

```bash
pio device monitor -b 115200
```

O assinante está na [Aplicação 07](../app07_mqtt_subscriber_atuador/).
