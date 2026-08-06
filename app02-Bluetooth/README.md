# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 02 - Uso de Namespaces, Structs e Funções

Esta aplicação demonstra conceitos avançados de programação em C++ para ESP32, incluindo o uso de namespaces, structs e funções organizadas em módulos. O sistema coleta múltiplas medições de temperatura e umidade através de um botão, calculando a média e avaliando as condições ambientais.

### Sensores e Atuadores

**Sensores:**
- DHT22 (Temperatura e Umidade) - Pino GPIO 21
- Botão Push - Pino GPIO 18 (com pull-up interno)

**Atuadores:**
- LED Vermelho - Pino GPIO 26 (Acende quando temperatura média > 35°C ou umidade média > 80%)

### Funcionamento

O sistema aguarda o pressionamento do botão para realizar cada medição. Após 5 leituras consecutivas, calcula a média de temperatura e umidade. Se a temperatura média ultrapassar 35°C ou a umidade média ultrapassar 80%, o LED vermelho é acionado, indicando condições ambientais inadequadas.

### Conceitos Demonstrados

- Organização de código com namespaces (ESP32Sensors)
- Uso de structs para armazenar dados de sensores
- Separação de funcionalidades em módulos (headers .hpp)
- Arrays para armazenamento de múltiplas leituras
- Funções para processamento de dados agregados