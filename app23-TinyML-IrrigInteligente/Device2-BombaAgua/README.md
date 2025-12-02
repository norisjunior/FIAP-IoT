# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 16 - Arquitetura Edge/Fog Computing

Esta aplicação demonstra arquitetura de computação de borda (Edge Computing) e névoa (Fog Computing), implementando processamento local próximo aos sensores. O sistema conecta-se a um broker MQTT rodando em Raspberry Pi (camada Fog) para processamento intermediário antes do envio à nuvem.

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
- MQTT Broker: localhost:1883 (nome do container: mqtt-broker)
- Node-RED: http://localhost:1880 (admin/FIAPIoT)
- n8n: http://localhost:5678
- InfluxDB: http://localhost:8086 (admin/FIAP@123)
- Grafana: http://localhost:3000 (admin/admin)

**Nota:** Para este app, você pode usar o broker MQTT local (plataforma IoT) ou configurar um Raspberry Pi como camada Fog intermediária.

### Sensores e Atuadores

**Sensores:**
- DHT22 (Temperatura e Umidade) - Pino GPIO 26
- MPU6050 (Acelerômetro/Giroscópio) - Pinos I2C:
  - SDA: GPIO 18
  - SCL: GPIO 19

**Atuadores:**
- LED - Pino GPIO 27 (Indicador de status do motor)

### Funcionamento

O sistema implementa arquitetura hierárquica Edge-Fog-Cloud, com processamento distribuído em múltiplas camadas:

**Tópico de Publicação (Dados):**
- `FIAPIoT/aula09/noris/motor/dados`
- Envia dados a cada 3 segundos em formato JSON
- Campos: temperatura, umidade, aceleração (x, y, z), status do motor

**Tópico de Subscrição (Comandos):**
- `FIAPIoT/aula09/noris/motor/cmd`
- Recebe comandos para ligar/desligar o motor

**Configuração MQTT:**
- Broker: Raspberry Pi (IP local configurável)
- Porta: 1883
- Client ID: IoTDeviceNoris001

**Arquitetura demonstrada:**
- **Edge Layer:** ESP32 com sensores fazendo pré-processamento local
- **Fog Layer (Near Edge):** Raspberry Pi executando broker MQTT, realizando agregação e filtragem de dados
- **Cloud Layer:** Servidores remotos para análise avançada e armazenamento de longo prazo

Esta arquitetura reduz latência, economiza largura de banda e permite operação parcial mesmo com conectividade intermitente à nuvem. O processamento próximo à origem dos dados (Edge) permite respostas rápidas para controle crítico.