#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Monitor de Ocupação de Sala - InfluxDB + ML
Script didático para detectar ocupação usando modelo ML
"""

import pandas as pd
import joblib
import time
import math
import warnings
warnings.filterwarnings('ignore')  # Suprime avisos de versão sklearn

try:
    from influxdb_client_3 import InfluxDBClient3
except ImportError:
    print("❌ Erro: influxdb3-python não encontrado!")
    print("Execute: pip install influxdb3-python")
    exit(1)

# ===== CONFIGURAÇÕES =====
INFLUX_URL = "https://us-east-1-1.aws.cloud2.influxdata.com"
INFLUX_TOKEN = "KXTPf0peaYQU-QMGu-yJNWwVbBLNoUMmNwBBsrfcnK5GseDHLs_QZx7hNW4sToLnp1qeEXu5CwUq6rwf30FcXQ=="
INFLUX_ORG = "e044ac59f07be199"
INFLUX_BUCKET = "IoTSensores"

MODELO_ARQUIVO = "modelo_ocupacao_best_xgbclassifier.pkl"
MEASUREMENT = "ML_occupancy_sala_FIAP"
DISPOSITIVO = "FIAP_IoT_app19_Noris"  # Conforme mostrado na imagem
INTERVALO = 10  # segundos

# ===== INICIALIZAÇÃO =====
print("🚀 Monitor de Ocupação de Sala usando ML em dados IoT")
print("="*60)

# Carrega o modelo treinado
print("📦 Carregando modelo...")
try:
    modelo = joblib.load(MODELO_ARQUIVO)
    print(f"✅ Modelo carregado: {MODELO_ARQUIVO}")
except:
    print(f"❌ ERRO: Arquivo {MODELO_ARQUIVO} não encontrado!")
    exit()

# Conecta ao InfluxDB usando Client3 (SQL)
print("🌐 Conectando ao InfluxDB...")
try:
    client = InfluxDBClient3(
        host=INFLUX_URL,
        token=INFLUX_TOKEN,
        database=INFLUX_BUCKET,
        verify_ssl=False  # Desabilita verificação SSL para evitar problemas de certificado (apenas para avaliação, não usar em produção)
    )
    
    # Teste de conectividade simples
    test_query = "SELECT COUNT(*) FROM sensores_iot LIMIT 1"
    result = client.query(query=test_query, language="sql")
    print("✅ Conectado ao InfluxDB")
    
except Exception as e:
    print(f"❌ ERRO de conexão InfluxDB: {e}")
    print("\n🔧 Soluções possíveis:")
    print("1. Verifique sua conexão com internet")
    print("2. Confirme se o token está correto")
    print("3. Tente executar novamente em alguns minutos")
    print("4. Verifique se o firewall não está bloqueando")
    exit(1)

# Controle para não reprocessar o mesmo dado
ultimo_timestamp = None

print(f"🔄 Monitorando dispositivo: {DISPOSITIVO}")
print(f"⏰ Verificando a cada {INTERVALO} segundos\n")

# ===== LOOP PRINCIPAL =====
while True:
    try:
        print("🔍 Consultando dados...", end=" ")
        
        # Query SQL para buscar os últimos dados
        query = f"""
        SELECT
          "time",
          "dispositivo", 
          "Temperature",
          "Humidity",
          "Light",
          "CO2",
          "HumidityRatio"
        FROM "{MEASUREMENT}"
        WHERE time >= now() - interval '10 minutes'
          AND "dispositivo" = '{DISPOSITIVO}'
        ORDER BY time DESC
        LIMIT 1
        """
        
        # Executa a consulta SQL
        try:
            table = client.query(query=query, language="sql")
            df = table.to_pandas()
        except Exception as e:
            print(f"❌ Erro na consulta: {e}")
            time.sleep(INTERVALO)
            continue
        
        # Verifica se encontrou dados
        if df.empty:
            print("📭 Nenhum dado encontrado")
        else:
            # Pega os dados mais recentes
            dados = df.iloc[0]
            timestamp_atual = pd.to_datetime(dados['time'], utc=True)
            
            # Verifica se é um dado novo
            if ultimo_timestamp is not None and timestamp_atual <= ultimo_timestamp:
                print("⏭️  Mesmo dado anterior")
            else:
                print("🆕 Novo dado encontrado!")
                ultimo_timestamp = timestamp_atual
                
                # Mostra os dados recebidos
                print(f"\n📊 Dados dos Sensores ({timestamp_atual.strftime('%H:%M:%S')}):")
                print(f"   🌡️  Temperature: {dados['Temperature']:.1f} °C")
                print(f"   💧 Humidity: {dados['Humidity']:.1f} %")
                print(f"   💡 Light: {dados['Light']:.1f} Lux")
                print(f"   🌫️  CO2: {dados['CO2']:.1f} ppm")
                print(f"   💨 HumidityRatio: {dados['HumidityRatio']:.6f}")
                
                # Prepara dados para o modelo (na ordem que o modelo espera)
                features = [[
                    dados['Temperature'],
                    dados['Humidity'],
                    dados['Light'],
                    dados['CO2'],
                    dados['HumidityRatio']
                ]]
                
                # Aplica o modelo
                predicao = modelo.predict(features)[0]
                probabilidades = modelo.predict_proba(features)[0]
                
                # Mostra o resultado
                print(f"\n🤖 Resultado do Modelo:")
                if predicao == 1:
                    print(f"   🔴 SALA OCUPADA")
                else:
                    print(f"   🟢 SALA VAZIA")
                
                print(f"   📈 Confiança: {max(probabilidades)*100:.1f}%")
                print(f"   📊 Probabilidades:")
                print(f"      • Vazia: {probabilidades[0]*100:.1f}%")
                print(f"      • Ocupada: {probabilidades[1]*100:.1f}%")
                print()
        
        # Aguarda próxima verificação
        time.sleep(INTERVALO)
        
    except KeyboardInterrupt:
        print("\n\n🛑 Interrompido pelo usuário")
        break
    except Exception as e:
        print(f"❌ Erro: {e}")
        time.sleep(INTERVALO)

print("👋 Fim do monitoramento")