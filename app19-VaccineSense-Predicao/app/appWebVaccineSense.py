#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Vaccine Sense - Classificação em tempo real (versão web)
Mesma lógica da versão console, apresentada em uma página que atualiza sozinha.
"""

import threading
import time

import joblib
import pandas as pd
from flask import Flask, render_template_string

try:
    from influxdb_client_3 import InfluxDBClient3
except ImportError:
    print("❌ Erro: influxdb3-python não encontrado!")
    print("Execute: pip install -r requirements.txt")
    exit(1)

# ===== CONFIGURAÇÕES =====
INFLUX_URL = "https://us-east-1-1.aws.cloud2.influxdata.com"
INFLUX_TOKEN = "XXXXX"
INFLUX_ORG = "XXXXX"
INFLUX_BUCKET = "XXXXX"

MEASUREMENT = "vaccinesense_raw_2026"
DISPOSITIVO = "ESP32Noris001Vaccine"
MODELO_ARQUIVO = "modelo_vaccinesense.pkl"
INTERVALO = 5  # segundos entre consultas

# As cinco medições que o modelo recebe, na ordem do treinamento.
FEATURES = [
    "tempInterna",
    "tempExterna",
    "luz",
    "criticidade",
    "distancia",
]

# ===== ESTADO COMPARTILHADO =====
# O laço de fundo escreve aqui; a página web lê.
estado = {
    "horario": "--:--:--",
    "medicoes": None,
    "predicao": None,
    "mensagem": "Aguardando a primeira medição...",
}

app = Flask(__name__)


def monitorar():
    """Consulta o InfluxDB, classifica e atualiza o estado compartilhado."""
    print("📦 Carregando modelo...")
    modelo = joblib.load(MODELO_ARQUIVO)
    print(f"✅ Modelo carregado: {MODELO_ARQUIVO}")

    print("🌐 Conectando ao InfluxDB...")
    client = InfluxDBClient3(
        host=INFLUX_URL,
        token=INFLUX_TOKEN,
        database=INFLUX_BUCKET,
    )
    print("✅ Conectado ao InfluxDB")

    ultimo_timestamp = None

    while True:
        try:
            query = f"""
            SELECT
              "time",
              "device",
              "tempInterna",
              "tempExterna",
              "umidade",
              "luz",
              "criticidade",
              "distancia"
            FROM "{MEASUREMENT}"
            WHERE time >= now() - interval '10 minutes'
              AND "device" = '{DISPOSITIVO}'
            ORDER BY time DESC
            LIMIT 1
            """

            df = client.query(query=query, language="sql").to_pandas()

            if df.empty:
                estado["mensagem"] = "Nenhuma medição nos últimos 10 minutos."
                time.sleep(INTERVALO)
                continue

            timestamp_atual = pd.to_datetime(df.iloc[0]["time"], utc=True)

            if ultimo_timestamp is not None and timestamp_atual <= ultimo_timestamp:
                time.sleep(INTERVALO)
                continue

            ultimo_timestamp = timestamp_atual

            # PASSO 1 - PEGAR as medições que vieram do dispositivo
            dados = df.iloc[0]

            temp_interna = float(dados["tempInterna"])
            temp_externa = float(dados["tempExterna"])
            umidade      = float(dados["umidade"])
            luz          = int(dados["luz"])
            criticidade  = int(dados["criticidade"])
            distancia    = float(dados["distancia"])

            # PASSO 2 - MONTAR a entrada do modelo, com os mesmos nomes
            # de coluna usados no treinamento
            medicao = pd.DataFrame([{
                "tempInterna": temp_interna,
                "tempExterna": temp_externa,
                "luz":         luz,
                "criticidade": criticidade,
                "distancia":   distancia,
            }])[FEATURES]

            # PASSO 3 - PREVER
            predicao = modelo.predict(medicao)[0]

            estado.update({
                "horario": timestamp_atual.strftime("%H:%M:%S"),
                "medicoes": {
                    "Temperatura interna": f"{temp_interna:.1f} °C",
                    "Temperatura externa": f"{temp_externa:.1f} °C",
                    "Umidade externa": f"{umidade:.1f} %",
                    "Luz dentro da caixa": f"{luz}",
                    "Distância até a carga": f"{distancia:.1f} cm",
                    "Criticidade da carga": f"{criticidade}",
                },
                "predicao": int(predicao),
                "mensagem": "",
            })

            print(f"[{estado['horario']}] "
                  f"{'CARGA EM PERIGO' if predicao == 1 else 'TRANSPORTE OK'}")

        except Exception as e:
            estado["mensagem"] = f"Erro: {e}"
            print(f"❌ {e}")

        time.sleep(INTERVALO)


PAGINA = """
<!doctype html>
<html lang="pt-br">
<head>
  <meta charset="utf-8">
  <meta http-equiv="refresh" content="5">
  <title>Vaccine Sense</title>
  <style>
    body { font-family: system-ui, sans-serif; background: #eef2f5;
           color: #16191d; margin: 0; padding: 2rem; }
    .caixa { max-width: 620px; margin: 0 auto; background: #fff;
             border-radius: 10px; padding: 1.5rem 2rem; }
    h1 { margin: 0 0 .3rem; font-size: 1.4rem; }
    .horario { color: #6b757e; font-size: .9rem; margin-bottom: 1.4rem; }
    table { width: 100%; border-collapse: collapse; margin-bottom: 1.5rem; }
    td { padding: .5rem 0; border-bottom: 1px solid #e6eaee; }
    td.valor { text-align: right; font-variant-numeric: tabular-nums;
               font-weight: 600; }
    .resultado { padding: 1rem 1.2rem; border-radius: 8px; font-size: 1.15rem;
                 font-weight: 700; margin-bottom: .6rem; }
    .ok     { background: #e2f4e8; color: #1c6b38; }
    .perigo { background: #fbe3dc; color: #a5330f; }
    .rotulo { font-size: .75rem; letter-spacing: .1em; text-transform: uppercase;
              color: #6b757e; margin: 1.2rem 0 .4rem; }
    .aviso { background: #fdf3d8; color: #8a6100; padding: 1rem;
             border-radius: 8px; }
  </style>
</head>
<body>
  <div class="caixa">
    <h1>🚑 Vaccine Sense</h1>
    <div class="horario">Última medição: {{ e.horario }}</div>

    {% if e.mensagem %}
      <div class="aviso">{{ e.mensagem }}</div>
    {% endif %}

    {% if e.medicoes %}
      <div class="rotulo">Medições da caixa</div>
      <table>
        {% for nome, valor in e.medicoes.items() %}
          <tr><td>{{ nome }}</td><td class="valor">{{ valor }}</td></tr>
        {% endfor %}
      </table>

      <div class="rotulo">Modelo</div>
      {% if e.predicao == 1 %}
        <div class="resultado perigo">
          🔴 CARGA EM PERIGO
        </div>
      {% else %}
        <div class="resultado ok">
          🟢 TRANSPORTE OK
        </div>
      {% endif %}
    {% endif %}
  </div>
</body>
</html>
"""


@app.route("/")
def pagina():
    return render_template_string(PAGINA, e=estado)


if __name__ == "__main__":
    print("🚑 Vaccine Sense - versão web")
    print("=" * 60)

    threading.Thread(target=monitorar, daemon=True).start()

    print("🌐 Acesse: http://localhost:5000")
    app.run(host="0.0.0.0", port=5000, debug=False)
