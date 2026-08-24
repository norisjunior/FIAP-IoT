#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Vaccine Sense - Classificação em tempo real
Lê a última medição da caixa no InfluxDB e classifica com o modelo treinado.
"""

import pandas as pd
import joblib
import time

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

# As cinco medições que o modelo recebe.
# A ordem e os nomes precisam ser EXATAMENTE os do treinamento.
FEATURES = [
    "tempInterna",
    "tempExterna",
    "luz",
    "criticidade",
    "distancia",
]

# ===== INICIALIZAÇÃO =====
print("🚑 Vaccine Sense - classificação da condição da carga")
print("=" * 60)

print("📦 Carregando modelo...")
try:
    modelo = joblib.load(MODELO_ARQUIVO)
    print(f"✅ Modelo carregado: {MODELO_ARQUIVO}")
except Exception as e:
    print(f"❌ ERRO: não consegui carregar {MODELO_ARQUIVO}")
    print(f"   {e}")
    print("   Gere o modelo no notebook do Colab e salve nesta pasta.")
    exit(1)

print("🌐 Conectando ao InfluxDB...")
try:
    client = InfluxDBClient3(
        host=INFLUX_URL,
        token=INFLUX_TOKEN,
        database=INFLUX_BUCKET,
    )
    print("✅ Conectado ao InfluxDB")
except Exception as e:
    print(f"❌ ERRO de conexão: {e}")
    print("\n🔧 Verifique:")
    print("   1. O token, a organização e o bucket")
    print("   2. A conexão com a internet")
    exit(1)

ultimo_timestamp = None

print(f"🔄 Monitorando o dispositivo: {DISPOSITIVO}")
print(f"⏰ Consultando a cada {INTERVALO} segundos\n")

# ===== LOOP PRINCIPAL =====
while True:
    try:
        print("🔍 Consultando...", end=" ")

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

        try:
            tabela = client.query(query=query, language="sql")
            df = tabela.to_pandas()
        except Exception as e:
            print(f"❌ Erro na consulta: {e}")
            time.sleep(INTERVALO)
            continue

        if df.empty:
            print("📭 Nenhuma medição encontrada")
            time.sleep(INTERVALO)
            continue

        timestamp_atual = pd.to_datetime(df.iloc[0]["time"], utc=True)

        if ultimo_timestamp is not None and timestamp_atual <= ultimo_timestamp:
            print("⏭️  Medição repetida")
            time.sleep(INTERVALO)
            continue

        print("🆕 Medição nova!")
        ultimo_timestamp = timestamp_atual

        # ------------------------------------------------------------------
        # PASSO 1 - PEGAR as medições que vieram do dispositivo
        # ------------------------------------------------------------------
        # Cada campo do JSON publicado pelo ESP32 virou uma coluna do
        # DataFrame. Aqui separamos cada uma em uma variável, para ficar
        # explícito o que o dispositivo mediu.
        dados = df.iloc[0]

        temp_interna = float(dados["tempInterna"])
        temp_externa = float(dados["tempExterna"])
        umidade      = float(dados["umidade"])
        luz          = int(dados["luz"])
        criticidade  = int(dados["criticidade"])
        distancia    = float(dados["distancia"])

        print(f"\n📊 Medições da caixa ({timestamp_atual.strftime('%H:%M:%S')}):")
        print(f"   🌡️  Temperatura interna: {temp_interna:.1f} °C")
        print(f"   🌤️  Temperatura externa: {temp_externa:.1f} °C")
        print(f"   💧 Umidade externa: {umidade:.1f} %")
        print(f"   💡 Luz dentro da caixa: {luz}")
        print(f"   📏 Distância até a carga: {distancia:.1f} cm")
        print(f"   🎚️  Criticidade da carga: {criticidade}")

        # ------------------------------------------------------------------
        # PASSO 2 - MONTAR a entrada do modelo
        # ------------------------------------------------------------------
        # O modelo foi treinado com um DataFrame que tinha estes nomes de
        # coluna, nesta ordem. Montamos um DataFrame de UMA linha com os
        # mesmos nomes: assim o scikit-learn casa cada valor com a coluna
        # certa, e não é preciso confiar na ordem.
        medicao = pd.DataFrame([{
            "tempInterna": temp_interna,
            "tempExterna": temp_externa,
            "luz":         luz,
            "criticidade": criticidade,
            "distancia":   distancia,
        }])[FEATURES]

        # ------------------------------------------------------------------
        # PASSO 3 - PREVER
        # ------------------------------------------------------------------
        predicao = modelo.predict(medicao)[0]

        print("\n🤖 Modelo:")
        if predicao == 1:
            print("   🔴 CARGA EM PERIGO")
        else:
            print("   🟢 TRANSPORTE OK")
        print()

        time.sleep(INTERVALO)

    except KeyboardInterrupt:
        print("\n\n🛑 Interrompido pelo usuário")
        break
    except Exception as e:
        print(f"❌ Erro: {e}")
        time.sleep(INTERVALO)

print("👋 Fim do monitoramento")
