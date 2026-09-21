# service_app.py - app25: o modelo multiclasse do motor vira servico.
#
# Recebe UMA janela de features (1 s @ 100 Hz) ja calculada a bordo pelo
# app17-7 e devolve a classe. E o MESMO codigo do app24 com uma feature a mais
# na lista: a API nao sabe quantas classes existem - ela devolve o nome que o
# modelo aprendeu.

import os

import joblib
import pandas as pd
from fastapi import FastAPI
from pydantic import BaseModel

app = FastAPI(title="app25 - IMU multiclasse: 4 estados do motor")

print("\n" + "=" * 60)
print("Iniciando a API de estado do motor (multiclasse)")
print("=" * 60)

# Carrega apenas 1x (rapido nas requisicoes). O .pkl e um Pipeline: o
# StandardScaler viaja dentro dele, junto com a floresta.
MODELO_ARQUIVO = os.getenv("MODELO_ARQUIVO", "modelo_motor_multiclasse.pkl")
modelo = joblib.load(MODELO_ARQUIVO)

# As oito features, com os mesmos nomes do treino.
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
print("=" * 60 + "\n")


class JanelaMotor(BaseModel):
    # Os campos que a API le do JSON publicado pelo ESP32. O device, a rodada,
    # a janela, o ts_epoch_ms e o label tambem chegam no JSON e sao
    # simplesmente ignorados: o label e o gabarito do botao, ele NAO entra no
    # modelo.
    mean_ax: float
    mean_ay: float
    mean_az: float
    std_ax: float
    std_ay: float
    std_az: float
    std_mag: float
    p2p_mag: float


@app.get("/")
def raiz():
    return {
        "servico": "app25 - estado do motor (multiclasse)",
        "modelo": type(modelo[-1]).__name__,
        "features": FEATURES,
        "classes": list(modelo.classes_),
    }


@app.post("/predict")
def predict(janela: JanelaMotor):
    # DataFrame de UMA linha com os nomes das FEATURES: o scikit-learn casa
    # cada valor com a coluna certa, e nao e preciso confiar na ordem.
    x_df = pd.DataFrame([janela.model_dump()])[FEATURES]

    # O modelo foi treinado com os rotulos em TEXTO: o predict ja devolve
    # "operando", "inclinado_frente", "inclinado_tras" ou "anomalia".
    predicao = str(modelo.predict(x_df)[0])
    probabilidades = modelo.predict_proba(x_df)[0]

    resultado = {
        "class": predicao,
        "probabilities": {
            classe: float(p) for classe, p in zip(modelo.classes_, probabilidades)
        },
    }

    print("--- Resultado da Predicao do Motor ---")
    print(f"  entrada: {janela.model_dump()}")
    print(f"  saida:   {resultado}")
    print("--------------------------------------")

    return resultado
