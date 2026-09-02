#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
gerar_modelo_sintetico.py - app25 (4 classes do motor, app17-7)

ANDAIME TEMPORARIO. Cria janelas sinteticas com a mesma FORMA das que o
app17-7 publica e treina um .pkl de brinquedo, so para a API poder ser testada
antes da coleta real. Quando o modelo treinado com os dados do InfluxDB chegar,
basta substituir o .pkl - nada mais muda.

A fisica que este gerador respeita (a mesma do app17-7):
  - o FastIMU devolve aceleracao em g: parado e nivelado -> mean_az ~ 1.0
  - mean_ax/ay/az = projecao da gravidade -> separa inclinado_frente de
    inclinado_tras (as duas SO diferem em orientacao)
  - std_ax/ay/az e std_mag = vibracao      -> separa operando de anomalia
    (as duas estao na MESMA orientacao, niveladas)
  - p2p_mag = pior caso da janela; num sinal ~gaussiano de 100 amostras da
    ~5x o desvio, e sobe mais que isso quando ha impacto isolado (anomalia)
  - nenhuma familia sozinha resolve as 4 classes: e o gancho do notebook 2.6
"""

import os

import numpy as np
import pandas as pd
import joblib
from sklearn.inspection import permutation_importance
from sklearn.neural_network import MLPClassifier
from sklearn.pipeline import make_pipeline
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import accuracy_score, classification_report, confusion_matrix

rng = np.random.default_rng(42)

G = 1.0                  # gravidade em g (unidade do FastIMU)
RODADAS = 3              # voltas completas pela sequencia de classes
JANELAS_POR_RODADA = 30  # META_JANELAS do app17-7
ARQUIVO_MODELO = "modelo_motor_multiclasse.pkl"
ARQUIVO_CSV = "dataset_sintetico.csv"

if os.path.exists(ARQUIVO_MODELO):
    raise SystemExit(
        f"\n{ARQUIVO_MODELO} ja existe e este script gera um modelo SINTETICO.\n"
        "Apague o arquivo antes de rodar, para nao sobrescrever um modelo real.")

RUIDO_MONTAGEM_GRAUS = 4.0

# Cada classe = uma INCLINACAO (graus, eixo x) + uma faixa de VIBRACAO + o
# fator de pico do p2p_mag. Note operando x anomalia: mesma inclinacao, so a
# vibracao muda. E inclinado_frente x inclinado_tras: mesma vibracao, so a
# inclinacao muda.
CLASSES = {
    "operando":         dict(inclinacao=0.0,   vibracao=(0.030, 0.100), pico=(4.5, 5.5)),
    "inclinado_frente": dict(inclinacao=25.0,  vibracao=(0.030, 0.100), pico=(4.5, 5.5)),
    "inclinado_tras":   dict(inclinacao=-25.0, vibracao=(0.030, 0.100), pico=(4.5, 5.5)),
    "anomalia":         dict(inclinacao=0.0,   vibracao=(0.300, 0.800), pico=(6.0, 9.0)),
}
SEQUENCIA = list(CLASSES.keys())   # a mesma ordem ciclica do firmware


def gerar_janela(cfg):
    """Uma janela = vetor gravidade (as medias) + vibracao (os desvios)."""
    theta = np.deg2rad(cfg["inclinacao"] + rng.normal(0, RUIDO_MONTAGEM_GRAUS))
    phi = np.deg2rad(rng.normal(0, RUIDO_MONTAGEM_GRAUS))
    mean_ax = G * np.sin(theta)
    mean_ay = G * np.sin(phi)
    mean_az = G * np.cos(theta) * np.cos(phi)

    std_ax, std_ay, std_az = rng.uniform(*cfg["vibracao"], size=3)

    # std_mag: a vibracao vista NA MAGNITUDE. So a parte da vibracao alinhada
    # com a gravidade sobrevive - por isso projetamos os desvios na direcao do
    # vetor medio (propagacao de variancia de 1a ordem).
    media = np.array([mean_ax, mean_ay, mean_az])
    direcao = media / np.linalg.norm(media)
    std_mag = np.linalg.norm(direcao * np.array([std_ax, std_ay, std_az]))

    p2p_mag = std_mag * rng.uniform(*cfg["pico"])

    return dict(mean_ax=mean_ax, mean_ay=mean_ay, mean_az=mean_az,
                std_ax=std_ax, std_ay=std_ay, std_az=std_az,
                std_mag=std_mag, p2p_mag=p2p_mag)


# ===== 1. Gerar as janelas =====
print("1. Gerando janelas sinteticas...")
linhas = []
ts = 1_749_760_000_000
for rodada in range(1, RODADAS + 1):
    for label in SEQUENCIA:                      # a sequencia ciclica do botao 18
        for janela in range(1, JANELAS_POR_RODADA + 1):
            linha = {"ts_epoch_ms": ts, "label": label,
                     "rodada": rodada, "janela": janela}
            linha.update(gerar_janela(CLASSES[label]))
            linhas.append(linha)
            ts += 1000

COLUNAS = ["ts_epoch_ms", "label", "rodada", "janela",
           "mean_ax", "mean_ay", "mean_az",
           "std_ax", "std_ay", "std_az", "std_mag", "p2p_mag"]
df = pd.DataFrame(linhas)[COLUNAS]
df.round(3).to_csv(ARQUIVO_CSV, index=False)
print(f"   {len(df)} janelas -> {ARQUIVO_CSV}")
print(df.groupby(["label", "rodada"]).size().to_string())

# ===== 2. Split por RODADA (sem vazamento) =====
# A ultima rodada de cada classe fica de fora do treino - mesma regra do
# notebook 2.6, que usa a rodada como grupo do LeaveOneGroupOut.
print("\n2. Holdout: a ultima rodada de cada classe vai para teste...")
treino = df[df["rodada"] < RODADAS]
teste = df[df["rodada"] == RODADAS]
print(f"   treino: {len(treino)} janelas | teste: {len(teste)} janelas")

# ===== 3. Treinar o Pipeline =====
# O StandardScaler viaja DENTRO do .pkl, e o y vai em TEXTO: a API fica com o
# MESMO codigo do app24, mesmo tendo 4 classes em vez de 2.
print("3. Treinando Pipeline(StandardScaler + MLPClassifier)...")
FEATURES = ["mean_ax", "mean_ay", "mean_az",
            "std_ax", "std_ay", "std_az",
            "std_mag", "p2p_mag"]

modelo = make_pipeline(
    StandardScaler(),
    MLPClassifier(hidden_layer_sizes=(16,), max_iter=2000, random_state=42),
)
modelo.fit(treino[FEATURES], treino["label"])

# ===== 4. Metricas no teste =====
y_pred = modelo.predict(teste[FEATURES])
print(f"\n4. Acuracia no teste: {accuracy_score(teste['label'], y_pred):.3f}")
print(classification_report(teste["label"], y_pred, zero_division=0))
print("Matriz de confusao (linhas = verdade, colunas = previsto):")
print(pd.DataFrame(confusion_matrix(teste["label"], y_pred, labels=list(modelo.classes_)),
                   index=modelo.classes_, columns=modelo.classes_).to_string())

perm = permutation_importance(modelo, teste[FEATURES], teste["label"],
                              n_repeats=20, random_state=42)
importancias = pd.Series(perm.importances_mean, index=FEATURES)
print("\nImportancia das features (permutation):")
print(importancias.sort_values(ascending=False).round(3).to_string())

# ===== 5. Salvar =====
joblib.dump(modelo, ARQUIVO_MODELO)
print(f"\n5. Modelo salvo: {ARQUIVO_MODELO}")
print(f"   classes: {list(modelo.classes_)}")
