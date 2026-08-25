#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Validação ponta a ponta do modelo.

Pega medições do CSV, monta o JSON exatamente como o ESP32 publica no MQTT,
passa pelo mesmo caminho que a aplicação usa e compara com o rótulo conhecido.

Serve para conferir que o .pkl funciona antes de ligar a caixa.

    python validar_modelo.py
"""

import json
import sys

import joblib
import pandas as pd

CSV = "../../app18-VaccineSense-Coleta/dataset_gerado/vaccinesense_dataset.csv"
MODELO_ARQUIVO = "modelo_vaccinesense.pkl"

# As mesmas seis features, na mesma ordem do treinamento.
FEATURES = [
    "tempInterna",
    "tempExterna",
    "umidade",
    "luz",
    "criticidade",
    "distancia",
]


def montar_payload(linha):
    """Monta o JSON como o ESP32 publica em fiap/iot/vaccinesense."""
    return json.dumps({
        "device": linha["device"],
        "rodada": int(linha["rodada"]),
        "id": int(linha["id"]),
        "tempInterna": float(linha["tempInterna"]),
        "tempExterna": float(linha["tempExterna"]),
        "umidade": float(linha["umidade"]),
        "luz": int(linha["luz"]),
        "criticidade": int(linha["criticidade"]),
        "distancia": float(linha["distancia"]),
        "tempoForaDaFaixa": int(linha["tempoForaDaFaixa"]),
    })


def classificar(modelo, payload_json):
    """O mesmo caminho da aplicação: JSON -> variáveis -> DataFrame -> predict."""
    p = json.loads(payload_json)

    # PASSO 1 - pegar as medições
    temp_interna = float(p["tempInterna"])
    temp_externa = float(p["tempExterna"])
    umidade      = float(p["umidade"])
    luz          = int(p["luz"])
    criticidade  = int(p["criticidade"])
    distancia    = float(p["distancia"])

    # PASSO 2 - montar a entrada com os nomes do treinamento
    medicao = pd.DataFrame([{
        "tempInterna": temp_interna,
        "tempExterna": temp_externa,
        "umidade":     umidade,
        "luz":         luz,
        "criticidade": criticidade,
        "distancia":   distancia,
    }])[FEATURES]

    # PASSO 3 - prever
    return int(modelo.predict(medicao)[0])


def main():
    print("Validação do modelo Vaccine Sense")
    print("=" * 64)

    try:
        modelo = joblib.load(MODELO_ARQUIVO)
    except FileNotFoundError:
        print(f"ERRO: {MODELO_ARQUIVO} não encontrado.")
        print("Gere o modelo no notebook do Colab e salve nesta pasta.")
        sys.exit(1)

    df = pd.read_csv(CSV)
    df["alvo"] = (df["situacao"].str.upper() == "CARGA_EM_PERIGO").astype(int)
    print(f"{len(df)} medições no CSV, {df.rodada.nunique()} rodadas\n")

    # ---- uma medição, mostrada por inteiro -----------------------------
    exemplo = df[df.alvo == 1].iloc[0]
    payload = montar_payload(exemplo)

    print("Exemplo de payload publicado pelo ESP32:")
    print(f"  {payload}\n")

    resultado = classificar(modelo, payload)
    esperado = int(exemplo["alvo"])
    print(f"  modelo respondeu: {'CARGA_EM_PERIGO' if resultado else 'TRANSPORTE_OK'}")
    print(f"  rótulo esperado:  {'CARGA_EM_PERIGO' if esperado else 'TRANSPORTE_OK'}")
    print(f"  {'confere' if resultado == esperado else 'DIVERGE'}\n")

    # ---- todas as medições, uma por uma --------------------------------
    print("-" * 64)
    print("Passando o CSV inteiro pelo caminho JSON -> predict...\n")

    acertos = 0
    erros_por_rodada = {}

    for _, linha in df.iterrows():
        previsto = classificar(modelo, montar_payload(linha))
        if previsto == int(linha["alvo"]):
            acertos += 1
        else:
            r = int(linha["rodada"])
            erros_por_rodada[r] = erros_por_rodada.get(r, 0) + 1

    print(f"acertos: {acertos} de {len(df)}  ({acertos / len(df) * 100:.1f} %)\n")

    if erros_por_rodada:
        print("Onde errou:")
        for r in sorted(erros_por_rodada):
            g = df[df.rodada == r]
            print(f"  rodada {r:2d} ({g.situacao.iloc[0]:15s}): "
                  f"{erros_por_rodada[r]:3d} de {len(g)} medições   "
                  f"tempInterna {g.tempInterna.min():.1f}–{g.tempInterna.max():.1f} °C, "
                  f"criticidade {g.criticidade.mean():.0f}")
        print()
        print("Erro concentrado nas rodadas de fronteira é o esperado:")
        print("ali o limite físico é uma região, não uma linha.")
    else:
        print("Nenhum erro. Desconfie: verifique se o modelo não viu estes")
        print("dados no treinamento.")


if __name__ == "__main__":
    main()
