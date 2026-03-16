# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 07 - Semáforo Inteligente com Controle via MQTT

Esta aplicação implementa um semáforo inteligente que recebe dados de detecção de pessoas via MQTT e controla automaticamente as fases do semáforo. O sistema conta detecções de pedestres em uma janela temporal de 60 segundos e aciona a fase vermelha quando necessário.

### Sensores e Atuadores

**Atuadores:**
- LED Vermelho (Semáforo) - Pino GPIO 18
- LED Amarelo (Semáforo) - Pino GPIO 17
- LED Verde (Semáforo) - Pino GPIO 16

### Funcionamento

O sistema se inscreve no tópico MQTT **noris/semaforo1/distancia** e recebe mensagens JSON com dados de distância de pessoas detectadas:

**Lógica de Controle:**
1. Permanece em fase verde por padrão
2. Conta detecções de pessoas (distância ≤ 50cm) em uma janela de 60 segundos
3. Quando detecta 2 ou mais pessoas em 60 segundos:
   - Muda para fase amarela (2 segundos)
   - Muda para fase vermelha (5 segundos)
   - Volta para fase verde
   - Reinicia a contagem

**Configurações MQTT:**
- Broker: broker.emqx.io (porta 1883)
- Tópico de subscrição: noris/semaforo1/distancia
- Device ID: NorisSemaforo00001
- QoS: 1

O sistema mantém conexão WiFi com a rede "Wokwi-GUEST" e reconecta automaticamente ao broker MQTT caso a conexão seja perdida.