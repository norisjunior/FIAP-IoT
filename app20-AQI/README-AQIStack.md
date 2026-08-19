# 🌎 AQI IoT Stack — ESP32 + MQTT + Flask (ML) + n8n

Stack completa para inferência da **qualidade do ar (AQI)** via ESP32 → MQTT → Flask (modelo de Machine Learning) → n8n → Telegram.

---

## ⚙️ Estrutura do Projeto

```
aqi-stack/
├─ docker-compose.yml
├─ mosquitto/
│  └─ mosquitto.conf
├─ ml-service/
│  ├─ Dockerfile
│  ├─ requirements.txt
│  ├─ service_app.py
│  └─ model/
│     ├─ modelo_aqi_nn.keras
│     └─ preprocess_aqi.pkl
└─ n8n/
```

---

## 🚀 Subindo a Stack

```bash
# 1. Clone o repositório
git clone https://github.com/norisjunior/FIAP-IoT.git
cd app20-AQI/AQI_stack

# 2. Suba os containers
docker compose up -d --build
```

---

## 🧩 Serviços

| Serviço        | Porta | Descrição                                    |
| -------------- | ----- | -------------------------------------------- |
| **Mosquitto**  | 1883  | Broker MQTT para o ESP32                     |
| **ML Service** | 8000  | API Flask/FastAPI para predições do modelo   |
| **n8n**        | 5678  | Orquestrador de fluxo (MQTT → ML → Telegram) |

---

## 🔗 Teste Rápido

```bash
curl -X POST http://localhost:8000/predict \
  -H "Content-Type: application/json" \
  -d '{
    "PM2_5": 45, "PM10": 82, "NO": 15, "NO2": 28, "NOx": 40,
    "NH3": 9, "CO": 0.7, "SO2": 6, "O3": 32,
    "Benzene": 2.1, "Toluene": 3.5, "Xylene": 1.2
  }'
```

➡️ Retorno esperado:

```json
{"class":"Aceitável","probabilities":{"Aceitável":0.971751868724823,"Perigoso":8.366844122065231e-05,"Ruim":0.028164511546492577}}
```

---

## 💡 Fluxo de Dados

```
ESP32 → MQTT (mosquitto)
        ↓
      n8n (MQTT Trigger)
        ↓
  Flask ML API (classifica AQI)
        ↓
  Telegram (alerta crítico)
```

## 🧮 Incialize o n8n e crie o fluxo para automatizar

-> Endereço do MQTT Broker: aqi-mosquitto
-> Endereço da aplicação ML: aqi-ml-service