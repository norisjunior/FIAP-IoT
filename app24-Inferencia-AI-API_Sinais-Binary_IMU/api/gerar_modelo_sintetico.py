#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
gerar_modelo_sintetico.py - app24 (binario: ligado_normal x ligado_anomalia)

ANDAIME TEMPORARIO. Cria janelas sinteticas com a mesma FORMA das que o
app17-6 publica e treina um .pkl de brinquedo, so para a API poder ser testada
antes da coleta real. Quando o modelo treinado com os dados do InfluxDB chegar,
basta substituir o .pkl - nada mais muda.

A fisica que este gerador respeita (a mesma do app17-6):
  - o FastIMU devolve aceleracao em g: parado e nivelado -> mean_az ~ 1.0
  - a MEDIA de cada eixo e a projecao da gravidade  -> diz a ORIENTACAO
  - o DESVIO PADRAO de cada eixo e a vibracao       -> diz a INTENSIDADE
  - rms^2 = media^2 + desvio^2 (identidade exata, eixo a eixo)
  - as duas classes ficam na MESMA orientacao: so a vibracao as separa,
    que e o que o notebook 1.5 espera que o modelo aprenda.
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

G = 1.0                 # gravidade em g (unidade do FastIMU)
JANELAS_POR_CLASSE = 90  # 3 rodadas de 30 janelas, como no protocolo de coleta
ARQUIVO_MODELO = "modelo_vibracao_binaria.pkl"
ARQUIVO_CSV = "dataset_sintetico.csv"

if os.path.exists(ARQUIVO_MODELO):
    raise SystemExit(
        f"\n{ARQUIVO_MODELO} ja existe e este script gera um modelo SINTETICO.\n"
        "Apague o arquivo antes de rodar, para nao sobrescrever um modelo real.")

# Ruido de montagem: o sensor nunca fica perfeitamente nivelado. Vale para as
# DUAS classes, entao mean_* nao carrega informacao de classe nenhuma.
RUIDO_MONTAGEM_GRAUS = 5.0

# A unica coisa que separa as classes: o quanto o sinal treme dentro da janela.
VIBRACAO = {
    "ligado_normal":   (0.005, 0.030),   # motor parado: so o ruido do MPU
    "ligado_anomalia": (0.250, 1.200),   # sensor em movimento
}


def gerar_janela(faixa_vibracao):
    """Uma janela = vetor gravidade (as medias) + vibracao (os desvios)."""
    # Orientacao: gravidade projetada nos 3 eixos, com o desalinho da montagem.
    theta = np.deg2rad(rng.normal(0, RUIDO_MONTAGEM_GRAUS))
    phi = np.deg2rad(rng.normal(0, RUIDO_MONTAGEM_GRAUS))
    mean_ax = G * np.sin(theta)
    mean_ay = G * np.sin(phi)
    mean_az = G * np.cos(theta) * np.cos(phi)

    std_ax, std_ay, std_az = rng.uniform(*faixa_vibracao, size=3)

    # rms por eixo e rms da magnitude saem das duas familias acima.
    rms_ax = np.hypot(mean_ax, std_ax)
    rms_ay = np.hypot(mean_ay, std_ay)
    rms_az = np.hypot(mean_az, std_az)
    rms_mag = np.sqrt(rms_ax**2 + rms_ay**2 + rms_az**2)

    return dict(mean_ax=mean_ax, mean_ay=mean_ay, mean_az=mean_az,
                std_ax=std_ax, std_ay=std_ay, std_az=std_az,
                rms_ax=rms_ax, rms_ay=rms_ay, rms_az=rms_az, rms_mag=rms_mag)


# ===== 1. Gerar as janelas =====
print("1. Gerando janelas sinteticas...")
linhas = []
ts = 1_749_760_000_000            # epoch em ms, so para dar realismo
for label, faixa in VIBRACAO.items():
    for _ in range(JANELAS_POR_CLASSE):
        linha = {"ts_epoch_ms": ts, "label": label}
        linha.update(gerar_janela(faixa))
        linhas.append(linha)
        ts += 1000                # 1 janela por segundo

COLUNAS = ["ts_epoch_ms", "label", "mean_ax", "mean_ay", "mean_az",
           "std_ax", "std_ay", "std_az",
           "rms_ax", "rms_ay", "rms_az", "rms_mag"]
df = pd.DataFrame(linhas)[COLUNAS]
df.round(3).to_csv(ARQUIVO_CSV, index=False)
print(f"   {len(df)} janelas -> {ARQUIVO_CSV}")
print(df["label"].value_counts().to_string())

# ===== 2. Split cronologico por classe (sem vazamento) =====
# Janelas vizinhas no tempo sao quase iguais: um split aleatorio vazaria o
# treino para o teste. Mesma regra do notebook 1.5.
print("\n2. Split cronologico 70/30 por classe...")
treino_idx, teste_idx = [], []
for _, g in df.groupby("label"):
    corte = int(len(g) * 0.7)
    treino_idx += list(g.index[:corte])
    teste_idx += list(g.index[corte:])

# ===== 3. Treinar o Pipeline =====
# O StandardScaler viaja DENTRO do .pkl, e o y vai em TEXTO: assim a API so
# precisa de modelo.predict(x) - sem scaler solto, sem mapa de int para nome.
print("3. Treinando Pipeline(StandardScaler + MLPClassifier)...")
FEATURES = ["mean_ax", "mean_ay", "mean_az",
            "std_ax", "std_ay", "std_az",
            "rms_mag"]

modelo = make_pipeline(
    StandardScaler(),
    MLPClassifier(hidden_layer_sizes=(16,), max_iter=2000, random_state=42),
)
modelo.fit(df.loc[treino_idx, FEATURES], df.loc[treino_idx, "label"])

# ===== 4. Metricas no teste =====
y_teste = df.loc[teste_idx, "label"]
y_pred = modelo.predict(df.loc[teste_idx, FEATURES])
print(f"\n4. Acuracia no teste: {accuracy_score(y_teste, y_pred):.3f}")
print(classification_report(y_teste, y_pred, zero_division=0))
print("Matriz de confusao (linhas = verdade, colunas = previsto):")
print(pd.DataFrame(confusion_matrix(y_teste, y_pred, labels=list(modelo.classes_)),
                   index=modelo.classes_, columns=modelo.classes_).to_string())

perm = permutation_importance(modelo, df.loc[teste_idx, FEATURES], df.loc[teste_idx, "label"],
                              n_repeats=20, random_state=42)
importancias = pd.Series(perm.importances_mean, index=FEATURES)
print("\nImportancia das features (permutation):")
print(importancias.sort_values(ascending=False).round(3).to_string())

# ===== 5. Salvar =====
joblib.dump(modelo, ARQUIVO_MODELO)
print(f"\n5. Modelo salvo: {ARQUIVO_MODELO}")
print(f"   classes: {list(modelo.classes_)}")
