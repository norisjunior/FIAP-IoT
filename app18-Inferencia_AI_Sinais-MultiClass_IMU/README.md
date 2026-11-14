# Estado do motor (multiclasse): na nuvem e na borda

A mesma janela de features da coleta, classificada de dois jeitos. Quatro
classes: `operando`, `inclinado_frente`, `inclinado_tras` e `anomalia`.

| Pasta | Modelo | Quem decide |
|---|---|---|
| `CloudAI/` | a MLP treinada na coleta, num `.pkl` | a nuvem, e o ESP32 obedece |
| `EdgeAI/` | uma Random Forest embarcada | o próprio ESP32, sem rede |

As seções 1 a 4 são o caminho da nuvem; a seção 5 aponta para a borda.

```text
ESP32 → MQTT → n8n → API FastAPI (.pkl) → n8n → MQTT → ESP32
                                            └→ Telegram (só em anomalia)
```

## 1) O modelo

O notebook `treinamento_multiclasse.ipynb`, do app de coleta, gera o
`modelo_motor_multiclasse.pkl`. Copie o arquivo para `CloudAI/api/`.

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
  -d '{"mean_ax":0.049,"mean_ay":0.696,"mean_az":0.687,"std_ax":0.009,"std_ay":0.011,"std_az":0.011,"std_mag":0.011,"p2p_mag":0.088}'
```

Resposta, com as probabilidades arredondadas:

```json
{"class":"inclinado_frente","probabilities":{"anomalia":0.0,"inclinado_frente":0.99,"inclinado_tras":0.01,"operando":0.0}}
```

## 3) O fluxo n8n

Importe `CloudAI/n8n/Fluxo-n8n-predict.json` (n8n em `http://localhost:5678`) e
selecione as credenciais: MQTT nos dois nós de MQTT e Telegram nos dois nós de
aviso. Nos dois nós de MQTT, troque `DEFINA_UM_NOME` pelo nome da sua placa; nos
dois nós do Telegram, troque `SEU_CHAT_ID` pelo seu.

Passo a passo: [Construir o fluxo n8n](CloudAI/n8n/CONSTRUIR-O-FLUXO.md).

| # | Nó | Configuração |
|---|---|---|
| 1 | `MQTT Trigger` | topic `FIAPIoT/motor/multiclasse/grupo03`, option **JSON Parse Body** |
| 2 | `HTTP Request` | `POST http://host.docker.internal:8000/predict`, body JSON `{{ $json.message }}`, com **On Error: continue using error output** |
| 3 | `MQTT` | topic `FIAPIoT/motor/multiclasse/grupo03/cmd`, **Send Input Data: OFF**, message `{{ $json.class }}` |
| 4 | `IF` | `{{ $json.class }}` é igual a `anomalia` |
| 5 | `Telegram` | conectado à saída **true** do IF |
| 6 | `Telegram` | conectado à **saída de erro** do nó 2 |

Os nós **3 e 4 saem diretamente do nó 2**, em paralelo: o alerta nunca atrasa
o LED. Todas as classes voltam ao ESP32.

O nó **6 sai da saída de erro** do nó 2. Sem ele, a API fora do ar não avisa
ninguém: o ESP32 continua publicando e o LED **fica aceso na classe velha**,
parecendo um sistema que funciona.

O Telegram recebe um alerta a cada predição `anomalia`, **inclusive repetida**:
uma anomalia de meio minuto manda meia dúzia de mensagens. É assim de
propósito — o alerta parar poderia ser lido como problema resolvido.

- **Send Input Data desligado:** o ESP32 espera somente o nome da classe.
- **URL:** com n8n no Docker Desktop e API no Windows, use `host.docker.internal`.
  Com ambos nativos no mesmo computador, use `localhost`.

## 4) O firmware

```bash
cd device
pio run
```

Ajuste no `.cpp`, antes de gravar: o Wi-Fi, o `MQTT_SERVER` (o broker da aula) e o
`MQTT_CLIENT_ID`, com **o nome da sua placa** (o mesmo do dataset, sem espaço nem
acento).

**Cada placa tem o seu par de tópicos**, montado com esse nome: publica a janela em
`FIAPIoT/motor/multiclasse/<nome>` e escuta a classe em
`FIAPIoT/motor/multiclasse/<nome>/cmd`. Num broker compartilhado (o da sala ou um
público), é isso que faz a placa obedecer só ao modelo do próprio grupo. Os exemplos
deste README usam `grupo03`.

| GPIO | Componente | Classe |
|---|---|---|
| 4 | LED azul | `operando` |
| 21 | LED amarelo | `inclinado_frente` |
| 18 | LED vermelho | `inclinado_tras` |
| 19 | buzzer | `anomalia` |
| — | tudo apagado | sem resposta da nuvem |

Uma saída por classe, e ela **fica** ligada: o dispositivo mantém a última
decisão recebida até a próxima chegar. Os pinos 21 e 18 eram os dois botões do
app de coleta — de entrada do rótulo humano viraram saída do rótulo do modelo.

LED onboard aceso = conectado ao broker.

Para o `.pkl` de referência (o que vem em `CloudAI/api/`), incline **uns 45°**: ele
foi treinado com a gravidade da inclinação indo para o **eixo y** (`mean_ay` perto
de +0,7 para a frente e de −0,8 para trás). Com o MPU preso de outro jeito, as
inclinações saem erradas: use o `.pkl` do seu dataset.

No Wokwi a **inclinação** sai, pelo controle de aceleração do MPU6050. A
`anomalia` não: ela é vibração, e o controle parado deixa as 100 amostras da
janela idênticas.

## 5) A mesma janela, sem nuvem

[`EdgeAI/`](EdgeAI/README.md) traz o outro extremo: a mesma janela, as mesmas 8
features e as mesmas 4 saídas, mas com uma **Random Forest embarcada** decidindo
dentro do ESP32. Sai o MQTT, sai o n8n, sai a API — e a resposta cai de ~1 s
para poucos microssegundos.

Nada do que está acima muda: o `CloudAI/` continua valendo inteiro. O
`EdgeAI/` é uma segunda leitura do mesmo problema, com um modelo novo treinado a
partir do mesmo dataset da coleta.

## Estrutura

```text
CloudAI/   o modelo na nuvem
  api/       service_app.py · modelo_motor_multiclasse.pkl · requirements.txt
  device/    firmware (publica a janela, assina o tópico de comando)
  n8n/       Fluxo-n8n-predict.json
EdgeAI/    o modelo na borda
  colab/     treinamento da Random Forest
  device/    firmware sem rede + os dois .hpp do modelo
```
