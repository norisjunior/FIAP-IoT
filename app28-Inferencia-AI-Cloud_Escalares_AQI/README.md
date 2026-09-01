# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 20 - Sistema de Monitoramento de Qualidade do Ar (AQI) com Machine Learning

Esta aplicação demonstra um sistema completo de monitoramento de qualidade do ar (Air Quality Index - AQI) utilizando Machine Learning. Um ESP32-S3 com 12 sensores analógicos coleta dados de diferentes poluentes, publica via MQTT, e um modelo de Rede Neural classifica a qualidade do ar em tempo real, enviando alertas via Telegram.

## Estrutura do Projeto

```
app20-AQI/
├── App-Device-ESP32_AQI/              # Código do dispositivo ESP32
│   ├── src/
│   │   ├── iot-app20.ino             # Código principal ESP32
│   │   ├── ESP32Sensors.hpp          # Biblioteca de sensores genérica
│   │   └── ESP32SensorsAQI.hpp       # Biblioteca específica AQI
│   ├── diagram.json                  # Diagrama Wokwi
│   └── platformio.ini                # Config PlatformIO
│
├── App-Service-AI_Predict/            # Serviço de ML (FastAPI/Uvicorn)
│   ├── service_app.py                # API de predição
│   └── requirements.txt              # Dependências Python
│
├── App-Fluxo_n8n/                     # Workflow n8n base
│   └── Fluxo-n8n-predict.json        # Fluxo de predição e alerta
│
├── Fase1_Plataforma_AI-Predict_Manual/  # Fase 1: Execução manual
│   └── README-AQI_ML_Manual.md         # Instruções da Fase 1
│
├── Fase2_Plataforma_AI-Predict_Service/ # Fase 2: Stack Docker
│   ├── ML_AQI_service/                  # Docker Compose + serviços
│   └── README-AQI_ML_Service.md         # Instruções da Fase 2
│
├── Fase3_Plataforma_AI-LLM_Agente/     # Fase 3: Agente LLM
│   ├── Fluxos-n8n-LLM/                 # Workflows n8n com LLM
│   └── (README específico da Fase 3)
│
├── Fase4_Plataforma_AI-LLM-OpenClaw_MCP/ # Fase 4: MCP Server
│   ├── MCP_ML_AQI_Predict/               # Servidor MCP
│   └── README-MCP_ML_AQI_Predict.md      # Instruções da Fase 4
│
└── README.md                          # Este arquivo (visão geral)
```

## Pré-requisitos

### Inicializar a Plataforma IoT

Esta aplicação **requer obrigatoriamente** a plataforma IoT completa rodando. Siga as instruções em `IoT-platform/README.md`:

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

**Serviços utilizados nesta aplicação:**
- MQTT Broker: localhost:1883 (para ESP32: host.wokwi.internal:1883)
- n8n: http://localhost:5678 (orquestrador de fluxo)
- Node-RED: http://localhost:1880 (admin/FIAPIoT)

## Código do Dispositivo (App-Device-ESP32_AQI)

O código do ESP32-S3 está em `App-Device-ESP32_AQI/`. Utiliza 12 potenciômetros simulando sensores de qualidade do ar:

**Sensores de Material Particulado:**
- PM2.5 (Particulado fino) - Pino GPIO 4
- PM10 (Particulado inalável) - Pino GPIO 5

**Sensores de Óxidos de Nitrogênio:**
- NO (Monóxido de Nitrogênio) - Pino GPIO 6
- NO2 (Dióxido de Nitrogênio) - Pino GPIO 7
- NOx (Óxidos de Nitrogênio totais) - Pino GPIO 8

**Sensores de Gases:**
- NH3 (Amônia) - Pino GPIO 3
- CO (Monóxido de Carbono) - Pino GPIO 9
- SO2 (Dióxido de Enxofre) - Pino GPIO 1
- O3 (Ozônio) - Pino GPIO 2

**Sensores de COVs (Compostos Orgânicos Voláteis):**
- Benzene - Pino GPIO 10
- Toluene - Pino GPIO 11
- Xylene - Pino GPIO 12

**Configuração MQTT:**
- Client ID: IoTDeviceNorisAQI001
- Tópico publicação: `FIAPIoT/aqi/dados`
- Intervalo de coleta: 10 segundos

### Como Executar o ESP32

1. **Abrir o projeto no Wokwi:**
   - Arquivo: `App-Device-ESP32_AQI/src/iot-app20.ino`
   - Diagrama: `App-Device-ESP32_AQI/diagram.json`

2. **Compilar e executar:**
   - O ESP32 conectará ao broker MQTT
   - Coletará dados dos 12 sensores a cada 10 segundos
   - Publicará no tópico `FIAPIoT/aqi/dados`

3. **Ajustar valores dos sensores:**
   - Use os 12 potenciômetros (sliders) no Wokwi
   - Cada potenciômetro representa um poluente

4. **Monitorar no Serial Monitor:**
   ```
   [PARTICULADOS] - PM2.5: 45.23 μg/m³ | PM10: 82.11 μg/m³
   [ÓXIDOS NITROGÊNIO] - NO: 15.32 μg/m³ | NO2: 28.71 μg/m³ | NOx: 40.23 μg/m³
   [GASES] - NH3: 9.12 μg/m³ | CO: 0.71 mg/m³ | SO2: 6.43 μg/m³ | O3: 32.54 μg/m³
   [COVs] - Benzene: 2.11 μg/m³ | Toluene: 3.52 μg/m³ | Xylene: 1.23 μg/m³
   ```

## Serviço de ML (App-Service-AI_Predict)

O serviço de predição está em `App-Service-AI_Predict/`. É uma API FastAPI que recebe os dados dos sensores e retorna a classificação da qualidade do ar usando uma Rede Neural treinada com TensorFlow/Keras.

**Arquivos necessários (gerados no treinamento):**
- `modelo_aqi_nn.keras` - Modelo treinado
- `preprocess_aqi.pkl` - Scaler para normalização

**Classificação em 3 níveis:**
- **Aceitável**: Qualidade do ar boa
- **Ruim**: Qualidade do ar moderada
- **Perigoso**: Qualidade do ar crítica (alerta enviado ao Telegram)

**Exemplo de requisição:**
```bash
curl -X POST http://localhost:8000/predict \
  -H "Content-Type: application/json" \
  -d '{
    "PM2_5": 342, "PM10": 477, "NO": 10, "NO2": 51, "NOx": 40,
    "NH3": 42, "CO": 1.7, "SO2": 17, "O3": 91,
    "Benzene": 5.2, "Toluene": 29, "Xylene": 0.5
  }'
```

## Pipeline de Dados

```
┌──────────────┐
│  ESP32-S3    │
│ 12 Sensores  │ ──► PM2.5, PM10, NO, NO2, NOx, NH3, CO, SO2, O3, Benzene, Toluene, Xylene
└──────┬───────┘
       │ MQTT (10s)
       ▼
┌──────────────┐
│ MQTT Broker  │
└──────┬───────┘
       │
       ▼
┌──────────────┐
│     n8n      │ ──► Orquestração
└──────┬───────┘
       │
       ▼
┌──────────────┐
│  ML Service  │ ──► Rede Neural (Keras)
│   FastAPI    │     • modelo_aqi_nn.keras
│              │     • preprocess_aqi.pkl
└──────┬───────┘
       │
       ▼
┌──────────────┐
│   Telegram   │ ──► Alertas: Aceitável / Ruim / Perigoso
└──────────────┘
```

## Fases de Execução

O projeto está organizado em 4 fases evolutivas. Cada fase possui seu próprio README com instruções detalhadas no respectivo diretório:

| Fase | Diretório | Descrição |
|------|-----------|-----------|
| **Fase 1** | `Fase1_Plataforma_AI-Predict_Manual/` | Execução manual: serviço ML local em ambiente virtual Python, n8n via Docker |
| **Fase 2** | `Fase2_Plataforma_AI-Predict_Service/` | Stack Docker: MQTT + ML Service + n8n containerizados com Docker Compose |
| **Fase 3** | `Fase3_Plataforma_AI-LLM_Agente/` | Agente LLM: integração com Ollama e PostgreSQL para consultas em linguagem natural |
| **Fase 4** | `Fase4_Plataforma_AI-LLM-OpenClaw_MCP/` | MCP Server: servidor de ferramentas HTTP para integração com LLMs via OpenClaw |

## Troubleshooting

**ESP32 não conecta ao MQTT:**
- Verificar se broker está rodando: `docker ps | grep mqtt`
- Verificar configuração do host no código: `host.wokwi.internal`

**Modelo não classifica corretamente:**
- Verificar se arquivos .keras e .pkl estão presentes
- Verificar compatibilidade de versões TensorFlow/Keras
- Revisar normalização dos dados de entrada

**n8n não recebe dados:**
- Verificar tópico MQTT no workflow
- Verificar conectividade com broker
- Checar logs do n8n: `docker logs n8n`

**Alertas não chegam no Telegram:**
- Configurar bot do Telegram e chat ID no n8n
- Verificar se classificação está como "Perigoso"
- Testar manualmente o nó do Telegram no n8n