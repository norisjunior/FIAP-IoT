# Fase 1 - Execução Manual do Sistema AQI

Nesta fase, o serviço de ML roda localmente no seu computador e o n8n (via Docker) consome a API.

## Arquitetura

```
ESP32 (Wokwi) ──► MQTT Broker (Docker) ──► n8n (Docker) ──► ML Service (Local :8000) ──► Telegram
```

## Passo 1 — Subir a Plataforma IoT

```bash
wsl -d ubuntu
cd ~/FIAP-IoT/IoT-platform/
sudo ./start-linux.sh
```

Confirme que os serviços estão rodando:
```bash
docker ps
```

Você deve ver: MQTT Broker, n8n, Node-RED, InfluxDB, Grafana.

## Passo 2 — Iniciar o Serviço de ML

```bash
cd App-Service-AI_Predict/
```

Criar e ativar o ambiente virtual (apenas na primeira vez):
```bash
# Windows PowerShell
python -m venv venv
.\venv\Scripts\Activate.ps1

# Linux/Mac
python3 -m venv venv
source venv/bin/activate
```

Instalar dependências (apenas na primeira vez):
```bash
pip install -r requirements.txt
```

Subir o serviço:
```bash
uvicorn service_app:app --reload --port 8000
```

Teste rápido para confirmar que está funcionando:
```bash
curl -X POST http://localhost:8000/predict -H "Content-Type: application/json" -d "{\"PM2_5\":342,\"PM10\":477,\"NO\":10,\"NO2\":51,\"NOx\":40,\"NH3\":42,\"CO\":1.7,\"SO2\":17,\"O3\":91,\"Benzene\":5.2,\"Toluene\":29,\"Xylene\":0.5}"
```

Resposta esperada: `{"class":"Perigoso","probabilities":{...}}`

> **Importante:** os arquivos `modelo_aqi_nn.keras` e `preprocess_aqi.pkl` (gerados no treinamento via Colab) devem estar neste diretório.

## Passo 3 — Importar o Fluxo no n8n

1. Acesse o n8n: http://localhost:5678
2. Menu → **Import from File**
3. Selecione: `App-Fluxo_n8n/Fluxo-n8n-predict.json`
4. Configure as credenciais:
   - **MQTT:** Host `mqtt-broker`, Porta `1883`
   - **Telegram:** Bot Token (via @BotFather) e Chat ID (via @userinfobot)
5. No nó **Predict AQI**, confirme a URL: `http://host.docker.internal:8000/predict`
6. Ative o workflow (toggle no canto superior direito)

## Passo 4 — Executar o ESP32 no Wokwi

1. Abra o VSCode com a extensão Wokwi
2. Abra o projeto em `App-Device-ESP32_AQI/`
3. Compile (PlatformIO) e inicie a simulação Wokwi
4. Ajuste os potenciômetros para simular diferentes condições de ar
5. No Serial Monitor, confirme:
   ```
   SUCESSO: Dados AQI enviados para o MQTT Broker
   ```

## Validando o Fluxo Completo

| Onde verificar | O que esperar |
|---|---|
| Serial Monitor (ESP32) | `SUCESSO: Dados AQI enviados para o MQTT Broker` |
| Terminal do ML Service | `POST /predict HTTP/1.1 200 OK` |
| n8n (aba Executions) | Execuções bem-sucedidas |
| Telegram | Mensagens de INFO ou ALERTA conforme classificação |