# service_fumaca.py - o modelo da caixa de fumaca vira servico.
#
# Recebe UMA leitura da caixa (1 por segundo) e devolve "fire" ou "not fire".
# E o mesmo esqueleto da API do motor: muda o .pkl e a lista de features.

import os

import joblib
import pandas as pd
from fastapi import FastAPI
from pydantic import BaseModel

app = FastAPI(title="Caixa de fumaca: fire / not fire")

MODELO_ARQUIVO = os.getenv("MODELO_ARQUIVO", "modelo_fumaca_incendio.pkl")
modelo = joblib.load(MODELO_ARQUIVO)

# As sete features, com os mesmos nomes e na mesma ordem do treino.
FEATURES = ["temp", "umid", "raw_h2", "raw_ethanol", "pm2_5", "pm10", "nc0_5"]

print(f"Modelo carregado: {MODELO_ARQUIVO} ({type(modelo).__name__})")
print(f"Features: {', '.join(FEATURES)}")
print(f"Classes: {', '.join(modelo.classes_)}")


class LeituraCaixa(BaseModel):
    # Os campos que a API le do JSON publicado pela caixa. Os outros (device,
    # amostra, rodada, tvoc, eco2, pressao, estimulo...) chegam e sao ignorados.
    temp: float
    umid: float
    raw_h2: float
    raw_ethanol: float
    pm2_5: float
    pm10: float
    nc0_5: float


@app.get("/")
def raiz():
    return {
        "servico": "caixa de fumaca (fire / not fire)",
        "modelo": type(modelo).__name__,
        "features": FEATURES,
        "classes": list(modelo.classes_),
    }


@app.post("/predict")
def predict(leitura: LeituraCaixa):
    x_df = pd.DataFrame([leitura.model_dump()])[FEATURES]
    predicao = str(modelo.predict(x_df)[0])
    probabilidades = modelo.predict_proba(x_df)[0]
    resultado = {
        "class": predicao,
        "probabilities": {
            classe: float(p) for classe, p in zip(modelo.classes_, probabilidades)
        },
    }
    print(f"  entrada: {leitura.model_dump()} -> {predicao}")
    return resultado
