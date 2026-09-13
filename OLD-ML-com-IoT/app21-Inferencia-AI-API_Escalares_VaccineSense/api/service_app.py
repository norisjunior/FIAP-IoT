# service_app.py
import os

import joblib
import pandas as pd
from fastapi import FastAPI
from pydantic import BaseModel

app = FastAPI(title="Vaccine Sense - API de predição")

print("\n" + "=" * 60)
print("Iniciando a API do Vaccine Sense")
print("=" * 60)

# Carrega apenas 1x (rápido nas requisições)
MODEL_PATH = os.getenv("MODEL_PATH", "modelo_vaccinesense.pkl")
modelo = joblib.load(MODEL_PATH)

# As seis medições, na mesma ordem do treinamento.
FEATURES = [
    "tempInterna",
    "tempExterna",
    "umidade",
    "luz",
    "criticidade",
    "distancia",
]

CLASSES = ["TRANSPORTE_OK", "CARGA_EM_PERIGO"]

print(f"Modelo carregado: {MODEL_PATH}")
print(f"Tipo: {type(modelo.named_steps['clf']).__name__}")
print(f"Features: {', '.join(FEATURES)}")
print("=" * 60 + "\n")


class MedicaoVaccineSense(BaseModel):
    tempInterna: float
    tempExterna: float
    umidade: float
    luz: float
    criticidade: float
    distancia: float


@app.get("/")
def raiz():
    return {
        "servico": "Vaccine Sense",
        "modelo": type(modelo.named_steps["clf"]).__name__,
        "features": FEATURES,
        "classes": CLASSES,
    }


@app.post("/predict")
def predict(item: MedicaoVaccineSense):
    # mapeia os nomes do JSON do MQTT -> nomes do treino
    payload = {
        "tempInterna": item.tempInterna,
        "tempExterna": item.tempExterna,
        "umidade": item.umidade,
        "luz": item.luz,
        "criticidade": item.criticidade,
        "distancia": item.distancia,
    }

    # Cria uma lista de valores na ordem correta
    valores = [float(payload[coluna]) for coluna in FEATURES]

    # DataFrame com os nomes das FEATURES: o scikit-learn casa cada valor
    # com a coluna certa, e não é preciso confiar na ordem
    x_df = pd.DataFrame([valores], columns=FEATURES)

    predicao = int(modelo.predict(x_df)[0])
    probabilidades = modelo.predict_proba(x_df)[0]

    # Estrutura do resultado
    resultado = {
        "class": CLASSES[predicao],
        "probabilities": {
            classe: float(p) for classe, p in zip(CLASSES, probabilidades)
        },
    }

    # Mostra resultado no terminal
    print("--- Resultado da Predição Vaccine Sense ---")
    print(f"  entrada: {payload}")
    print(f"  saída:   {resultado}")
    print("-------------------------------------------")

    return resultado
