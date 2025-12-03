# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## App 20 - Modo Manual: Serviço de ML para Índice de Qualidade do Ar (AQI)

Este documento descreve a execução **manual** da aplicação AQI, onde o serviço de Machine Learning roda localmente em ambiente virtual Python, e o n8n (rodando em Docker) consome a API via `http://host.docker.internal:8000`.

## Pré-requisitos

Antes de executar este experimento, certifique-se de:

1. **Plataforma IoT inicializada** (veja README.md principal)
2. **ESP32 compilado e pronto** (veja README.md principal)
3. **Python 3.8+** instalado no sistema
4. **Modelo treinado** disponível:
   - `modelo_aqi_nn.keras`
   - `preprocess_aqi.pkl`

## Arquitetura do Modo Manual

```
┌──────────────┐
│   ESP32-S3   │  ──► Publica dados via MQTT
└──────┬───────┘
       │
       ▼
┌──────────────┐
│ MQTT Broker  │  (Docker - Plataforma IoT)
│ mqtt-broker  │
└──────┬───────┘
       │
       ▼
┌──────────────┐
│     n8n      │  (Docker - Plataforma IoT)
│              │  ──► http://host.docker.internal:8000/predict
└──────┬───────┘
       │
       ▼
┌──────────────┐
│ Flask/Uvicorn│  (Local - Ambiente Virtual Python)
│ ML Service   │  localhost:8000
│              │  • modelo_aqi_nn.keras
│              │  • preprocess_aqi.pkl
└──────┬───────┘
       │
       ▼
┌──────────────┐
│   Telegram   │  ──► Alertas críticos
└──────────────┘
```

## Passo a Passo

### 1. Treinar o Modelo de Machine Learning

> **Importante:** O treinamento pode ser feito no Google Colab apresentado em sala de aula.

Execute o pipeline de treinamento que:
- Normaliza os dados usando StandardScaler
- Treina uma Rede Neural para classificação de AQI
- Salva dois arquivos:
  - `modelo_aqi_nn.keras` - Modelo treinado
  - `preprocess_aqi.pkl` - Scaler para normalização

**Certifique-se de que ambos os arquivos estão em:**
```
app20-AQI/AQI_ML_app/model/
```

### 2. Iniciar a Plataforma IoT

Se ainda não iniciou, execute:

```bash
# Acessar WSL2 Ubuntu
wsl -d ubuntu

# Navegar para o diretório da plataforma
cd ~/FIAP-IoT/IoT-platform/

# Iniciar todos os serviços
sudo ./start-linux.sh
```

Verifique se os serviços estão rodando:
```bash
docker ps
```

Você deve ver containers:
- `mqtt-broker` (porta 1883)
- `n8n` (porta 5678)
- Outros serviços da plataforma

### 3. Configurar Ambiente Python Local

Navegue até o diretório da aplicação ML:

```bash
cd app20-AQI/AQI_ML_app
```

**Criar ambiente virtual:**

```bash
# Windows
python -m venv venv

# Linux/Mac
python3 -m venv venv
```

**Ativar ambiente virtual:**

```bash
# Windows PowerShell
.\venv\Scripts\Activate.ps1

# Windows CMD
.\venv\Scripts\activate.bat

# Linux/Mac
source venv/bin/activate
```

**Atualizar pip:**

```bash
python -m pip install --upgrade pip
```

**Instalar dependências:**

```bash
pip install -r requirements.txt
```

Dependências típicas:
- `fastapi` ou `flask`
- `uvicorn`
- `tensorflow` ou `keras`
- `scikit-learn`
- `numpy`
- `pydantic`

### 4. Iniciar o Serviço de ML (Flask/Uvicorn)

Com o ambiente virtual ativado:

```bash
uvicorn service_app:app --reload --port 8000
```

**Saída esperada:**

```
INFO:     Uvicorn running on http://127.0.0.1:8000 (Press CTRL+C to quit)
INFO:     Started reloader process [xxxxx] using StatReload
INFO:     Started server process [xxxxx]
INFO:     Waiting for application startup.
INFO:     Application startup complete.
```

O serviço estará disponível em:
- **Localhost:** `http://localhost:8000`
- **Docker (via host.docker.internal):** `http://host.docker.internal:8000`

### 5. Testar a API Manualmente

Antes de configurar o n8n, teste se a API está funcionando:

**Exemplo de requisição (curl):**

```bash
curl -X POST http://localhost:8000/predict \
  -H "Content-Type: application/json" \
  -d '{
    "PM2_5": 342,
    "PM10": 477,
    "NO": 10,
    "NO2": 51,
    "NOx": 40,
    "NH3": 42,
    "CO": 1.7,
    "SO2": 17,
    "O3": 91,
    "Benzene": 5.2,
    "Toluene": 29,
    "Xylene": 0.5
  }'
```

**Saída esperada:**

```json
{
  "class": "Perigoso",
  "probabilities": {
    "Aceitável": 3.5151686006429372e-06,
    "Perigoso": 0.8978066444396973,
    "Ruim": 0.102189801633358
  }
}
```

Se receber esta resposta, a API está funcionando corretamente!

### 6. Configurar o n8n

1. **Acessar n8n:**
   ```
   http://localhost:5678
   ```

2. **Importar workflow:**
   - Menu > Import from File
   - Selecionar: `app20-AQI/Fluxo-n8n-manual.json`
   - Clicar em "Import"

3. **Configurar nó MQTT Trigger:**
   - **Host:** `mqtt-broker`
   - **Port:** `1883`
   - **Topics:** `FIAPIoT/aqi/dados`
   - **Client ID:** `n8n-aqi-subscriber`

4. **Configurar nó HTTP Request:**
   - **Method:** POST
   - **URL:** `http://host.docker.internal:8000/predict`
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

5. **Configurar nó Switch (Classificação):**
   - **Route 1 (Aceitável):** `{{ $json["class"] === "Aceitável" }}`
   - **Route 2 (Ruim):** `{{ $json["class"] === "Ruim" }}`
   - **Route 3 (Perigoso):** `{{ $json["class"] === "Perigoso" }}`

6. **Configurar nó Telegram:**
   - **Bot Token:** Criar bot via [@BotFather](https://t.me/botfather)
   - **Chat ID:** Obter via [@userinfobot](https://t.me/userinfobot)
   - **Mensagem para Aceitável/Ruim:**
     ```
     ℹ️ INFO - Qualidade do Ar: {{ $json["class"] }}
     📊 Probabilidades:
     • Aceitável: {{ ($json["probabilities"]["Aceitável"] * 100).toFixed(1) }}%
     • Ruim: {{ ($json["probabilities"]["Ruim"] * 100).toFixed(1) }}%
     • Perigoso: {{ ($json["probabilities"]["Perigoso"] * 100).toFixed(1) }}%
     ```
   - **Mensagem para Perigoso:**
     ```
     🚨 CRITICAL - Qualidade do Ar PERIGOSA!
     ⚠️ Nível: {{ $json["class"] }}
     📈 Confiança: {{ ($json["probabilities"]["Perigoso"] * 100).toFixed(1) }}%

     Tome medidas imediatas!
     ```

7. **Ativar workflow:**
   - Clique no toggle "Active" no canto superior direito

### 7. Executar o ESP32

1. **Abrir Wokwi:**
   - `app20-AQI/`

2. **Compilar e executar:**
   - O ESP32 começará a publicar dados a cada 10 segundos

3. **Ajustar sensores:**
   - Use os sliders dos 12 potenciômetros
   - Simule condições de "Perigoso" aumentando os valores

### 8. Monitorar o Fluxo Completo

**No Serial Monitor do ESP32:**
```
SUCESSO: Dados AQI enviados para o MQTT Broker
```

**No terminal do Flask/Uvicorn:**
```
INFO:     127.0.0.1:xxxxx - "POST /predict HTTP/1.1" 200 OK
```

**No n8n (aba Executions):**
- Verifique execuções bem-sucedidas
- Veja dados fluindo pelo workflow
- Confirme envio ao Telegram

**No Telegram:**
- Receba alertas conforme classificação
- INFO para Aceitável/Ruim
- CRITICAL para Perigoso

## Entendimento do Ciclo

```
ESP32 → MQTT Broker (Docker)
         ↓
       n8n (Docker)
         ↓
   Flask API (Local) ──► Modelo ML classifica
         ↓
     Telegram ──► Alerta enviado
```

## Vantagens do Modo Manual

✅ **Debugging facilitado:** Logs detalhados do Flask no terminal
✅ **Desenvolvimento ágil:** Modificar código Python sem rebuild de container
✅ **Teste de modelos:** Trocar modelos .keras rapidamente
✅ **Visualização de inferências:** Ver cada predição no console

## Desvantagens do Modo Manual

❌ **Dependência de ambiente local:** Requer Python instalado
❌ **Não portável:** Difícil compartilhar configuração exata
❌ **Processo manual:** Precisa iniciar Flask manualmente

## Próximo Passo: Modo Stack

Para uma solução mais robusta e reproduzível, veja **[README-AQIStack.md](README-AQIStack.md)**, que empacota tudo em containers Docker.

## Troubleshooting

**Flask não inicia:**
- Verificar se porta 8000 está livre: `netstat -an | findstr 8000`
- Verificar se ambiente virtual está ativado
- Verificar se dependências foram instaladas

**n8n não conecta à API:**
- Verificar URL: `http://host.docker.internal:8000/predict`
- Testar manualmente com curl primeiro
- Verificar firewall do Windows

**Modelo não carrega:**
- Verificar se arquivos .keras e .pkl existem em `AQI_ML_app/model/`
- Verificar compatibilidade de versões TensorFlow
- Checar logs do Flask para erros de carregamento

**Telegram não envia:**
- Verificar token do bot no n8n
- Verificar chat ID correto
- Testar nó Telegram manualmente no n8n
