# ================================================================
# ML + InfluxDB - Versão Didática Simples
# Demonstra: Conexão InfluxDB → Carregar Modelo → Consulta → Predição
# ================================================================

# Célula 1: Imports
import pandas as pd
import joblib
from influxdb_client_3 import InfluxDBClient3

# ================================================================
# Célula 2: Configurações
# ================================================================

# InfluxDB
INFLUX_URL = "https://us-east-1-1.aws.cloud2.influxdata.com"
INFLUX_TOKEN = "KXTPf0peaYQU-QMGu-yJNWwVbBLNoUMmNwBBsrfcnK5GseDHLs_QZx7hNW4sToLnp1qeEXu5CwUq6rwf30FcXQ=="
INFLUX_BUCKET = "IoTSensores"
TABELA_NOME = "ML_occupancy_sala_FIAP"

# Modelo e dispositivo
MODELO_ARQUIVO = "modelo_ocupacao_best_xgbclassifier.pkl"
DISPOSITIVO = "FIAP_IoT_app19_001"

# ================================================================
# Célula 3: Carregar Modelo ML
# ================================================================

print("1. Carregando modelo ML...")
modelo = joblib.load(MODELO_ARQUIVO)
print(f"   Modelo carregado: {MODELO_ARQUIVO}")

# ================================================================
# Célula 4: Conectar ao InfluxDB
# ================================================================

print("\n2. Conectando ao InfluxDB...")
client = InfluxDBClient3(
    host=INFLUX_URL,
    token=INFLUX_TOKEN,
    database=INFLUX_BUCKET,
    verify_ssl=False
)
print("   Conectado ao InfluxDB!")

# ================================================================
# Célula 5: Consultar Dados
# ================================================================

print("\n3. Consultando dados dos sensores...")

# Query SQL para buscar último registro
query = f"""
SELECT
  "Temperature",
  "Humidity", 
  "Light",
  "CO2",
  "HumidityRatio"
FROM "{TABELA_NOME}"
WHERE "dispositivo" = '{DISPOSITIVO}'
ORDER BY time DESC
LIMIT 1
"""

# Executar consulta
resultado = client.query(query=query, language="sql")
df = resultado.to_pandas()

print(f"   Dados encontrados: {len(df)} registro(s)")

# ================================================================
# Célula 6: Preparar Dados para o Modelo
# ================================================================

print("\n4. Preparando dados para o modelo...")

# Extrair dados do DataFrame
dados = df.iloc[0]  # Primeiro (e único) registro

print("   Dados dos sensores:")
print(f"   - Temperature: {dados['Temperature']:.1f} °C")
print(f"   - Humidity: {dados['Humidity']:.1f} %")
print(f"   - Light: {dados['Light']:.1f} Lux")
print(f"   - CO2: {dados['CO2']:.1f} ppm")
print(f"   - HumidityRatio: {dados['HumidityRatio']:.6f}")

# Preparar features na ordem que o modelo espera
features = [[
    dados['Temperature'],
    dados['Humidity'],
    dados['Light'],
    dados['CO2'],
    dados['HumidityRatio']
]]

print(f"   Features preparadas: {features}")

# ================================================================
# Célula 7: Fazer Predição
# ================================================================

print("\n5. Aplicando modelo ML...")

# Fazer predição
predicao = modelo.predict(features)[0]
probabilidades = modelo.predict_proba(features)[0]

# Interpretar resultado
status = "OCUPADA" if predicao == 1 else "VAZIA"
confianca = max(probabilidades) * 100

print(f"   Predição: {predicao} ({status})")
print(f"   Confiança: {confianca:.1f}%")
print(f"   Probabilidades:")
print(f"   - Vazia: {probabilidades[0]*100:.1f}%")
print(f"   - Ocupada: {probabilidades[1]*100:.1f}%")

# ================================================================
# Célula 8: Resultado Final
# ================================================================

print("\n" + "="*50)
print("RESULTADO FINAL:")
print(f"SALA {status} (Confiança: {confianca:.1f}%)")
print("="*50)