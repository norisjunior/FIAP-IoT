# Fase 2 - Stack Docker do Sistema AQI

Nesta fase, toda a infraestrutura (MQTT Broker, ML Service e n8n) roda containerizada via Docker Compose. Um único comando sobe tudo.

## Arquitetura

```
ESP32 (Wokwi) ──► aqi-mosquitto:1883 ──► aqi-n8n:5678 ──► aqi-ml-service:8000 ──► Telegram
                   (container)             (container)       (container)
```

## Passo 1 — Parar a Plataforma da Fase 1

Se a plataforma da Fase 1 ainda estiver rodando, pare-a para liberar as portas:

```bash
wsl -d ubuntu
cd ~/FIAP-IoT/IoT-platform/
docker compose down
```

## Passo 2 — Preparar os Arquivos do Modelo

Copie os arquivos do treinamento (Colab) para o diretório do serviço:

```
Fase2_Plataforma_AI-Predict_Service/ML_AQI_service/ml-service/model/
├── modelo_aqi_nn.keras
└── preprocess_aqi.pkl
```

Confirme:
```bash
ls Fase2_Plataforma_AI-Predict_Service/ML_AQI_service/ml-service/model/
```

## Passo 3 — Subir a Stack

```bash
cd Fase2_Plataforma_AI-Predict_Service/ML_AQI_service/
docker compose up -d --build
```

Confirme que os 3 containers estão rodando:
```bash
docker ps
```

| Container | Porta | Função |
|---|---|---|
| `aqi-mosquitto` | 1883 | MQTT Broker |
| `aqi-ml-service` | 8000 | Serviço de ML |
| `aqi-n8n` | 5678 | Orquestrador de fluxo |

Teste rápido do ML Service:
```bash
curl -X POST http://localhost:8000/predict -H "Content-Type: application/json" -d "{\"PM2_5\":342,\"PM10\":477,\"NO\":10,\"NO2\":51,\"NOx\":40,\"NH3\":42,\"CO\":1.7,\"SO2\":17,\"O3\":91,\"Benzene\":5.2,\"Toluene\":29,\"Xylene\":0.5}"
```

Resposta esperada: `{"class":"Perigoso","probabilities":{...}}`

## Passo 4 — Importar o Fluxo no n8n

1. Acesse o n8n: http://localhost:5678
2. Menu → **Import from File**
3. Selecione: `Fase2_Plataforma_AI-Predict_Service/ML_AQI_service/n8n/AQI.json`
4. Configure as credenciais:
   - **MQTT:** Host `aqi-mosquitto`, Porta `1883`
   - **Telegram:** Bot Token (via @BotFather) e Chat ID (via @userinfobot)
5. No nó **Predict AQI**, confirme a URL: `http://aqi-ml-service:8000/predict`
6. Ative o workflow (toggle no canto superior direito)

> **Atenção:** os endpoints mudaram em relação à Fase 1. Agora usamos nomes de container:
> - MQTT: `aqi-mosquitto` (antes era `mqtt-broker`)
> - ML Service: `aqi-ml-service` (antes era `host.docker.internal`)

## Passo 5 — Executar o ESP32 no Wokwi

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
| `docker logs -f aqi-ml-service` | `POST /predict HTTP/1.1 200 OK` |
| n8n (aba Executions) | Execuções bem-sucedidas |
| Telegram | Mensagens de INFO ou ALERTA conforme classificação |

## Comandos Úteis

```bash
# Ver logs de todos os serviços
docker compose logs -f

# Parar a stack
docker compose down

# Rebuild após mudanças no código
docker compose up -d --build

# Parar e limpar volumes
docker compose down -v
```