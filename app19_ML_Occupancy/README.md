# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 19 - Machine Learning para Detecção de Ocupação em Tempo Real

Esta aplicação demonstra um sistema completo de Machine Learning aplicado a IoT para detecção de ocupação de ambientes. Um ESP32 com múltiplos sensores coleta dados ambientais continuamente, publicando via MQTT. Os dados são armazenados no InfluxDB e processados em tempo real por modelos de ML treinados (XGBoost), oferecendo interfaces console e web para visualização das predições.

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
- Node-RED: http://localhost:1880 (admin/FIAPIoT)
- InfluxDB: http://localhost:8086 (admin/FIAP@123)
  - Organização: fiapiot
  - Bucket: sensores
  - Token: TOKEN_SUPER_SECRETO_1234567890
- Grafana: http://localhost:3000 (admin/admin)

## Sensores e Atuadores

**Sensores:**
- DHT22 (Temperatura e Umidade) - Pino GPIO 26
- LDR (Sensor de Luminosidade) - Pino GPIO 35
- Potenciômetro (Simulador de CO2) - Pino GPIO 34

**Atuadores:**
- LED Vermelho - Pino GPIO 21 (Indicador de status durante envio de dados)

**Configuração MQTT:**
- Client ID: FIAP_IoT_app19_001
- Tópico publicação: `FIAPIoT/ML_occupancy`
- Intervalo de coleta: 30 segundos

## Funcionamento

### Pipeline de Machine Learning

O sistema implementa um pipeline completo de Machine Learning para IoT:

**Arquitetura:**
```
┌─────────────┐
│   ESP32     │
│  Sensores   │ ──► Temperature, Humidity, Light, CO2, HumidityRatio
└──────┬──────┘
       │ MQTT (30s)
       ▼
┌─────────────┐
│ MQTT Broker │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│  Node-RED   │ ──► Processamento
└──────┬──────┘
       │
       ▼
┌─────────────┐
│  InfluxDB   │ ──► Time-Series Database
└──────┬──────┘
       │
       ▼
┌─────────────┐
│  ML Model   │ ──► XGBoost Classifier
│  (Python)   │     • modelo_ocupacao_best_xgbclassifier.pkl
└──────┬──────┘
       │
       ├──► Console App (appConsoleOccupancy.py)
       └──► Web App     (appWebOccupancy.py)
```

**Features utilizadas:**
- Temperature (°C) - Temperatura ambiente
- Humidity (%) - Umidade relativa do ar
- Light (Lux) - Luminosidade do ambiente
- CO2 (ppm) - Concentração de CO2
- HumidityRatio - Razão de umidade (calculada)

**Modelo de ML:**
- Algoritmo: XGBoost Classifier
- Treinamento: GridSearchCV para otimização de hiperparâmetros
- Saída: Classificação binária (Sala Vazia / Sala Ocupada)
- Confiança: Probabilidades para ambas as classes

## Como Usar

### 1. Treinar o Modelo de Machine Learning

Execute o pipeline de treinamento que utilizará GridSearchCV para encontrar o melhor modelo:

```bash
# O melhor modelo gerado (2025) foi o XGBoost
# Arquivo: modelo_ocupacao_best_xgbclassifier.pkl
```

**Observação:** O modelo pode ser explorado no Google Colab:
- Faça upload do arquivo `modelo_ocupacao_best_xgbclassifier.pkl`
- Observe o carregamento do modelo
- Analise a conexão com InfluxDB
- Estude a estruturação da query Flux
- Examine o método `.predict()` aplicado aos dados

### 2. Configurar Node-RED

1. **Acessar Node-RED:**
   ```
   http://localhost:1880
   ```

2. **Importar o fluxo:**
   - Menu > Import
   - Selecionar arquivo: `app19-Fluxo-Node-RED-Influx.json`
   - Deploy

3. **Verificar configurações:**
   - Nó MQTT: Verificar conexão com broker (mqtt-broker:1883)
   - Nó InfluxDB: Verificar configurações do InfluxDB

### 3. Executar o ESP32

Compile e execute o código no Wokwi:
- O ESP32 coletará dados a cada 30 segundos
- Dados serão publicados no tópico `FIAPIoT/ML_occupancy`
- LED acende durante o envio de dados

### 4. Configurar Ambiente Python

**Criar e ativar ambiente virtual:**

```bash
# Criar virtualenv
python -m venv occupancyEnv

# Ativar (Windows PowerShell)
.\occupancyEnv\Scripts\Activate.ps1

# Ativar (Linux/Mac)
source occupancyEnv/bin/activate

# Atualizar pip
python -m pip install --upgrade pip

# Instalar dependências
pip install -r requirements.txt
```

### 5. Executar Aplicação Console

```bash
python appConsoleOccupancy.py
```

**Saída esperada:**

```
🚀 Monitor de Ocupação de Sala usando ML em dados IoT
==========================================================
📦 Carregando modelo...
✅ Modelo carregado: modelo_ocupacao_best_xgbclassifier.pkl
🌐 Conectando ao InfluxDB...
✅ Conectado ao InfluxDB
🔄 Monitorando dispositivo: FIAP_IoT_app19_001
⏰ Verificando a cada 10 segundos

🔍 Consultando dados... 🆕 Novo dado encontrado!

📊 Dados dos Sensores (02:58:02):
   🌡️  Temperature: 24.6 °C
   💧 Humidity: 40.5 %
   💡 Light: 216.7 Lux
   🌫️  CO2: 1323.7 ppm
   💨 HumidityRatio: 0.004043

🤖 Resultado do Modelo:
   🟢 SALA VAZIA
   📈 Confiança: 74.0%
   📊 Probabilidades:
      • Vazia: 74.0%
      • Ocupada: 26.0%
```

### 6. Executar Aplicação Web

```bash
python appWebOccupancy.py
```

Acesse a interface web no navegador para visualizar as predições em tempo real com interface gráfica.

## Estrutura do Projeto

```
app19_ML_Occupancy/
├── src/
│   ├── iot-aula13-app19_MQTT.ino    # Código ESP32
│   └── ESP32Sensors.hpp              # Biblioteca de sensores
├── app/
│   ├── appConsoleOccupancy.py        # Interface console
│   ├── appWebOccupancy.py            # Interface web
│   ├── ML_app.py                     # Pipeline ML
│   └── modelo_ocupacao_best_xgbclassifier.pkl  # Modelo treinado
├── app19-Fluxo-Node-RED-Influx.json  # Flow Node-RED
├── diagram.json                      # Diagrama Wokwi
├── requirements.txt                  # Dependências Python
└── README.md
```

## Casos de Uso

**1. Otimização de Consumo Energético:**
- Desligar ar-condicionado/iluminação automaticamente quando sala está vazia
- Economia de energia baseada em ocupação real

**2. Gestão de Espaços:**
- Monitoramento de utilização de salas de reunião
- Análise de padrões de ocupação para melhor alocação de recursos

**3. Controle de Qualidade do Ar:**
- Ajuste de ventilação baseado em ocupação e níveis de CO2
- Manutenção de ambiente saudável

**4. Segurança e Acesso:**
- Detecção de ocupação não autorizada
- Controle de acesso baseado em sensoriamento

## Troubleshooting

**Modelo não carrega:**
- Verificar se o arquivo .pkl está no diretório correto
- Verificar compatibilidade de versões do scikit-learn/xgboost

**Dados não aparecem no InfluxDB:**
- Verificar se o Node-RED está conectado ao broker MQTT
- Verificar credenciais do InfluxDB no fluxo Node-RED
- Checar logs do Node-RED

**Aplicação Python não conecta:**
- Verificar se InfluxDB está rodando: `docker ps`
- Verificar configurações de conexão no código
- Verificar se bucket "sensores" existe

**Predições inconsistentes:**
- Aguardar coleta de mais dados para estabilização
- Verificar se sensores estão funcionando corretamente
- Considerar re-treinar modelo com mais dados
