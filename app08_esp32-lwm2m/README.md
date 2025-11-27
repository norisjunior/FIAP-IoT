# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 08 - LwM2M (Lightweight M2M) para ESP32

Esta aplicação implementa um cliente LwM2M no ESP32, utilizando o protocolo CoAP para comunicação com um servidor Leshan. O sistema gerencia objetos IPSO (Internet Protocol for Smart Objects) para sensores de temperatura e umidade, além de controlar LEDs remotamente.

### Sensores e Atuadores

**Sensores:**
- DHT22 #1 (Temperatura e Umidade) - Pino GPIO 19
- DHT22 #2 (Temperatura e Umidade) - Pino GPIO 18

**Atuadores:**
- LED Vermelho - Pino GPIO 25 (Indicador de erro)
- LED Azul/Ciano - Pino GPIO 26 (Indicador de sistema pronto)
- LED Amarelo - Pino GPIO 27 (Indicador de conexão WiFi)

### Funcionamento

O sistema implementa o protocolo LwM2M usando CoAP para comunicação com o servidor Leshan (leshan.eclipseprojects.io:5683). Os seguintes objetos IPSO são disponibilizados:

**Objetos LwM2M:**
- **3303:** Temperature Sensor (2 instâncias - um para cada DHT22)
- **3304:** Humidity Sensor (2 instâncias - um para cada DHT22)
- **3311:** Light Control (3 instâncias - controle dos 3 LEDs)

**Indicadores LED:**
- LED Amarelo pisca durante conexão WiFi
- LED Azul acende quando sistema está pronto
- LED Vermelho indica erros de conexão

O dispositivo se registra automaticamente no servidor Leshan com um endpoint único e mantém comunicação ativa. Os sensores podem ser monitorados e os LEDs podem ser controlados remotamente através da interface web do Leshan.