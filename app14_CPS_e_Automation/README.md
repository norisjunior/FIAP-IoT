# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 14 - CPS (Cyber-Physical Systems) e Automação com Low-Code/No-Code

Esta aplicação demonstra a integração de sistemas ciber-físicos (CPS) utilizando ESP32, MQTT e plataformas de automação low-code/no-code (Node-RED e n8n). O sistema coleta dados de múltiplos sensores e permite automação visual de workflows, dashboards e notificações.

### Estrutura do Projeto

```
app14_CPS_e_Automation/
├── app14_NodeRED/                    # Implementação com Node-RED
│   ├── src/                          # Código ESP32
│   │   └── iot-aula08-app14.ino
│   ├── MQTT_local/                   # Configuração MQTT Broker
│   │   └── instalar_mqtt.md
│   └── NodeRED/                      # Configurações Node-RED
│       ├── dashboard_configs/        # Dashboard e flows
│       │   ├── dashboard.json
│       │   └── NodeRED-Info.md
│       └── docker_configs/           # Docker setup
│           ├── docker-compose.yml
│           ├── Dockerfile
│           └── instalar_docker.md
│
└── app14-n8n/                        # Implementação com n8n
    ├── n8n/                          # Workflows n8n
    │   ├── fluxo_mqtt.json
    │   ├── Config-Nó-*.json
    │   └── container-docker-local/
    └── NodeRED/                      # Integração Node-RED + Alexa
        └── diagrama_node-red_alexa.json
```

### Sensores e Atuadores (ESP32)

**Sensores:**
- DHT22 (Temperatura e Umidade) - Pino GPIO 26
- HC-SR04 (Sensor Ultrassônico de Distância) - Pinos:
  - TRIG: GPIO 17
  - ECHO: GPIO 16
- PIR (Sensor de Movimento/Presença) - Pino GPIO 25

**Atuadores:**
- LED Vermelho - Pino GPIO 27 (Indicador de status)

### Funcionamento

O ESP32 coleta dados de sensores ambientais e de presença, publicando via MQTT a cada 2.5 segundos:

**Tópico MQTT:** `FIAPIoT/sala1`

**Dados JSON publicados:**
```json
{
  "temp": 25.3,
  "umid": 60.5,
  "ic": 26.1,
  "dist": 45.2,
  "mov": true
}
```

## Opção 1: Automação com Node-RED

### Pré-requisitos

1. **Docker e Docker Compose instalados**
2. **MQTT Broker (Mosquitto)**

### Configuração do MQTT Broker

Siga as instruções em `app14_NodeRED/MQTT_local/instalar_mqtt.md`:

```bash
mkdir -p ~/mqtt-broker
cd ~/mqtt-broker
# Criar docker-compose.yml e mosquitto.conf conforme documentação
sudo docker compose up -d
```

**Configurações de conexão:**
- **ESP32:** host.wokwi.internal:1883
- **Node-RED:** host.docker.internal:1883

### Configuração do Node-RED

1. **Instalar Node-RED via Docker:**

   Consulte `app14_NodeRED/NodeRED/docker_configs/instalar_docker.md`

   ```bash
   cd app14_NodeRED/NodeRED/docker_configs
   docker compose up -d
   ```

2. **Importar Dashboard:**

   - Acesse Node-RED: http://localhost:1880
   - Menu > Import > Selecione `dashboard_configs/dashboard.json`

3. **Configurar nós MQTT:**

   Consulte `dashboard_configs/NodeRED-Info.md` para configuração dos nós:
   - **mqtt in**: Server EMQx, Tópico FIAPIoT/sala1, QoS 1
   - **Function nodes**: Separação de dados e alertas

### Funcionalidades Node-RED

- **Dashboard em tempo real** com gráficos de temperatura, umidade, distância
- **Alertas via Telegram** para:
  - Detecção de presença (sensor PIR)
  - Objeto próximo (distância < limiar)
- **Processamento visual** de dados com function nodes
- **Histórico** de medições

## Opção 2: Automação com n8n

### Configuração do n8n

1. **Executar n8n via Docker:**

   ```bash
   cd app14-n8n/n8n/container-docker-local
   docker compose up -d
   ```

2. **Importar workflows:**

   - Acesse n8n: http://localhost:5678
   - Importe `fluxo_mqtt.json`
   - Configure os nós:
     - `Config-Nó-Code.json`
     - `Config-Nó-Telegram-Dist.json`
     - `Config-Nó-Telegram-IC-Alto.json`

### Funcionalidades n8n

- **Automação visual** de workflows
- **Integração MQTT** com processamento de eventos
- **Notificações Telegram** configuráveis
- **Lógica condicional** para alertas baseados em:
  - Índice de calor alto
  - Distância crítica
  - Detecção de movimento

## Integração com Alexa (Opcional)

O diretório `app14-n8n/NodeRED/` contém configuração para integração com Alexa:
- Importar `diagrama_node-red_alexa.json` no Node-RED
- Permite controle por voz e consultas de status

## Como Usar

### 1. Iniciar Infraestrutura

```bash
# MQTT Broker
cd ~/mqtt-broker
sudo docker compose up -d

# Node-RED (escolha uma opção)
cd app14_NodeRED/NodeRED/docker_configs
docker compose up -d

# OU n8n
cd app14-n8n/n8n/container-docker-local
docker compose up -d
```

### 2. Executar ESP32

- Compile e execute `app14_NodeRED/src/iot-aula08-app14.ino`
- Verifique conexão WiFi e MQTT no monitor serial

### 3. Acessar Interfaces

- **Node-RED:** http://localhost:1880
- **Node-RED Dashboard:** http://localhost:1880/ui
- **n8n:** http://localhost:5678

### 4. Monitorar e Automatizar

- Visualize dados em tempo real no dashboard
- Configure alertas e automações conforme necessidade
- Crie novos workflows arrastando e soltando nós

## Casos de Uso

- **Automação predial**: Monitoramento de salas, controle de presença
- **Smart home**: Integração com assistentes de voz (Alexa)
- **Alertas inteligentes**: Notificações baseadas em condições dos sensores
- **Análise de dados**: Coleta e visualização histórica
- **Prototipagem rápida**: Desenvolvimento ágil com low-code/no-code

## Vantagens do CPS com Low-Code

- **Desenvolvimento visual** sem programação extensiva
- **Integração rápida** de dispositivos e serviços
- **Flexibilidade** para modificar lógica em tempo real
- **Escalabilidade** através de containers Docker
- **Reutilização** de componentes e workflows
