# app25 — Inferência na nuvem: estado do motor (multiclasse)

Recebe por MQTT uma janela de features do `app17-7`, classifica com o `.pkl` e
devolve a classe no tópico de comando. Quatro classes: `operando`,
`inclinado_frente`, `inclinado_tras` e `anomalia`.

```text
ESP32 → MQTT → n8n → API FastAPI (.pkl) → n8n → MQTT → ESP32
                                            └→ Telegram (só em anomalia)
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

## 3) O fluxo n8n

Importe `n8n/Fluxo-n8n-predict.json` (n8n em `http://localhost:5678`) e
selecione as credenciais: MQTT nos dois nós de MQTT e Telegram no nó de alerta.

Passo a passo: [Construir o fluxo n8n](n8n/CONSTRUIR-O-FLUXO.md).

| # | Nó | Configuração |
|---|---|---|
| 1 | `MQTT Trigger` | topic `FIAPIoT/motor/multiclasse` |
| 2 | `Code` | *Run Once for Each Item* — converte a mensagem em JSON |
| 3 | `HTTP Request` | `POST http://host.docker.internal:8000/predict`, body JSON `{{ $json }}` |
| 4 | `MQTT` | topic `FIAPIoT/motor/multiclasse/cmd`, **Send Input Data: OFF**, message `{{ $json.class }}` |
| 5 | `IF` | `{{ $json.class }}` é igual a `anomalia` |
| 6 | `Telegram` | conectado à saída **true** do IF |

Os nós **4 e 5 saem diretamente do nó 3**. Todas as classes voltam ao ESP32;
o Telegram recebe um alerta a cada predição `anomalia`, inclusive repetida.
A validação das oito features fica na API.

- **Send Input Data desligado:** o ESP32 espera somente o nome da classe.
- **URL:** com n8n no Docker Desktop e API no Windows, use `host.docker.internal`.
  Com ambos nativos no mesmo computador, use `localhost`.

## 4) O firmware

```bash
cd device
pio run
```

Ajuste no `.cpp` o Wi-Fi e o `MQTT_SERVER` antes de gravar.

| GPIO | Componente | Classe |
|---|---|---|
| 4 | LED azul | `operando` |
| 21 | LED amarelo | `inclinado_frente` |
| 18 | LED vermelho | `inclinado_tras` |
| 19 | buzzer | `anomalia` |
| — | tudo apagado | sem resposta da nuvem |

Uma saída por classe, e ela **fica** ligada: o dispositivo mantém a última
decisão recebida até a próxima chegar. Os pinos 21 e 18 eram os dois botões do
`app17-7` — de entrada do rótulo humano viraram saída do rótulo do modelo.

LED onboard aceso = conectado ao broker.

No Wokwi não há como inclinar o MPU6050: só `operando` e `anomalia` têm
equivalente no simulador.

## Estrutura

```text
api/       service_app.py · modelo_motor_multiclasse.pkl · requirements.txt
device/    firmware (publica a janela, assina o tópico de comando)
n8n/       Fluxo-n8n-predict.json
```
