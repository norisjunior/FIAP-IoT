# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 25 — Quatro estados do motor, decididos na nuvem

Mesma arquitetura da Aplicação 24, com uma diferença: em vez de responder
*normal ou anomalia*, o modelo responde **qual dos quatro estados** o motor está
— `operando`, `inclinado_frente`, `inclinado_tras` ou `anomalia`.

O interessante é o que **não** mudou para isso acontecer.

## O pipeline

```text
ESP32  →  MQTT  →  Node-RED  →  API FastAPI (.pkl)  →  Node-RED  →  MQTT  →  ESP32
  │                                                                           │
  └── publica 1 janela/s (8 features)              LED pisca a classe ────────┘
```

Como na Aplicação 24, o InfluxDB **não está nesse caminho**: ele guarda o
dataset e alimenta o treino, não a inferência.

## Estrutura

```text
api/       a API que executa o modelo (é aqui que o .pkl mora)
device/    firmware: publica a janela E assina o tópico de comando
nodered/   o fluxo da ponte, 4 nós
```

O notebook de treino fica junto do coletor, em
[`app17-7-MultiClassAccFeaturesInflux/colab/`](../app17-GerarDatasetSinais_IMU/app17-7-MultiClassAccFeaturesInflux/colab/).

## O que muda da Aplicação 24 para esta

| | app24 | app25 |
|---|---|---|
| classes | 2 | 4 |
| features | 7 (`rms_mag`) | 8 (`std_mag`, `p2p_mag`) |
| tópico | `FIAPIoT/motor/features` | `FIAPIoT/motor/multiclasse` |
| **linhas de código a mais na API** | — | **zero** |

A `service_app.py` das duas aplicações é o mesmo arquivo, com uma lista de
features diferente. Não há `if` por classe, nem mapa de número para nome: o
modelo foi treinado com os rótulos em **texto**, então `modelo.predict()` já
devolve `"inclinado_tras"`, e `modelo.classes_` diz quais nomes existem. **A API
não sabe quantas classes o problema tem** — e é justamente por isso que ela não
precisou mudar.

## 1. O modelo

Rode `treinamento_multiclasse.ipynb` no Colab: ele lê as janelas do InfluxDB
(measurement `vibracao_multiclasse`), treina, valida com `LeaveOneGroupOut` por
rodada e baixa o `modelo_motor_multiclasse.pkl`. Copie o arquivo para `api/`.

> **Enquanto o dataset real não existe:** `api/` já vem com um `.pkl` treinado
> com dados **sintéticos**, gerado por `api/gerar_modelo_sintetico.py`. Serve
> para o loop inteiro rodar antes da primeira coleta. Substitua pelo modelo de
> verdade quando ele existir — e acerte o pin do `scikit-learn` no
> `requirements.txt` para a versão que o notebook imprimiu.

## 2. Subir a API

```bash
cd api
pip install -r requirements.txt
uvicorn service_app:app --host 0.0.0.0 --port 8000
```

```bash
curl -X POST http://localhost:8000/predict \
  -H "Content-Type: application/json" \
  -d '{"mean_ax":0.355,"mean_ay":-0.01,"mean_az":0.935,"std_ax":0.036,"std_ay":0.046,"std_az":0.061,"std_mag":0.058,"p2p_mag":0.306}'
```

```json
{"class":"inclinado_frente","probabilities":{"anomalia":0.01,"inclinado_frente":0.96,"inclinado_tras":0.01,"operando":0.03}}
```

> **Os valores estão em `g`**, não em m/s². O FastIMU devolve aceleração em `g`,
> e o firmware publica o número cru: nivelado, `mean_az` fica perto de `1.0`;
> inclinado 25°, parte da gravidade migra para `mean_ax` (`sen 25° ≈ 0,42`).

## 3. Importar o fluxo no Node-RED

Importe `nodered/fluxo-multiclasse-predict.json`. Os mesmos quatro nós da
Aplicação 24, só que nos tópicos `FIAPIoT/motor/multiclasse` e
`.../multiclasse/cmd`.

## 4. Gravar o firmware

```bash
cd device
pio run
```

É o `app17-7` com o callback do MQTT no lugar dos botões. A função
`atualizarLedClasse()` **não mudou uma linha** — o que mudou foi de onde vem o
índice que ela pisca:

```cpp
// app17-7 (gerador de dataset): o índice vem do botão 18
int totalPiscadas = indiceClasse + 1;

// app25 (monitor): o índice vem da resposta da nuvem
int totalPiscadas = classePrevista + 1;
```

A nuvem manda o **nome** da classe; o firmware procura o nome no array
`SEQUENCIA[]` — que já existia — para saber quantas piscadas dar.

| LED externo | Significado |
|---|---|
| 1 piscada | `operando` |
| 2 piscadas | `inclinado_frente` |
| 3 piscadas | `inclinado_tras` |
| 4 piscadas | `anomalia` |
| apagado | a nuvem ainda não respondeu |

Saíram os botões, o debounce, o `META_JANELAS` (a coleta parava em 30 janelas) e
os campos `label`, `rodada` e `janela` do payload. Tudo isso era instrumentação
de **coleta**. Um monitor de condição roda sem parar e não rotula nada.

## O que demonstrar em aula

| O que fazer | O que acontece |
|---|---|
| motor ligado, na posição de trabalho | 1 piscada — `operando` |
| inclinar o conjunto para a frente | 2 piscadas em ~1 s |
| inclinar para trás | 3 piscadas |
| voltar ao nível e bater no motor | 4 piscadas — `anomalia` |

A demonstração que vale a aula é o par **inclinado_frente × inclinado_tras**: as
duas têm a mesma vibração, o motor está ligado nas duas, e o que as separa é só
o sinal de `mean_ax`. Nenhuma feature de vibração distingue as duas — e nenhuma
feature de orientação distingue `operando` de `anomalia`. Foi por isso que o
modelo precisou das **duas famílias**: a permutation importance no notebook
mostra `mean_*` e `std_*`/`p2p_mag` dividindo o trabalho.

> **No Wokwi:** não há como inclinar o MPU6050 do simulador, então só
> `operando` e `anomalia` têm equivalente fiel lá. O simulador serve para testar
> o loop MQTT → API → MQTT; as inclinações pedem o ESP32 físico.

## A pergunta que fecha a aula

Pare o `uvicorn`. O LED **congela** no último padrão de piscadas: o motor pode
tombar, e o dispositivo continua repetindo a última verdade que ouviu.

## Sobre o modelo

`Pipeline(StandardScaler + MLPClassifier(16,))`, igual à Aplicação 24 — inclusive
nos hiperparâmetros. Quatro classes em vez de duas não mudaram nada além do
número de saídas da rede, que o próprio `fit` determina a partir dos rótulos.
