# Fase 3 - Agente LLM para Consulta de Qualidade do Ar

Nesta fase, além da predição e alerta via Telegram, as medições são armazenadas no PostgreSQL e um agente LLM (Ollama/Llama 3.2 ou outro) permite consultar a qualidade do ar em linguagem natural via chat.

## Arquitetura

```
ESP32 (Wokwi) ──► MQTT Broker (Docker) ──► n8n - Fluxo Ingestão ──► ML Service (Local :8000)
                                                    │                        │
                                                    │                   Predição
                                                    │                        │
                                                    ├──► PostgreSQL (armazena medições)
                                                    └──► Telegram (alertas)

Usuário ──► n8n - Fluxo Chat ──► Ollama (Llama 3.2) ──► PostgreSQL (consulta medições)
```

São **dois fluxos n8n** nesta fase:
- **Ingestão:** recebe dados MQTT → prediz → armazena no PostgreSQL → alerta Telegram
- **Chat:** usuário pergunta em linguagem natural → LLM consulta PostgreSQL → responde

## Passo 1 — Parar a Plataforma Anterior

Se a stack da Fase 2 ou a plataforma da Fase 1 ainda estiver rodando, pare:

```bash
wsl -d ubuntu

# Se veio da Fase 2:
cd ~/FIAP-IoT/app20-AQI/Fase2_Plataforma_AI-Predict_Service/ML_AQI_service/
docker compose down

# Se veio da Fase 1:
cd ~/FIAP-IoT/IoT-platform/
docker compose down
```

## Passo 2 — Subir a Plataforma LLM + IoT

```bash
cd ~/FIAP-IoT/LLM-IoT-platform/
sudo ./start-llm-iot-platform.sh
```

Confirme que os serviços estão rodando:
```bash
docker ps
```

Você deve ver: MQTT Broker, n8n, PostgreSQL, Ollama, entre outros.

## Passo 3 — Iniciar o Serviço de ML (manual)

```bash
cd App-Service-AI_Predict/
```

Ativar o ambiente virtual:
```bash
# Windows PowerShell
.\venv\Scripts\Activate.ps1

# Linux/Mac
source venv/bin/activate
```

Subir o serviço:
```bash
uvicorn service_app:app --reload --port 8000
```

> **Importante:** os arquivos `modelo_aqi_nn.keras` e `preprocess_aqi.pkl` devem estar neste diretório.

## Passo 4 — Importar os Fluxos no n8n

Acesse o n8n: http://localhost:5678

### Fluxo 1: Ingestão + Predição + PostgreSQL

1. Menu → **Import from File**
2. Selecione: `Fase3_Plataforma_AI-LLM_Agente/Fluxos-n8n-LLM/LLM-AQI_ingestão_postgres.json`
3. Configure as credenciais:
   - **MQTT:** Host `mqtt-broker`, Porta `1883`
   - **PostgreSQL:** conforme a plataforma (host, porta, usuário, senha, banco)
   - **Telegram:** Bot Token (via @BotFather) e Chat ID (via @userinfobot)
4. No nó **Predict AQI**, confirme a URL: `http://host.docker.internal:8000/predict`
5. Ative o workflow

### Fluxo 2: Chat com Agente LLM

1. Menu → **Import from File**
2. Selecione: `Fase3_Plataforma_AI-LLM_Agente/Fluxos-n8n-LLM/LLM-AQI-Chat_IoT_ML.json`
3. Configure as credenciais:
   - **Ollama:** URL do serviço Ollama da plataforma
   - **PostgreSQL:** mesmas credenciais do fluxo anterior
4. Ative o workflow

O chat ficará disponível via URL pública do n8n (exibida no nó **When chat message received**).

## Passo 5 — Executar o ESP32 no Wokwi

1. Abra o VSCode com a extensão Wokwi
2. Abra o projeto em `App-Device-ESP32_AQI/`
3. Compile (PlatformIO) e inicie a simulação Wokwi
4. Ajuste os potenciômetros para simular diferentes condições de ar
5. No Serial Monitor, confirme:
   ```
   SUCESSO: Dados AQI enviados para o MQTT Broker
   ```

## Passo 6 — Testar o Chat LLM

Acesse a URL do chat (exibida no fluxo do n8n) e faça perguntas como:

- *"Como está a qualidade do ar agora?"* → usa a ferramenta `aqi_atual`
- *"Como estava a qualidade do ar ontem à noite?"* → usa a ferramenta `aqi_historico`
- *"E nesta manhã?"* → usa a ferramenta `aqi_historico`

O agente consulta o PostgreSQL e responde em linguagem natural.

## Validando o Fluxo Completo

| Onde verificar | O que esperar |
|---|---|
| Serial Monitor (ESP32) | `SUCESSO: Dados AQI enviados para o MQTT Broker` |
| Terminal do ML Service | `POST /predict HTTP/1.1 200 OK` |
| n8n - Fluxo Ingestão (Executions) | Execuções com predição + gravação no PostgreSQL |
| Telegram | Mensagens de INFO ou ALERTA conforme classificação |
| n8n - Chat LLM | Respostas do agente baseadas nos dados armazenados |