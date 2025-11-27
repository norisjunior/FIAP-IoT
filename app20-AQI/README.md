# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 20 - Sistema de Monitoramento de Qualidade do Ar (AQI) com Machine Learning

Esta aplicação demonstra um sistema completo de monitoramento de qualidade do ar (Air Quality Index - AQI) utilizando Machine Learning. Um ESP32-S3 com 12 sensores analógicos coleta dados de diferentes poluentes, publica via MQTT, e um modelo de Rede Neural classifica a qualidade do ar em tempo real, enviando alertas via Telegram.

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

## Sensores e Atuadores

Esta aplicação utiliza um **ESP32-S3** com 12 potenciômetros simulando sensores de qualidade do ar:

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

## Funcionamento

### Pipeline de Dados

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
│ Flask/FastAPI│     • modelo_aqi_nn.keras
│              │     • preprocess_aqi.pkl
└──────┬───────┘
       │
       ▼
┌──────────────┐
│   Telegram   │ ──► Alertas: Aceitável / Ruim / Perigoso
└──────────────┘
```

**Modelo de Machine Learning:**
- Algoritmo: Rede Neural (Keras)
- Pré-processamento: Normalização via StandardScaler
- Saída: Classificação em 3 níveis
  - **Aceitável**: Qualidade do ar boa
  - **Ruim**: Qualidade do ar moderada
  - **Perigoso**: Qualidade do ar crítica (alerta enviado ao Telegram)

**Dados Coletados (JSON):**
```json
{
  "device": "IoTDeviceNorisAQI001",
  "PM25": 45.2,
  "PM10": 82.1,
  "NO": 15.3,
  "NO2": 28.7,
  "NOx": 40.2,
  "NH3": 9.1,
  "CO": 0.7,
  "SO2": 6.4,
  "O3": 32.5,
  "Benzene": 2.1,
  "Toluene": 3.5,
  "Xylene": 1.2
}
```

## Modos de Execução

Esta aplicação oferece **duas formas de execução**:

### 1. Modo Manual (README-AQIManual.md)

Execução tradicional com serviços rodando localmente:
- Plataforma IoT via Docker
- Serviço Flask/Uvicorn rodando em ambiente virtual Python
- n8n consumindo API local via `http://host.docker.internal:8000`

**Use este modo quando:**
- Quiser debugar o modelo de ML
- Precisar modificar o código Python do serviço
- Quiser visualizar logs detalhados do Flask

**[Consulte o README-AQIManual.md para instruções detalhadas](README-AQIManual.md)**

### 2. Modo Stack Docker (README-AQIStack.md)

Execução completa via Docker Compose:
- MQTT Broker (Mosquitto)
- ML Service (containerizado)
- n8n (orquestrador)
- Tudo sobe com um único comando

**Use este modo quando:**
- Quiser deploy rápido e reproduzível
- Precisar de ambiente isolado e portável
- Quiser simular ambiente de produção

**[Consulte o README-AQIStack.md para instruções detalhadas](README-AQIStack.md)**

## Como Executar o ESP32

**Independente do modo escolhido (Manual ou Stack)**, o ESP32 deve ser executado da mesma forma:

1. **Abrir o projeto no Wokwi:**
   - Arquivo: `app20-AQI/src/iot-app20.ino`
   - Diagrama: `app20-AQI/diagram.json`

2. **Compilar e executar:**
   - O ESP32 conectará ao broker MQTT
   - Coletará dados dos 12 sensores a cada 10 segundos
   - Publicará no tópico `FIAPIoT/aqi/dados`

3. **Ajustar valores dos sensores:**
   - Use os 12 potenciômetros (sliders) no Wokwi
   - Cada potenciômetro representa um poluente
   - Ajuste para simular diferentes condições de qualidade do ar

4. **Monitorar no Serial Monitor:**
   ```
   ========================================
   DADOS AQI COLETADOS (Far Edge/Extreme Edge):
   ========================================
   [PARTICULADOS] - PM2.5: 45.23 μg/m³ | PM10: 82.11 μg/m³
   [ÓXIDOS NITROGÊNIO] - NO: 15.32 μg/m³ | NO2: 28.71 μg/m³ | NOx: 40.23 μg/m³
   [GASES] - NH3: 9.12 μg/m³ | CO: 0.71 mg/m³ | SO2: 6.43 μg/m³ | O3: 32.54 μg/m³
   [COVs] - Benzene: 2.11 μg/m³ | Toluene: 3.52 μg/m³ | Xylene: 1.23 μg/m³
   ```

## Estrutura do Projeto

```
app20-AQI/
├── src/
│   ├── iot-app20.ino              # Código ESP32
│   ├── ESP32Sensors.hpp           # Biblioteca de sensores genérica
│   └── ESP32SensorsAQI.hpp        # Biblioteca específica AQI
├── AQI_ML_app/                    # Modo Manual
│   ├── service_app.py             # API Flask/FastAPI
│   ├── requirements.txt           # Dependências Python
│   └── model/
│       ├── modelo_aqi_nn.keras    # Modelo treinado
│       └── preprocess_aqi.pkl     # Scaler de normalização
├── AQI_stack/                     # Modo Stack Docker
│   ├── docker-compose.yml         # Orquestração de containers
│   ├── mosquitto/                 # Config MQTT Broker
│   ├── ml-service/                # Container do modelo ML
│   └── n8n/                       # Config n8n
├── Fluxo-n8n-manual.json          # Workflow n8n (modo manual)
├── diagram.json                   # Diagrama Wokwi
├── platformio.ini                 # Config PlatformIO
├── README.md                      # Este arquivo (visão geral)
├── README-AQIManual.md            # Instruções modo manual
└── README-AQIStack.md             # Instruções modo stack
```

## Casos de Uso

**1. Monitoramento Ambiental Urbano:**
- Estações de monitoramento de qualidade do ar em cidades
- Alertas automáticos quando poluição atinge níveis críticos
- Visualização de tendências e padrões

**2. Controle Industrial:**
- Monitoramento de emissões em fábricas
- Conformidade com regulamentações ambientais
- Prevenção de exposição a poluentes perigosos

**3. Ambientes Internos (Indoor):**
- Monitoramento de qualidade do ar em escritórios e escolas
- Controle de ventilação baseado em dados reais
- Alerta de presença de COVs perigosos

**4. Smart Cities:**
- Rede de sensores distribuídos pela cidade
- Dashboard público de qualidade do ar
- Integração com sistemas de mobilidade urbana

## Próximos Passos

1. **Treinamento do Modelo**: Treinar a rede neural com dataset de AQI real
2. **Escolher Modo de Execução**: Manual (debugging) ou Stack (produção)
3. **Configurar ESP32**: Executar código no Wokwi
4. **Configurar n8n**: Importar workflow e conectar ao Telegram
5. **Testar Pipeline**: Ajustar sensores e verificar classificações
6. **Monitorar Alertas**: Validar recebimento de notificações Telegram

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
