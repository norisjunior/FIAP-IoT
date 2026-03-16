# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 06 - Semáforo com Sensor de Distância e MQTT

Esta aplicação implementa um sistema de detecção de pessoas utilizando sensor ultrassônico HC-SR04. Quando detecta a presença de alguém próximo (distância menor ou igual a 50cm), o ESP32 publica os dados de distância via protocolo MQTT para um broker remoto.

### Sensores e Atuadores

**Sensores:**
- HC-SR04 (Sensor Ultrassônico de Distância) - Pinos:
  - TRIG: GPIO 25
  - ECHO: GPIO 26

### Funcionamento

O sistema realiza medições contínuas de distância. Quando detecta um objeto ou pessoa a uma distância de até 50cm, os dados são enviados via MQTT:

- **Broker MQTT:** broker.emqx.io (porta 1883)
- **Tópico de publicação:** noris/semaforo1/distancia
- **Device ID:** NorisDistSensorSemaforo00001
- **Formato dos dados:** JSON com campo "dist" contendo a distância em centímetros

O sistema mantém conexão WiFi com a rede "Wokwi-GUEST" e reconecta automaticamente ao broker MQTT caso a conexão seja perdida. Após detectar uma pessoa, aguarda 5 segundos antes de realizar nova detecção, evitando múltiplas publicações seguidas.