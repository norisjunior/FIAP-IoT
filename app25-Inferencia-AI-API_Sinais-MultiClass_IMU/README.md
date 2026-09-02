# app25 — Inferência na nuvem: estado do motor (multiclasse)

Recebe por MQTT uma janela de features do `app17-7`, classifica com o `.pkl` e
devolve a classe no tópico de comando. Quatro classes: `operando`,
`inclinado_frente`, `inclinado_tras` e `anomalia`.

```text
ESP32 → MQTT → Node-RED → API (.pkl) → Node-RED → MQTT → ESP32
```

## 1) O modelo

`app17-7-MultiClassAccFeaturesInflux/colab/treinamento_multiclasse.ipynb` gera o
`modelo_motor_multiclasse.pkl`. Copie o arquivo para `api/`.

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
  -d '{"mean_ax":-0.413,"mean_ay":0.811,"mean_az":0.965,"std_ax":0.017,"std_ay":0.011,"std_az":0.005,"std_mag":0.01,"p2p_mag":0.04}'
```

Resposta, com as probabilidades arredondadas:

```json
{"class":"inclinado_frente","probabilities":{"anomalia":0.0,"inclinado_frente":0.99,"inclinado_tras":0.01,"operando":0.0}}
```

## 3) O fluxo Node-RED

Importe `nodered/fluxo-multiclasse-predict.json`.

| Nó | |
|---|---|
| `mqtt in` | `FIAPIoT/motor/multiclasse` |
| `http request` | `POST http://host.docker.internal:8000/predict` |
| `change` | `msg.payload = msg.payload.class` |
| `mqtt out` | `FIAPIoT/motor/multiclasse/cmd` |

Node-RED nativo (fora de contêiner): troque a URL por `http://localhost:8000/predict`.

## 4) O firmware

```bash
cd device
pio run
```

Ajuste no `.cpp` o Wi-Fi e o `MQTT_SERVER` antes de gravar.

| LED externo | |
|---|---|
| 1 piscada | `operando` |
| 2 piscadas | `inclinado_frente` |
| 3 piscadas | `inclinado_tras` |
| 4 piscadas | `anomalia` |
| apagado | sem resposta da nuvem |

LED onboard aceso = conectado ao broker.

No Wokwi não há como inclinar o MPU6050: só `operando` e `anomalia` têm
equivalente no simulador.

## Estrutura

```text
api/       service_app.py · modelo_motor_multiclasse.pkl · requirements.txt
device/    firmware (publica a janela, assina o tópico de comando)
nodered/   fluxo-multiclasse-predict.json
```
