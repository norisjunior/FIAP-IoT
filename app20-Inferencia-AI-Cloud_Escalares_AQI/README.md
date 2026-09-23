# Qualidade do ar: da medição à conversa

Doze sensores de poluentes, uma rede neural que classifica o ar em três faixas,
e um chatbot que responde sobre isso em português.

| Pasta | O que faz |
|---|---|
| [`colab/`](colab/README.md) | treina a rede neural com um dataset público do Kaggle |
| [`api/`](api/) | serve o modelo treinado por HTTP |
| [`CloudAI/`](CloudAI/README.md) | o ESP32 mede, o n8n pergunta à API, o Telegram avisa |
| [`Chatbot/`](Chatbot/README.md) | a mesma predição vai para o PostgreSQL, e um agente LLM responde sobre ela |

```text
CloudAI   ESP32 → MQTT → n8n → API → Telegram
Chatbot   ESP32 → MQTT → n8n → API → PostgreSQL → chat com LLM
```

O `Chatbot` é o `CloudAI` com dois nós a mais no fluxo. Nada muda no dispositivo
nem na API.

## Roda inteiro no Wokwi

Este app não precisa de hardware nenhum. Os doze sensores são potenciômetros no
simulador, e dá para acompanhar o caminho completo — medição, classificação,
banco e conversa — só com o VSCode e o Docker.

É a diferença dele para os apps de motor e de bag, onde alguém precisa montar o
sensor num gabarito e gerar o dataset à mão.

## As três classes

`Aceitável`, `Ruim` e `Perigoso`. Elas vêm de um mapa que reduz as seis faixas
oficiais do dataset a três situações operacionais, e **os nomes são contrato**:
o fluxo do n8n compara a resposta do modelo com a palavra `Perigoso` para
decidir se dispara o alerta crítico. O porquê do corte está em
[`colab/README.md`](colab/README.md).

## A ordem

1. **`colab/`** — opcional na primeira passada. O modelo treinado já vem em
   `api/`; rode o notebook quando quiser o seu.
2. **`CloudAI/`** — sobe a plataforma comum, a API e o fluxo de predição.
3. **`Chatbot/`** — troca para a plataforma com PostgreSQL e Ollama, e
   acrescenta a persistência e o chat.

O `CloudAI` e o `Chatbot` usam **plataformas diferentes**: o segundo precisa do
PostgreSQL e do Ollama, que a plataforma comum não sobe. Pare uma antes de subir
a outra — as duas disputam as mesmas portas.

## A API

```bash
cd api
python -m venv venv

venv\Scripts\activate        # Windows
source venv/bin/activate     # Linux / macOS

pip install -r requirements.txt
uvicorn service_app:app --reload --port 8000
```

Documentação automática em `http://localhost:8000/docs`. Do n8n em contêiner, a
URL é `http://host.docker.internal:8000/predict` — `localhost` ali dentro seria
o próprio contêiner.

## Estrutura

```text
colab/      AQI_NN.ipynb — treinamento com o dataset do Kaggle
api/        service_app.py · modelo_aqi_nn.keras · preprocess_aqi.pkl · requirements.txt
CloudAI/    device/ (ESP32 no Wokwi) · n8n/ (predição + Telegram)
Chatbot/    n8n/ (ingestão + PostgreSQL · chat com o agente LLM)
```
