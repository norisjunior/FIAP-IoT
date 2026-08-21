# Aplicação 07 — assinante MQTT

Este é o ESP32 dos alunos. Todos assinam o mesmo tópico e acendem o LED quando
a distância publicada pelo professor é menor ou igual ao limite.

```text
broker MQTT → subscribe → ESP32 → LED
```

## Ligação

Ligue o GPIO 21 ao resistor de 220 Ω, o resistor ao ânodo do LED e o cátodo ao
GND.

## Antes de gravar

No início do arquivo `src/app07-subscriber-atuador.ino`, ajuste:

- `WIFI_SSID`
- `WIFI_SENHA`
- `BROKER_IP`
- `LIMITE`, se quiser mudar a distância que acende o LED

Não é necessário informar número de grupo. O código usa o endereço MAC para
dar automaticamente um nome MQTT diferente a cada ESP32.

```bash
pio run -t upload
pio device monitor
```

O publicador está na [Aplicação 06](../app06_mqtt_publisher_distancia/).
