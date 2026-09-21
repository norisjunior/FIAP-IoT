# Construir a API do app20, do zero

A API recebe as seis features escalares medidas a bordo, consulta a Random
Forest treinada no app19 e devolve a classe e as probabilidades em JSON.

```text
ESP32 → MQTT → n8n → API Python → modelo .pkl
                       ↓
ESP32 ← MQTT ← n8n ← classe
```

Três etapas. Cada uma roda.

1. Abrir o `.pkl` e ver o que tem dentro.
2. A API respondendo no `/docs`.
3. A API conferida contra o notebook.

---

## Etapa 0 — O modelo e o ambiente

Nesta pasta você precisa de dois arquivos:

- `requirements.txt` — já está aqui.
- `modelo_smartbag.pkl` — **você copia**, do `smartbag_rf.zip` que o
  `app19_treinamento_smartbag_rf.ipynb` baixa.

Não há modelo de exemplo, e a API se recusa a subir sem o seu. Uma floresta
inventada responderia com confiança sobre uma bag que ela nunca viu — e como a
resposta sai bem formatada, ninguém desconfiaria.

```powershell
python -m venv venv
.\venv\Scripts\python.exe -m pip install -r requirements.txt
```

Usaremos o Python do `venv` diretamente, sem precisar ativar o ambiente.

**Mantenha as versões do `requirements.txt`.** Elas são as mesmas pinadas na
primeira célula do notebook de treino. `numpy` e `scikit-learn` são as que
importam de verdade: é no formato delas que o `.pkl` se apoia, e uma divergência
aparece como erro no `joblib.load` — ou, pior, como um modelo que carrega e
responde diferente.

---

## Etapa 1 — Ver o que tem no `.pkl`

O `.pkl` é binário. Para consultar o conteúdo, carregue com `joblib`. Use o
arquivo que **você** gerou: carregar um pickle pode executar código.

Crie `inspecionar_modelo.py` nesta pasta:

```python
import joblib

modelo = joblib.load("modelo_smartbag.pkl")

print("Objeto:", type(modelo).__name__)
print("Etapas:", modelo.steps)
print("Classificador:", type(modelo[-1]).__name__)
print("Features:", modelo.feature_names_in_.tolist())
print("Classes:", modelo.classes_.tolist())
print("Arvores:", modelo[-1].n_estimators)
```

```powershell
.\venv\Scripts\python.exe inspecionar_modelo.py
```

Você deve ver um **Pipeline** de duas etapas, `StandardScaler` e
`RandomForestClassifier`, com 21 árvores e as classes `[0, 1]`.

Duas coisas para reparar:

**O scaler viaja dentro do `.pkl`.** Você manda os valores originais — 24,5 °C,
3600 de luz — e a normalização acontece lá dentro. Não normalize antes de
chamar a API: seria normalizar duas vezes.

**As classes são `0` e `1`, não os nomes.** Diferente do app18, onde o treino
usava rótulo em texto e o `predict` já devolvia `"anomalia"`. Aqui o notebook
mapeou os nomes para inteiros porque o **micromlgen**, no app21, só entende
classe numérica. O mapa de volta é responsabilidade de quem consome:

```python
CLASSES = ["ENTREGA_OK", "REVISAR_ENTREGA"]   # o indice E o codigo
```

---

## Etapa 2 — A API

O `service_app.py` já está pronto nesta pasta. As partes que interessam:

**a) Carrega uma vez**, fora das funções. Carregar a cada requisição
funcionaria e seria absurdamente lento — o `.pkl` tem 21 árvores dentro.

**b) A ordem das features é o contrato:**

```python
FEATURES = ["temperatura", "umidade", "delta_distancia", "luz", "mov_max", "incl_max"]
```

```python
x_df = pd.DataFrame([amostra.model_dump()])[FEATURES]
```

O DataFrame casa por **nome**, então trocar a ordem aqui não quebraria a API.
Mas quebraria a comparação com o app21, que monta um `float dados[6]` e casa por
**posição**. Um contrato só, para os dois lados.

**c) Faltar campo é erro, não zero.** Os seis campos do modelo Pydantic não têm
valor padrão: sem um deles, o FastAPI devolve **422** antes de chegar ao modelo.

```powershell
.\venv\Scripts\python.exe -m uvicorn service_app:app --host 0.0.0.0 --port 8000
```

Abra `http://localhost:8000/docs` e use o **Try it out** do `/predict`:

```json
{"temperatura":24.5,"umidade":45.0,"delta_distancia":6.5,"luz":1150,"mov_max":4.8,"incl_max":12.0}
```

- [ ] `GET /` lista as seis features e as duas classes
- [ ] `POST /predict` devolve `class`, `code` e `probabilities`
- [ ] Tire o campo `luz` e reenvie: **422**, dizendo qual campo faltou
- [ ] Mande `"mov_max": null`: **422** também
- [ ] Mande um campo a mais, `"device":"SmartBagEquipe01"`: funciona, ignorado

O penúltimo item é o que o ESP32 evita publicando só com as seis leituras boas.
O último é por que ele pode mandar o `device` junto sem atrapalhar.

---

## Etapa 3 — Conferir contra o notebook

A API não pode discordar do notebook que treinou o modelo. Pegue algumas linhas
do `entradas_teste_normalizadas.csv`... **não**: aquele arquivo está
normalizado, e é para o app21. Para a API, use linhas do **CSV original**, em
valores de sensor.

```python
import joblib, pandas as pd

modelo = joblib.load("modelo_smartbag.pkl")
FEATURES = ["temperatura", "umidade", "delta_distancia", "luz", "mov_max", "incl_max"]

df = pd.read_csv("smartbag_dataset.csv").tail(5)
print(modelo.predict(df[FEATURES]))
```

Os mesmos números mandados ao `/predict` têm que devolver as mesmas classes. Se
divergirem, o `.pkl` da pasta não é o que você acha que é — confira o
`csv_sha256` no `metadados_rf.json` do ZIP.

---

## Deu errado

| Sintoma | Onde olhar |
|---|---|
| `Modelo nao encontrado` ao subir | o `.pkl` não está nesta pasta, ou o nome difere |
| Erro no `joblib.load` | versões: compare o `metadados_rf.json` com o `requirements.txt` |
| `422` em tudo | o corpo não é JSON, ou falta um dos seis campos |
| Sempre a mesma classe | provável no início: com duas rodadas, o modelo é fraco. Confira a matriz de confusão do notebook |
| Porta 8000 ocupada | outra API rodando; troque em `--port` e no nó HTTP do n8n |
