# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 15 - Arquitetura Cloud Computing com MQTT

Esta aplicação demonstra arquitetura de computação em nuvem (Cloud Computing) para IoT, implementando um sistema de monitoramento de motor industrial. O ESP32 coleta dados de sensores e publica via MQTT, além de receber comandos remotos para controle do motor.

## Pré-requisitos

### Inicializar a Plataforma IoT

Esta aplicação requer a plataforma IoT completa rodando. Siga as instruções em `IoT-platform/README.md`:

1. **Acessar WSL2 Ubuntu:**
   ```bash
   wsl -d ubuntu
   ```

2. **Clonar o repositório (se ainda não clonou):**
   ```bash
   cd ~
   git clone https://github.com/norisjunior/FIAP-IoT
   ```

3. **Iniciar todos os serviços:**
   ```bash
   cd FIAP-IoT/IoT-platform/
   sudo ./start-linux.sh
   ```

Isso iniciará: MQTT Broker, Node-RED, n8n, InfluxDB e Grafana.

**Serviços disponíveis:**
- MQTT Broker: localhost:1883
- Node-RED: http://localhost:1880 (admin/FIAPIoT)
- n8n: http://localhost:5678
- InfluxDB: http://localhost:8086 (admin/FIAP@123)
- Grafana: http://localhost:3000 (admin/admin)

### Sensores e Atuadores

**Sensores:**
- DHT22 (Temperatura e Umidade) - Pino GPIO 26
- MPU6050 (Acelerômetro/Giroscópio) - Pinos I2C:
  - SDA: GPIO 18
  - SCL: GPIO 19

**Atuadores:**
- LED - Pino GPIO 27 (Indicador de status do motor)

### Funcionamento

O sistema implementa comunicação bidirecional via MQTT, enviando dados de sensores e recebendo comandos de controle:

**Tópico de Publicação (Dados):**
- `FIAPIoT/aula08/noris/motor/dados`
- Envia dados a cada 3 segundos em formato JSON
- Campos: temperatura, umidade, aceleração (x, y, z), status do motor

**Tópico de Subscrição (Comandos):**
- `FIAPIoT/aula08/noris/motor/cmd`
- Recebe comandos para ligar/desligar o motor
- Comandos: "ligar", "desligar"

**Configuração MQTT:**
- Broker: host.wokwi.internal (porta 1883)
- Client ID: IoTDeviceNoris001 (único no broker)

**Arquitetura demonstrada:**
- **Edge:** Sensores e atuadores no ESP32
- **Fog:** Processamento intermediário (broker MQTT)
- **Cloud:** Análise e armazenamento de dados históricos

O LED acende quando o motor está operando e apaga quando recebe comando de desligamento, simulando controle remoto de equipamento industrial.