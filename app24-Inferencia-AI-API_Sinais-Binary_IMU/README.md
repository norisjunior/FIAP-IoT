# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 24 — Vibração do motor: a decisão acontece na nuvem

O `app17-6` terminou com um dataset de janelas no InfluxDB. Aqui o modelo
treinado com ele vira **serviço**, e a resposta **volta para o dispositivo**: o
Node-RED recebe cada janela, chama a API que executa o `.pkl` e publica a classe
no tópico de comando. O ESP32 escuta e acende o LED.

O ESP32 não decide nada. Ele mede, pergunta e obedece.

## O pipeline

```text
ESP32  →  MQTT  →  Node-RED  →  API FastAPI (.pkl)  →  Node-RED  →  MQTT  →  ESP32
  │                                                                           │
  └── publica 1 janela/s (7 features)                      acende o LED ──────┘
```

Repare no que **não** está nesse caminho: o InfluxDB. Ele é onde o dataset foi
gravado e de onde o notebook treinou — mas a inferência não passa por banco
nenhum. A janela passa, a resposta volta, e o LED responde dentro do mesmo
segundo. Nas Aplicações 20 e 22 é o contrário: lá um script **consulta o banco**
e mostra na tela, e o dispositivo nunca fica sabendo. Mesmo modelo, dois lugares
diferentes na arquitetura.

## Estrutura

```text
api/       a API que executa o modelo (é aqui que o .pkl mora)
device/    firmware: publica a janela E assina o tópico de comando
nodered/   o fluxo da ponte, 4 nós
```

O notebook de treino fica junto do coletor, em
[`app17-6-BinaryAccFeaturesInflux/colab/`](../app17-GerarDatasetSinais_IMU/app17-6-BinaryAccFeaturesInflux/colab/).

## 1. O modelo

Rode `treinamento_binario.ipynb` no Colab: ele lê as janelas do InfluxDB
(measurement `vibracao_binario`), treina, mostra as métricas e baixa o
`modelo_vibracao_binaria.pkl`. Copie o arquivo para `api/`.

O `.pkl` é um **`Pipeline`**: o `StandardScaler` viaja dentro dele, junto com o
classificador. É por isso que a API cabe em 35 linhas — ela carrega um arquivo e
chama `predict`, sem normalizar nada na mão.

O `.pkl` versionado em `api/` foi treinado com dados reais, no Colab. Ele carrega
sem `InconsistentVersionWarning` porque o `requirements.txt` fixa exatamente as
versões de `numpy`, `pandas`, `scikit-learn` e `joblib` que a primeira célula do
notebook instala — o arquivo é lido pelo mesmo ambiente que o escreveu.

## 2. Subir a API

```bash
cd api
pip install -r requirements.txt
uvicorn service_app:app --host 0.0.0.0 --port 8000
```

Confira em `http://localhost:8000/docs` — o FastAPI gera a documentação sozinho,
e dá para testar o `/predict` pelo navegador antes de ligar qualquer
dispositivo. Um teste rápido pela linha de comando:

```bash
curl -X POST http://localhost:8000/predict \
  -H "Content-Type: application/json" \
  -d '{"mean_ax":-0.019,"mean_ay":-0.009,"mean_az":1.0,"std_ax":1.083,"std_ay":1.107,"std_az":1.185,"rms_mag":2.191}'
```

```json
{"class":"ligado_anomalia","probabilities":{"ligado_anomalia":1.0,"ligado_normal":0.0}}
```

> **Os valores estão em `g`**, não em m/s². O FastIMU devolve aceleração em `g`,
> e o firmware publica o número cru: parado e nivelado, `mean_az` fica perto de
> `1.0` e `rms_mag` também.

## 3. Importar o fluxo no Node-RED

Importe `nodered/fluxo-binary-predict.json`. São quatro nós no caminho:

| Nó | O que faz |
|---|---|
| `mqtt in` | assina `FIAPIoT/motor/features` |
| `http request` | `POST` para `http://host.docker.internal:8000/predict` |
| `change` | `msg.payload = msg.payload.class` |
| `mqtt out` | publica a classe em `FIAPIoT/motor/features/cmd` |

Não há nó de código. O `http request` manda `msg.payload` **inteiro** como corpo
JSON, e a API lê só os sete campos que declarou — `device` e `ts_epoch_ms`
chegam junto e são ignorados.

> `host.docker.internal` é como o contêiner do Node-RED enxerga a sua máquina. Se
> o Node-RED for nativo, troque a URL por `http://localhost:8000/predict`.

## 4. Gravar o firmware

```bash
cd device
pio run
```

É o `app17-6` com três acréscimos e uma subtração. Os acréscimos:

```cpp
#define MQTT_SUB_TOPIC "FIAPIoT/motor/features/cmd"
String classePrevista = "";

void receberComando(char* topico, byte* conteudo, unsigned int tamanho) {
  classePrevista = String(conteudo, tamanho);
  classePrevista.trim();
  Serial.printf("MODELO: %s\r\n", classePrevista.c_str());
}
```

Mais `mqttClient.setCallback(receberComando)` no `setup()` e
`mqttClient.subscribe(MQTT_SUB_TOPIC)` ao conectar.

O `tamanho` não é decoração: o buffer que o PubSubClient entrega **não termina em
`\0`** — ele aponta para dentro do buffer de rede, que logo depois é reaproveitado.
`String((char*)conteudo)` compilaria e funcionaria na maior parte dos testes, lendo
até topar com um zero por acaso: às vezes lixo no fim, às vezes a mensagem
anterior. O construtor de dois argumentos elimina isso por construção.

**A subtração são os botões.** O `app17-6` tinha dois porque era um gerador de
dataset: um humano escolhia o rótulo de cada janela. Aqui não há o que escolher —
quem rotula é o modelo. Saíram os botões, o debounce, o campo `label` do payload
e o `if` da coleta. O dispositivo liga e publica, uma janela por segundo, para
sempre. É assim que um sensor de CBM (*Condition-Based Maintenance*) se comporta:
ele não tem botão de "agora estou com defeito".

| LED | Significado |
|---|---|
| externo aceso fixo | a nuvem respondeu `ligado_normal` |
| externo piscando | a nuvem respondeu `ligado_anomalia` |
| externo apagado | a nuvem ainda não respondeu |
| onboard aceso | conectado ao broker |

**Repare no que não existe no firmware:** nenhum `if` sobre `std_ax`, nenhum
limiar de vibração. O ESP32 não sabe o que é anomalia — ele só executa o que a
nuvem mandou.

## O que demonstrar em aula

| O que fazer | O que acontece |
|---|---|
| ligar, sensor parado na mesa | LED apagado por ~1 s, depois **aceso fixo** |
| bater na mesa / sacudir o sensor | LED começa a **piscar** em ~1 s |
| parar de novo | volta a ficar aceso |
| girar o sensor devagar, sem vibrar | continua **aceso**: girar não é anomalia |

A última linha é a que vale a discussão: mudar a orientação mexe nas `mean_*`,
mas o modelo aprendeu que anomalia é **energia na janela**, não postura. Foi por
isso que as duas classes foram coletadas na mesma posição.

## A pergunta que fecha a aula

Pare o `uvicorn` e sacuda o sensor.

O LED **congela** na última resposta. A vibração está lá, o dispositivo continua
publicando, e o alarme está mudo — porque a decisão mora do outro lado da rede.

Um motor industrial fica num galpão, às vezes sem rede. É esse o problema que a
trilha de *Edge AI* resolve, levando o modelo para dentro do ESP32.

## Sobre o modelo

`Pipeline(StandardScaler + MLPClassifier(16,))` — uma rede neural pequena, não
uma árvore. A escolha tem um motivo didático: **Random Forest ignora escala**, e
com ele o `StandardScaler` no pipeline fica decorativo. Com o MLP não: tire o
scaler, treine de novo e veja o modelo desandar. É a demonstração mais direta de
por que o normalizador precisa viajar junto com o modelo, dentro do mesmo `.pkl`.

Trocar de estimador é uma linha no notebook. A API não muda — ela carrega um
`Pipeline`, chama `predict` e devolve o nome que veio; `GET /` reporta qual
modelo está no ar.
