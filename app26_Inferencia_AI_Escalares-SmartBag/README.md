# Inferência na nuvem: revisar a entrega?

A mesma bag do app de coleta, com os mesmos seis sensores e o mesmo cálculo. O que muda
é quem rotula: no app de coleta era uma pessoa apertando um botão; aqui é a Random
Forest treinada com aquele dataset.

```text
ESP32 → MQTT → n8n → API FastAPI (.pkl) → n8n → MQTT → ESP32
```

Duas classes: `ENTREGA_OK` e `REVISAR_ENTREGA`.

## O eixo desta etapa

O app da floresta embarcada roda **esta mesma floresta** dentro do ESP32. A comparação entre os dois
só diz alguma coisa porque o modelo é o mesmo arquivo: muda **onde** ele roda,
não o que ele aprendeu. Por isso nada aqui pode ser retreinado "só para a API".

| | este app | o app da floresta embarcada |
|---|---|---|
| Onde roda | servidor | ESP32 |
| Artefato | `modelo_smartbag.pkl` | `AIoTRandomForest_micromlgen.hpp` |
| Precisa de rede | sim, a cada segundo | não |
| Modelo | **o mesmo** | **o mesmo** |

## 1) O modelo

O `treinamento_smartbag_rf.ipynb` gera o `modelo_smartbag.pkl` dentro do
`smartbag_rf.zip`. Copie o arquivo para `api/`.

**Ele não vem no repositório**, e a API se recusa a subir sem ele — com a
mensagem dizendo como gerá-lo. Não há modelo de exemplo: uma floresta inventada
responderia com confiança sobre uma bag que ela nunca viu.

O `.pkl` é um **Pipeline**: o `StandardScaler` viaja dentro dele. A API manda os
valores originais, na escala dos sensores, e a normalização acontece lá dentro.

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
  -d '{"temperatura":24.5,"umidade":45.0,"delta_distancia":6.5,"luz":1150,"mov_max":4.8,"incl_max":12.0}'
```

Resposta:

```json
{"class":"REVISAR_ENTREGA","code":1,"probabilities":{"ENTREGA_OK":0.238,"REVISAR_ENTREGA":0.762}}
```

O `code` vem junto porque o treino mapeou `ENTREGA_OK = 0` e
`REVISAR_ENTREGA = 1` — é o inteiro que o `predict()` do app da floresta embarcada devolve. Ter os
dois na resposta deixa a comparação entre nuvem e borda direta.

Passo a passo: [Construir a API](api/CONSTRUIR-A-API.md).

## 3) O fluxo n8n

Importe `n8n/Fluxo-n8n-predict.json` (n8n em `http://localhost:5678`) e
selecione a credencial MQTT nos dois nós de MQTT.

| # | Nó | Configuração |
|---|---|---|
| 1 | `MQTT Trigger` | topic `FIAPIoT/smartbag/equipe01/dados` |
| 2 | `Code` | *Run Once for Each Item* — converte a mensagem em JSON |
| 3 | `HTTP Request` | `POST http://host.docker.internal:8000/predict`, body JSON `{{ $json }}` |
| 4 | `MQTT` | topic `FIAPIoT/smartbag/equipe01/cmd`, **Send Input Data: OFF**, message `{{ $json.class }}` |

Passo a passo: [Construir o fluxo n8n](n8n/CONSTRUIR-O-FLUXO.md).

- **Send Input Data desligado:** o ESP32 espera só o nome da classe, não o JSON.
- **URL:** com n8n no Docker e API no Windows, use `host.docker.internal`. Com
  os dois nativos na mesma máquina, `localhost`.
- A validação das seis features fica na API, não no fluxo.

## 4) O firmware

```bash
cd device
pio run
```

Ajuste o Wi-Fi e o `MQTT_SERVER` no `.ino` antes de gravar.

| GPIO | Componente | Significado |
|---|---|---|
| 27 | LED verde | `ENTREGA_OK` |
| 21 | LED vermelho | `REVISAR_ENTREGA` |
| — | tudo apagado | a nuvem não respondeu |

**Feche a bag antes de ligar.** O baseline da distância é medido uma vez, no
`setup()` — aqui não há botão para recalibrar.

**Os botões sumiram.** No app de coleta eles existiam porque aquilo era um gerador de
dataset, com uma pessoa rotulando cada amostra. O GPIO 27, que era o botão
COLETA, virou saída: de entrada do rótulo humano para saída do rótulo do modelo.

O LED 21 é o mesmo pino do alerta do NexoLog. Lá quem acendia era um limiar
desenhado no Node-RED; aqui é um modelo treinado. O caminho é o mesmo — o
dispositivo mede, publica e obedece —, o que mudou foi a natureza da decisão.

Passo a passo: [Construir o firmware](device/CONSTRUIR-O-FIRMWARE.md).

## O que o dispositivo faz quando algo falha

Nenhum destes casos acende o verde. `ENTREGA_OK` é uma afirmação sobre a carga,
e afirmar isso por falta de resposta seria mentir na direção mais cara.

| Situação | O que acontece |
|---|---|
| Uma das seis features sem leitura | não publica; Serial avisa qual faltou |
| Sem baseline no boot | Serial manda conferir o HC-SR04 e reiniciar |
| Sem resposta da nuvem por 5 s | as duas saídas apagam |
| Classe com nome desconhecido | tudo apagado, e o payload recebido vai ao Serial |

O tempo de validade existe porque a saída é a memória do dispositivo: ela fica
como está até chegar algo novo. Sem prazo, um LED verde aceso às 14h continuaria
aceso às 18h com a rede caída — dizendo "está tudo bem" sobre uma bag que
ninguém está medindo há quatro horas.

## Estrutura

```text
api/       service_app.py · requirements.txt · (modelo_smartbag.pkl, você copia)
device/    firmware: publica as seis features, assina o tópico de comando
n8n/       Fluxo-n8n-predict.json
```
