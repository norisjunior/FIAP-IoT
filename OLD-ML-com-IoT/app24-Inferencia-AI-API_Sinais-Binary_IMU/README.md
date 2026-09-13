# app24 — Inferência na nuvem: vibração do motor (binário)

Recebe por MQTT uma janela de features do `app17-6`, classifica com o `.pkl` e
devolve a classe no tópico de comando.

```text
ESP32 → MQTT → Node-RED → API (.pkl) → Node-RED → MQTT → ESP32
```

## 1) O modelo

`app17-6-BinaryAccFeaturesInflux/colab/treinamento_binario.ipynb` gera o
`modelo_vibracao_binaria.pkl`. Copie o arquivo para `api/`.

## 2) A API

```bash
cd api
python -m venv venv

venv\Scripts\activate        # Windows
source venv/bin/activate     # Linux / macOS

pip install -r requirements.txt
uvicorn service_app:app --host 0.0.0.0 --port 8000
```

Documentação automática em `http://localhost:8000/docs`.

```bash
curl -X POST http://localhost:8000/predict \
  -H "Content-Type: application/json" \
  -d '{"mean_ax":-0.019,"mean_ay":-0.009,"mean_az":1.0,"std_ax":1.083,"std_ay":1.107,"std_az":1.185,"rms_mag":2.191}'
```

Resposta, com as probabilidades arredondadas:

```json
{"class":"ligado_anomalia","probabilities":{"ligado_anomalia":1.0,"ligado_normal":0.0}}
```

## 3) O fluxo Node-RED

Importe `nodered/fluxo-binary-predict.json`.

| Nó | |
|---|---|
| `mqtt in` | `FIAPIoT/motor/features` |
| `http request` | `POST http://host.docker.internal:8000/predict` |
| `change` | `msg.payload = msg.payload.class` |
| `mqtt out` | `FIAPIoT/motor/features/cmd` |

Node-RED nativo (fora de contêiner): troque a URL por `http://localhost:8000/predict`.

## 4) O firmware

```bash
cd device
pio run
```

Ajuste no `.cpp` o Wi-Fi e o `MQTT_SERVER` antes de gravar.

| LED externo | |
|---|---|
| aceso fixo | `ligado_normal` |
| piscando | `ligado_anomalia` |
| apagado | sem resposta da nuvem |

LED onboard aceso = conectado ao broker.

## Estrutura

```text
api/       service_app.py · modelo_vibracao_binaria.pkl · requirements.txt
device/    firmware (publica a janela, assina o tópico de comando)
nodered/   fluxo-binary-predict.json
```
