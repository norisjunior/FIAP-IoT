# montar_dataset_fiap.py: monta o dataset_detector_fumaca_fiap.csv (o que o aluno recebe)
# a partir das coletas brutas do professor (coleta_professor/).
#
# Uso, nesta pasta:  python montar_dataset_fiap.py
#
# O preparo e o mesmo do treinamento_fumaca.ipynb:
# - sai a rodada 1 (o aquecimento da caixa);
# - saem os primeiros 30 s de cada estimulo e os primeiros 120 s de cada trecho de ar
#   (o rotulo e o aperto do BOOT, mas a curva so muda 15 a 30 s depois);
# - ficam so as 7 colunas de sensor que os dois firmwares medem igual;
# - o alvo e o incendio: fire (fumaca ou chama) ou not fire (ar, alcool, vapor).
import os
import pandas as pd

AQUI = os.path.dirname(os.path.abspath(__file__))
APP16 = os.path.join(AQUI, "..", "..")

# Coletas brutas: (arquivo, nome da coleta)
COLETAS = [
    (os.path.join(APP16, "app16-2-TesteFenomenos", "coleta_professor", "coleta-20261111-1848-dataset.csv"),
     "2026-11-11 18:48"),
    (os.path.join(APP16, "app16-3-Fusao1Hz", "coleta_professor", "coleta-20261111-2207.csv"),
     "2026-11-11 22:07"),
]
LOG_1848 = os.path.join(APP16, "app16-2-TesteFenomenos", "coleta_professor", "coleta-20261111-1848.csv")

FEATURES = ["temp", "umid", "raw_h2", "raw_ethanol", "pm2_5", "pm10", "nc0_5"]
NOMES = {0: "ar", 1: "álcool", 2: "vapor", 3: "fumaça", 4: "chama"}
FOGO = [3, 4]   # fumaca e chama
SAIDA = os.path.join(AQUI, "dataset_detector_fumaca_fiap.csv")


# Converte o log do teste dos fenomenos (">var: valor") no formato do coletor do dataset
def converte_log(log, saida):
    linhas = []
    atual = {}
    for t in open(log, encoding="utf-8", errors="replace"):
        t = t.strip()
        if not t.startswith(">") or ":" not in t:
            continue
        nome, valor = t[1:].split(":", 1)
        try:
            atual[nome] = float(valor)
        except ValueError:
            continue
        if nome == "estimulo":   # a ultima variavel de cada segundo
            linhas.append(atual)
            atual = {}
    d = pd.DataFrame(linhas)
    d["amostra"] = range(1, len(d) + 1)
    d["rodada"] = (d["estimulo"] != d["estimulo"].shift()).cumsum()   # +1 a cada aperto do BOOT
    # tvoc e eco2 saem: sem a correcao de umidade do SGP30, nao se comparam com o coletor do dataset
    colunas = ["amostra", "rodada", "temp", "umid", "raw_h2", "raw_ethanol", "pm2_5", "nc0_5", "pm10", "estimulo"]
    d = d[colunas]
    for c in ["amostra", "rodada", "raw_h2", "raw_ethanol", "estimulo"]:
        d[c] = d[c].astype(int)
    d.to_csv(saida, index=False)
    print(f"convertido: {os.path.basename(saida)} ({len(d)} linhas)")


# Preparo de uma coleta: o mesmo do treinamento_fumaca.ipynb
def prepara(d, coleta):
    d = d.sort_values("amostra").copy()
    mudou = (d["rodada"] != d["rodada"].shift()) | (d["estimulo"] != d["estimulo"].shift())
    d["seg"] = d.groupby(mudou.cumsum()).cumcount()   # segundos desde o aperto do BOOT
    descarta = d["estimulo"].map(lambda e: 120 if e == 0 else 30)
    d = d[(d["rodada"] > 1) & (d["seg"] >= descarta)].copy()
    d = d.dropna(subset=FEATURES)

    saida = pd.DataFrame()
    saida["coleta"] = [coleta] * len(d)
    saida["estimulo"] = d["estimulo"].astype(int).map(NOMES).values
    for f in FEATURES:
        saida[f] = d[f].values
    incendio = []
    for e in d["estimulo"].astype(int):
        if e in FOGO:
            incendio.append("fire")
        else:
            incendio.append("not fire")
    saida["incendio"] = incendio
    return saida


if not os.path.exists(COLETAS[0][0]):
    converte_log(LOG_1848, COLETAS[0][0])

partes = []
for arquivo, coleta in COLETAS:
    bruto = pd.read_csv(arquivo)
    pronto = prepara(bruto, coleta)
    print(f"{coleta}: {len(bruto)} linhas brutas -> {len(pronto)} no dataset")
    partes.append(pronto)

dataset = pd.concat(partes, ignore_index=True)
dataset.to_csv(SAIDA, index=False, encoding="utf-8")
print(f"\n{os.path.basename(SAIDA)}: {len(dataset)} linhas, colunas {list(dataset.columns)}")
print(pd.crosstab([dataset["coleta"], dataset["estimulo"]], dataset["incendio"]))
