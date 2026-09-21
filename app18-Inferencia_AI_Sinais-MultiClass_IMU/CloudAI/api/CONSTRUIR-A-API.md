# Construir a API do app25, do zero

A API recebe as 8 features de uma janela do ESP32, consulta o modelo treinado
e devolve a classe e as probabilidades em JSON.

```text
ESP32 → MQTT → n8n → API Python → modelo .pkl
                       ↓
ESP32 ← MQTT ← n8n ← classe
```

## Etapa 0 — Preparar o ambiente

Na pasta `api`, você precisa destes dois arquivos já fornecidos:

- `modelo_motor_multiclasse.pkl`: modelo exportado pelo notebook de treino.
- `requirements.txt`: bibliotecas e versões usadas nesta aula.

Abra o terminal **PowerShell** nessa pasta e execute:

```powershell
python -m venv venv
.\venv\Scripts\python.exe -m pip install -r requirements.txt
```

Usaremos o Python do `venv` diretamente, sem precisar ativar o ambiente.
Mantenha as versões do `requirements.txt` para carregar o modelo no ambiente
compatível com o treino.

## Etapa 1 — Ler o `.pkl` e extrair as classes

O `.pkl` é binário: para consultar seu conteúdo, carregue o objeto com `joblib`.
Use o arquivo fornecido na aula; carregar um pickle pode executar código.

Crie `inspecionar_modelo.py` dentro de `api`:

```python
import joblib

modelo = joblib.load("modelo_motor_multiclasse.pkl")

print("Objeto:", type(modelo).__name__)
print("Etapas:", modelo.steps)
print("Classificador:", type(modelo[-1]).__name__)
print("Features:", modelo.feature_names_in_.tolist())

classes = modelo.classes_.tolist()
print("Classes:", classes)
print("Quantidade de classes:", len(classes))

for indice, classe in enumerate(classes):
    print(indice, classe)
```

Execute:

```powershell
.\venv\Scripts\python.exe inspecionar_modelo.py
```

**Confira:** o arquivo desta aula contém um `Pipeline` com `StandardScaler`
e `MLPClassifier`. As classes são:

```text
0 anomalia
1 inclinado_frente
2 inclinado_tras
3 operando
```

`classes_` contém os rótulos aprendidos. Essa também é a ordem das colunas
de `predict_proba()`: a probabilidade de índice `0` corresponde a `anomalia`.
Não digite uma lista de classes manualmente: extraia do modelo.

O `StandardScaler` já está no pipeline. A API entrega as features originais;
o próprio pipeline aplica a transformação antes de classificar.

## Etapa 2 — Criar a API e carregar o modelo

Comece com `service_app.py` vazio. **Os blocos das etapas 2 a 6 são cumulativos:**
adicione cada um ao final do arquivo, na ordem indicada.

```python
import os

import joblib
import pandas as pd
from fastapi import FastAPI
from pydantic import BaseModel

app = FastAPI(title="app25 - IMU multiclasse: 4 estados do motor")

MODELO_ARQUIVO = os.getenv("MODELO_ARQUIVO", "modelo_motor_multiclasse.pkl")
modelo = joblib.load(MODELO_ARQUIVO)
```

O modelo é carregado uma vez por processo, ao iniciar a API.
`MODELO_ARQUIVO` permite trocar o caminho por uma variável de ambiente;
sem ela, usamos o `.pkl` da pasta atual.

## Etapa 3 — Definir as features

```python
FEATURES = [
    "mean_ax",
    "mean_ay",
    "mean_az",
    "std_ax",
    "std_ay",
    "std_az",
    "std_mag",
    "p2p_mag",
]

print(f"Modelo carregado: {MODELO_ARQUIVO}")
print(f"Tipo: {type(modelo[-1]).__name__}")
print(f"Features: {', '.join(FEATURES)}")
print(f"Classes: {', '.join(modelo.classes_)}")
```

**Confira:** os nomes e a ordem devem ser iguais aos de `feature_names_in_`
exibidos na etapa 1. O ESP32 já calculou essas features; a API não recalcula
médias, desvios ou magnitude.

## Etapa 4 — Definir o JSON de entrada

```python
class JanelaMotor(BaseModel):
    mean_ax: float
    mean_ay: float
    mean_az: float
    std_ax: float
    std_ay: float
    std_az: float
    std_mag: float
    p2p_mag: float
```

O Pydantic valida os oito campos. Se faltar algum ou um valor não puder ser
convertido em número, a API responde com erro `422`.
Campos extras, como `device`, são ignorados por padrão.

## Etapa 5 — Criar uma rota para consultar o modelo

```python
@app.get("/")
def raiz():
    return {
        "servico": "app25 - estado do motor (multiclasse)",
        "modelo": type(modelo[-1]).__name__,
        "features": FEATURES,
        "classes": list(modelo.classes_),
    }
```

Essa rota permite conferir pelo navegador qual modelo foi carregado e quais
features e classes ele utiliza.

## Etapa 6 — Criar a rota de inferência

```python
@app.post("/predict")
def predict(janela: JanelaMotor):
    x_df = pd.DataFrame([janela.model_dump()])[FEATURES]

    predicao = str(modelo.predict(x_df)[0])
    probabilidades = modelo.predict_proba(x_df)[0]

    resultado = {
        "class": predicao,
        "probabilities": {
            classe: float(p)
            for classe, p in zip(modelo.classes_, probabilidades)
        },
    }

    print("--- Resultado da Predicao do Motor ---")
    print(f"  entrada: {janela.model_dump()}")
    print(f"  saida:   {resultado}")
    print("--------------------------------------")

    return resultado
```

- `model_dump()` transforma os campos validados em um dicionário.
- `DataFrame([...])[FEATURES]` cria uma linha e coloca as colunas na ordem do treino.
- `predict(...)[0]` extrai a classe da única janela enviada.
- `predict_proba(...)[0]` extrai as probabilidades dessa janela.
- `zip(...)` associa cada classe à sua probabilidade, na ordem do modelo.

O campo `class` já contém texto, como `operando`. Não é necessário converter
um número em nome de classe.

## Etapa 7 — Executar e testar

No terminal, ainda dentro de `api`:

```powershell
.\venv\Scripts\python.exe -m uvicorn service_app:app --host 0.0.0.0 --port 8000 --reload
```

`service_app:app` indica o arquivo `service_app.py` e o objeto `app`.
O `--reload` reinicia a API quando você salva alterações durante a aula.
Deixe esse terminal aberto; use `Ctrl+C` para encerrar.

1. Abra `http://localhost:8000/` e confira as classes e features.
2. Abra `http://localhost:8000/docs`.
3. Expanda **POST /predict**, clique em **Try it out** e cole:

```json
{
  "mean_ax": -0.413,
  "mean_ay": 0.811,
  "mean_az": 0.965,
  "std_ax": 0.017,
  "std_ay": 0.011,
  "std_az": 0.005,
  "std_mag": 0.01,
  "p2p_mag": 0.04
}
```

4. Clique em **Execute**.

**Confira:** resposta `200`, com `class` contendo uma das quatro classes e
`probabilities` contendo uma probabilidade por classe. A soma deve ficar
próxima de `1`. A classe e os valores dependem do modelo carregado.

**Teste a validação:** remova `std_mag` e envie novamente. A resposta deve
ser `422`, indicando o campo ausente. Depois restaure o JSON completo.

## Etapa 8 — Conectar ao n8n

No fluxo fornecido, o nó **HTTP Request** usa:

- Método: `POST`.
- URL: `http://host.docker.internal:8000/predict` (n8n no Docker Desktop e API no Windows).
- Corpo: JSON com as oito features, conforme o teste anterior.

O nó MQTT de saída publica `{{ $json.class }}` em
`FIAPIoT/motor/multiclasse/cmd`, com **Send Input Data** desligado.
O ESP32 recebe somente o nome da classe e aciona a saída correspondente.

## Se não funcionar

| Sintoma | Confira |
|---|---|
| `No module named ...` | Instalou o `requirements.txt` e está usando o Python do `venv`? |
| `.pkl` não encontrado | O terminal está na pasta `api`, junto do modelo? |
| Erro ao carregar o modelo | O `.pkl` é o da aula e as versões instaladas são as do `requirements.txt`? |
| Resposta `422` | O corpo contém as oito features, com os nomes corretos e valores numéricos? |
| Navegador funciona, n8n não conecta | A API está em `0.0.0.0:8000` e o n8n usa `host.docker.internal`? |
| Classe inesperada | Confira modelo, ordem das features e paridade do sensor com o treino. |
