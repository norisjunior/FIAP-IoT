# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## App 20 - Modo Stack Docker: Sistema AQI Containerizado

Este documento descreve a execução via **Docker Compose Stack**, onde toda a infraestrutura (MQTT Broker, ML Service e n8n) é orquestrada em containers, permitindo deploy rápido e reproduzível.

## Pré-requisitos

Antes de executar este experimento, certifique-se de:

1. **Docker e Docker Compose instalados**
2. **ESP32 compilado e pronto** (veja README.md principal)
3. **Modelo treinado** disponível:
   - `modelo_aqi_nn.keras`
   - `preprocess_aqi.pkl`
4. **Arquivos do modelo copiados para:**
   ```
   app20-AQI/AQI_stack/ml-service/model/
   ```

## Arquitetura do Modo Stack

```
┌──────────────┐
│   ESP32-S3   │  ──► Publica dados via MQTT
└──────┬───────┘
       │
       ▼
┌──────────────┐
│  Mosquitto   │  (Container Docker)
│ MQTT Broker  │  aqi-mosquitto:1883
└──────┬───────┘
       │
       ▼
┌──────────────┐
│     n8n      │  (Container Docker)
│              │  ──► http://aqi-ml-service:8000/predict
└──────┬───────┘
       │
       ▼
┌──────────────┐
│  ML Service  │  (Container Docker)
│ Flask/FastAPI│  aqi-ml-service:8000
│              │  • modelo_aqi_nn.keras
│              │  • preprocess_aqi.pkl
└──────┬───────┘
       │
       ▼
┌──────────────┐
│   Telegram   │  ──► Alertas críticos
└──────────────┘
```

**Vantagens da Stack:**
- ✅ Deploy com um único comando
- ✅ Isolamento completo de ambiente
- ✅ Portável e reproduzível
- ✅ Simula ambiente de produção
- ✅ Networking automático entre containers

## Estrutura da Stack

```
app20-AQI/AQI_stack/
├── docker-compose.yml         # Orquestração dos serviços
├── mosquitto/
│   └── mosquitto.conf         # Configuração MQTT Broker
├── ml-service/
│   ├── Dockerfile             # Container do modelo ML
│   ├── requirements.txt       # Dependências Python
│   ├── service_app.py         # API Flask/FastAPI
│   └── model/
│       ├── modelo_aqi_nn.keras   # Modelo treinado (COPIAR AQUI)
│       └── preprocess_aqi.pkl    # Scaler (COPIAR AQUI)
└── n8n/
    └── (dados persistentes)
```

## Passo a Passo

### 1. Preparar Arquivos do Modelo

Certifique-se de que os arquivos do modelo treinado estão no diretório correto:

```bash
# Navegar até o diretório da stack
cd app20-AQI/AQI_stack/

# Verificar se os arquivos existem
ls ml-service/model/
```

Você deve ver:
- `modelo_aqi_nn.keras`
- `preprocess_aqi.pkl`

Se os arquivos não estiverem lá, copie do diretório de treinamento:

```bash
# Windows
copy ..\AQI_ML_app\model\*.* ml-service\model\

# Linux/Mac
cp ../AQI_ML_app/model/* ml-service/model/
```

### 2. Revisar o docker-compose.yml

O arquivo `docker-compose.yml` deve conter:

```yaml
version: '3.8'

services:
  # MQTT Broker (Mosquitto)
  aqi-mosquitto:
    image: eclipse-mosquitto:latest
    container_name: aqi-mosquitto
    ports:
      - "1883:1883"
      - "9001:9001"
    volumes:
      - ./mosquitto/mosquitto.conf:/mosquitto/config/mosquitto.conf
    networks:
      - aqi-network
    restart: unless-stopped

  # ML Service (Flask/FastAPI)
  aqi-ml-service:
    build:
      context: ./ml-service
      dockerfile: Dockerfile
    container_name: aqi-ml-service
    ports:
      - "8000:8000"
    volumes:
      - ./ml-service/model:/app/model
    environment:
      - MODEL_PATH=/app/model/modelo_aqi_nn.keras
      - SCALER_PATH=/app/model/preprocess_aqi.pkl
    networks:
      - aqi-network
    restart: unless-stopped
    depends_on:
      - aqi-mosquitto

  # n8n (Workflow Automation)
  aqi-n8n:
    image: n8nio/n8n:latest
    container_name: aqi-n8n
    ports:
      - "5678:5678"
    volumes:
      - aqi-n8n-data:/home/node/.n8n
    environment:
      - N8N_BASIC_AUTH_ACTIVE=false
      - WEBHOOK_URL=http://localhost:5678/
    networks:
      - aqi-network
    restart: unless-stopped
    depends_on:
      - aqi-mosquitto
      - aqi-ml-service

networks:
  aqi-network:
    driver: bridge

volumes:
  aqi-n8n-data:
```

### 3. Revisar o Dockerfile do ML Service

O arquivo `ml-service/Dockerfile` deve conter:

```dockerfile
FROM python:3.10-slim

WORKDIR /app

# Instalar dependências do sistema (se necessário)
RUN apt-get update && apt-get install -y \
    gcc \
    && rm -rf /var/lib/apt/lists/*

# Copiar requirements
COPY requirements.txt .

# Instalar dependências Python
RUN pip install --no-cache-dir --upgrade pip && \
    pip install --no-cache-dir -r requirements.txt

# Copiar código da aplicação
COPY service_app.py .

# Criar diretório para o modelo
RUN mkdir -p /app/model

# Expor porta
EXPOSE 8000

# Comando para iniciar o serviço
CMD ["uvicorn", "service_app:app", "--host", "0.0.0.0", "--port", "8000"]
```

### 4. Configurar Mosquitto

O arquivo `mosquitto/mosquitto.conf` deve conter:

```conf
# Configuração básica do Mosquitto
listener 1883
allow_anonymous true

# Log
log_dest stdout
log_type all

# Persistence
persistence true
persistence_location /mosquitto/data/
```

### 5. Subir a Stack

Com tudo configurado, execute:

```bash
# Navegar até o diretório da stack
cd app20-AQI/AQI_stack/

# Subir todos os containers
docker compose up -d --build
```

**Flags explicadas:**
- `-d`: Detached mode (roda em background)
- `--build`: Força rebuild das imagens (útil após mudanças)

**Saída esperada:**

```
[+] Building 45.2s (12/12) FINISHED
[+] Running 4/4
 ✔ Network aqi-stack_aqi-network     Created
 ✔ Container aqi-mosquitto           Started
 ✔ Container aqi-ml-service          Started
 ✔ Container aqi-n8n                 Started
```

### 6. Verificar Serviços

Verificar se todos os containers estão rodando:

```bash
docker ps
```

Você deve ver 3 containers:
- `aqi-mosquitto` (porta 1883)
- `aqi-ml-service` (porta 8000)
- `aqi-n8n` (porta 5678)

Verificar logs dos serviços:

```bash
# Logs do ML Service
docker logs aqi-ml-service

# Logs do Mosquitto
docker logs aqi-mosquitto

# Logs do n8n
docker logs aqi-n8n
```

### 7. Testar o ML Service

Teste se a API está respondendo:

```bash
curl -X POST http://localhost:8000/predict \
  -H "Content-Type: application/json" \
  -d '{
    "PM2_5": 45,
    "PM10": 82,
    "NO": 15,
    "NO2": 28,
    "NOx": 40,
    "NH3": 9,
    "CO": 0.7,
    "SO2": 6,
    "O3": 32,
    "Benzene": 2.1,
    "Toluene": 3.5,
    "Xylene": 1.2
  }'
```

**Retorno esperado:**

```json
{
  "class": "Aceitável",
  "probabilities": {
    "Aceitável": 0.971751868724823,
    "Perigoso": 8.366844122065231e-05,
    "Ruim": 0.028164511546492577
  }
}
```

### 8. Configurar o n8n

1. **Acessar n8n:**
   ```
   http://localhost:5678
   ```

2. **Criar novo workflow** ou importar workflow existente

3. **Configurar nó MQTT Trigger:**
   - **Host:** `aqi-mosquitto` (nome do container)
   - **Port:** `1883`
   - **Topics:** `FIAPIoT/aqi/dados`
   - **Client ID:** `n8n-aqi-stack-subscriber`

4. **Configurar nó HTTP Request:**
   - **Method:** POST
   - **URL:** `http://aqi-ml-service:8000/predict` (nome do container)
   - **Body Content Type:** JSON
   - **Body:**
     ```json
     {
       "PM2_5": {{$json["PM25"]}},
       "PM10": {{$json["PM10"]}},
       "NO": {{$json["NO"]}},
       "NO2": {{$json["NO2"]}},
       "NOx": {{$json["NOx"]}},
       "NH3": {{$json["NH3"]}},
       "CO": {{$json["CO"]}},
       "SO2": {{$json["SO2"]}},
       "O3": {{$json["O3"]}},
       "Benzene": {{$json["Benzene"]}},
       "Toluene": {{$json["Toluene"]}},
       "Xylene": {{$json["Xylene"]}}
     }
     ```

5. **Configurar lógica de roteamento e Telegram** (igual ao modo manual)

6. **Ativar workflow**

### 9. Configurar ESP32 para a Stack

**Importante:** O ESP32 precisa se conectar ao broker Mosquitto da stack.

**Opção 1: Wokwi (recomendado para desenvolvimento)**

No código `iot-app20.ino`, o ESP32 já está configurado para:
```cpp
#define MQTT_SERVER "host.wokwi.internal"
```

Isso funcionará se a stack estiver rodando na mesma máquina.

**Opção 2: Hardware real**

Se estiver usando ESP32 físico, configure:
```cpp
#define MQTT_SERVER "192.168.x.x"  // IP da máquina rodando Docker
```

### 10. Executar o ESP32

1. **Abrir Wokwi:**
   - Arquivo: `app20-AQI/src/iot-app20.ino`

2. **Compilar e executar:**
   - O ESP32 conectará ao broker Mosquitto da stack
   - Publicará dados a cada 10 segundos

3. **Monitorar Serial Monitor:**
   ```
   Conectando ao MQTT Broker--> Conectado ao MQTT Broker!
   SUCESSO: Dados AQI enviados para o MQTT Broker
   ```

### 11. Monitorar o Fluxo Completo

**Logs do ML Service:**
```bash
docker logs -f aqi-ml-service
```

Você verá requisições POST sendo processadas.

**Logs do Mosquitto:**
```bash
docker logs -f aqi-mosquitto
```

Você verá mensagens MQTT sendo publicadas.

**n8n Executions:**
- Acessar: http://localhost:5678
- Aba "Executions"
- Ver workflow sendo executado a cada mensagem

**Telegram:**
- Receber alertas conforme classificação

## Comandos Úteis

**Parar a stack:**
```bash
docker compose down
```

**Parar e remover volumes:**
```bash
docker compose down -v
```

**Rebuild completo:**
```bash
docker compose down
docker compose up -d --build
```

**Ver logs em tempo real:**
```bash
docker compose logs -f
```

**Acessar shell do ML Service:**
```bash
docker exec -it aqi-ml-service bash
```

**Reiniciar apenas um serviço:**
```bash
docker compose restart aqi-ml-service
```

## Fluxo de Dados

```
ESP32 (Wokwi)
    ↓ MQTT
aqi-mosquitto:1883
    ↓ MQTT Subscribe
aqi-n8n:5678
    ↓ HTTP POST
aqi-ml-service:8000/predict
    ↓ Classificação
aqi-n8n:5678
    ↓ Alerta
Telegram
```

## Vantagens do Modo Stack

✅ **Deploy rápido:** Um comando sobe tudo
✅ **Isolamento:** Ambiente reproduzível e portável
✅ **Networking automático:** Containers se comunicam por nomes
✅ **Produção-ready:** Simula ambiente real
✅ **Escalável:** Fácil adicionar mais serviços

## Desvantagens do Modo Stack

❌ **Debugging mais complexo:** Logs dentro de containers
❌ **Rebuild necessário:** Mudanças no código requerem rebuild
❌ **Overhead:** Mais recursos consumidos (CPU/RAM)

## Diferenças entre Modo Manual e Stack

| Aspecto | Manual | Stack Docker |
|---------|--------|--------------|
| **Setup** | Múltiplos passos | Um comando |
| **Debugging** | Fácil (logs diretos) | Médio (docker logs) |
| **Portabilidade** | Baixa | Alta |
| **Produção** | Não recomendado | Recomendado |
| **Desenvolvimento** | Ágil | Rebuild necessário |
| **Isolamento** | Baixo | Alto |
| **Networking** | host.docker.internal | Nomes de container |

## Troubleshooting

**Containers não sobem:**
- Verificar se portas 1883, 5678, 8000 estão livres
- Verificar logs: `docker compose logs`
- Verificar Docker está rodando

**ML Service falha ao iniciar:**
- Verificar se arquivos do modelo existem em `ml-service/model/`
- Verificar logs: `docker logs aqi-ml-service`
- Verificar requirements.txt possui todas dependências

**n8n não conecta ao MQTT:**
- Usar nome do container: `aqi-mosquitto` (não `localhost`)
- Verificar se containers estão na mesma network: `docker network inspect aqi-stack_aqi-network`

**n8n não conecta ao ML Service:**
- Usar URL: `http://aqi-ml-service:8000/predict` (não `localhost`)
- Testar conectividade: `docker exec -it aqi-n8n ping aqi-ml-service`

**ESP32 não conecta ao Mosquitto:**
- Verificar se porta 1883 está exposta: `docker ps`
- Em hardware real, usar IP da máquina (não `localhost`)
- Verificar firewall

