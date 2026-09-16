# service_app.py - app20: a Random Forest da Smart Delivery Bag vira servico.
#
# Recebe as SEIS features escalares medidas a bordo no intervalo de 1 s pelo
# app20 e devolve a classe. E o mesmo modelo que o app21 roda embarcado: aqui
# ele esta na nuvem, la esta na borda. So muda o lugar.

import os

import joblib
import pandas as pd
from fastapi import FastAPI

from pydantic import BaseModel, Field

app = FastAPI(title="app20 - Smart Delivery Bag: revisar a entrega?")

print("\n" + "=" * 60)
print("Iniciando a API da Smart Delivery Bag")
print("=" * 60)

# Carrega apenas 1x (rapido nas requisicoes). O .pkl e um Pipeline: o
# StandardScaler viaja dentro dele, junto com a floresta. Por isso o predict
# recebe os valores ORIGINAIS, na escala dos sensores.
MODELO_ARQUIVO = os.getenv("MODELO_ARQUIVO", "modelo_smartbag.pkl")

if not os.path.exists(MODELO_ARQUIVO):
    raise SystemExit(
        f"\nModelo nao encontrado: {MODELO_ARQUIVO}\n\n"
        "Ele nao vem no repositorio, e nao ha substituto ficticio: uma floresta\n"
        "inventada responderia com confianca sobre uma bag que ela nunca viu.\n\n"
        "Para gerar o seu:\n"
        "  1. Colete duas rodadas completas com o app19.\n"
        "  2. Rode app19_coleta_e_rotulagem.ipynb e baixe o CSV.\n"
        "  3. Rode app19_treinamento_smartbag_rf.ipynb ate o fim.\n"
        "  4. Do smartbag_rf.zip, copie modelo_smartbag.pkl para esta pasta.\n"
    )

modelo = joblib.load(MODELO_ARQUIVO)

# A ORDEM e o contrato. A mesma do CSV, do notebook e do float dados[6] do
# app21. O DataFrame abaixo casa por NOME, entao trocar a ordem aqui nao
# quebraria a API -- mas quebraria a comparacao com o app21, que casa por
# posicao. Um contrato so, para os dois.
FEATURES = [
    "temperatura",
    "umidade",
    "delta_distancia",
    "luz",
    "mov_max",
    "incl_max",
]

# O treino mapeou os nomes para 0 e 1 porque o micromlgen, no app21, so
# entende classe numerica. O indice desta lista E o codigo.
CLASSES = ["ENTREGA_OK", "REVISAR_ENTREGA"]

print(f"Modelo carregado: {MODELO_ARQUIVO}")
print(f"Tipo: {type(modelo[-1]).__name__} com {type(modelo[0]).__name__}")
print(f"Features: {', '.join(FEATURES)}")
print(f"Classes: {dict(enumerate(CLASSES))}")
print("=" * 60 + "\n")


class AmostraBag(BaseModel):
    # Os campos que a API le do JSON publicado pelo ESP32. O device tambem
    # chega no JSON e e simplesmente ignorado.
    #
    # Sem valor padrao: faltar uma feature e erro 422, nao zero. Zero e um
    # numero plausivel para qualquer uma destas seis, e o modelo o trataria
    # como medida boa.
    temperatura: float = Field(description="graus Celsius")
    umidade: float = Field(description="porcentagem")
    delta_distancia: float = Field(description="cm em relacao ao baseline, com sinal")
    luz: float = Field(description="RAW do ADC, 0 a 4095")
    mov_max: float = Field(description="m/s2, pico do intervalo")
    incl_max: float = Field(description="graus, pico do intervalo")


@app.get("/")
def raiz():
    return {
        "servico": "app20 - Smart Delivery Bag",
        "modelo": type(modelo[-1]).__name__,
        "features": FEATURES,
        "classes": CLASSES,
    }


@app.post("/predict")
def predict(amostra: AmostraBag):
    # DataFrame de UMA linha com os nomes das FEATURES: o scikit-learn casa
    # cada valor com a coluna certa, e nao e preciso confiar na ordem.
    x_df = pd.DataFrame([amostra.model_dump()])[FEATURES]

    # O Pipeline normaliza e classifica numa chamada so. O predict devolve o
    # INTEIRO 0 ou 1 -- diferente do app18, onde o treino usava rotulo em texto
    # e o predict ja devolvia o nome. Aqui o mapa de volta e nosso.
    codigo = int(modelo.predict(x_df)[0])
    probabilidades = modelo.predict_proba(x_df)[0]

    resultado = {
        "class": CLASSES[codigo],
        "code": codigo,
        "probabilities": {
            CLASSES[int(c)]: round(float(p), 4)
            for c, p in zip(modelo.classes_, probabilidades)
        },
    }

    print("--- Resultado da Predicao da Bag ---")
    print(f"  entrada: {amostra.model_dump()}")
    print(f"  saida:   {resultado}")
    print("------------------------------------")

    return resultado
