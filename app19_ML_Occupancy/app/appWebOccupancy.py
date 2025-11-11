#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Flask Monitor Console - Monitor de Ocupação Estilo Console
Imita o comportamento do script Python original com saída visual
"""

from flask import Flask, render_template_string
import pandas as pd
import joblib
import time
import warnings
from datetime import datetime
import threading
import logging

warnings.filterwarnings('ignore')

try:
    from influxdb_client_3 import InfluxDBClient3
except ImportError:
    print("Erro: influxdb3-python não encontrado!")
    exit(1)

# Configurações
INFLUX_URL = "https://us-east-1-1.aws.cloud2.influxdata.com"
INFLUX_TOKEN = "KXTPf0peaYQU-QMGu-yJNWwVbBLNoUMmNwBBsrfcnK5GseDHLs_QZx7hNW4sToLnp1qeEXu5CwUq6rwf30FcXQ=="
INFLUX_ORG = "e044ac59f07be199"
INFLUX_BUCKET = "IoTSensores"
MODELO_ARQUIVO = "modelo_ocupacao_best_xgbclassifier.pkl"
DISPOSITIVO = "Noris_ESP32_Aula13"
INTERVALO = 10  # segundos

# Inicializa Flask
app = Flask(__name__)

# Configuração de logging
logging.basicConfig(level=logging.INFO)

# Variáveis globais
modelo = None
client = None
TABELA_NOME = "ML_occupancy"
ultimo_timestamp = None
mensagens_log = []
dados_atuais = None
predicao_atual = None

def adicionar_log(mensagem):
    """Adiciona mensagem ao log com timestamp"""
    global mensagens_log
    timestamp = datetime.now().strftime("%H:%M:%S")
    mensagens_log.append(f"[{timestamp}] {mensagem}")
    
    # Mantém apenas as últimas 50 mensagens
    if len(mensagens_log) > 50:
        mensagens_log = mensagens_log[-50:]
    
    print(f"[{timestamp}] {mensagem}")

def inicializar_sistema():
    """Inicializa o modelo ML e conexão InfluxDB"""
    global modelo, client, TABELA_NOME
    
    try:
        adicionar_log("📦 Carregando modelo ML...")
        modelo = joblib.load(MODELO_ARQUIVO)
        adicionar_log(f"✅ Modelo carregado: {MODELO_ARQUIVO}")
        
        adicionar_log("🌐 Conectando ao InfluxDB...")
        client = InfluxDBClient3(
            host=INFLUX_URL,
            token=INFLUX_TOKEN,
            database=INFLUX_BUCKET,
            timeout=30,
            verify_ssl=False
        )
        
        # Detecta nome correto da tabela
        table_names = ["ML_occupancy", "ml_occupancy", '"ML_occupancy"']
        
        for table_name in table_names:
            try:
                test_query = f"SELECT COUNT(*) FROM {table_name} LIMIT 1"
                result = client.query(query=test_query, language="sql")
                adicionar_log(f"✅ Conectado ao InfluxDB - Tabela: {table_name}")
                TABELA_NOME = table_name
                return True
            except:
                continue
        
        adicionar_log("❌ Nenhuma tabela encontrada")
        return False
        
    except Exception as e:
        adicionar_log(f"❌ Erro na inicialização: {e}")
        return False

def consultar_dados_recentes():
    """Consulta dados mais recentes do InfluxDB"""
    try:
        query = f"""
        SELECT
          "time",
          "dispositivo", 
          "Temperature",
          "Humidity",
          "Light",
          "CO2",
          "HumidityRatio"
        FROM {TABELA_NOME}
        WHERE time >= now() - interval '10 minutes'
          AND "dispositivo" = '{DISPOSITIVO}'
        ORDER BY time DESC
        LIMIT 1
        """
        
        table = client.query(query=query, language="sql")
        df = table.to_pandas()
        
        if df.empty:
            return None
            
        dados = df.iloc[0]
        timestamp = pd.to_datetime(dados['time'], utc=True)
        
        return {
            'timestamp': timestamp,
            'dispositivo': dados['dispositivo'],
            'Temperature': float(dados['Temperature']),
            'Humidity': float(dados['Humidity']),
            'Light': float(dados['Light']),
            'CO2': float(dados['CO2']),
            'HumidityRatio': float(dados['HumidityRatio'])
        }
        
    except Exception as e:
        adicionar_log(f"❌ Erro na consulta: {e}")
        return None

def aplicar_modelo(dados_sensor):
    """Aplica modelo ML nos dados dos sensores"""
    try:
        features = [[
            dados_sensor['Temperature'],
            dados_sensor['Humidity'],
            dados_sensor['Light'],
            dados_sensor['CO2'],
            dados_sensor['HumidityRatio']
        ]]
        
        predicao = modelo.predict(features)[0]
        probabilidades = modelo.predict_proba(features)[0]
        
        return {
            'ocupado': bool(predicao),
            'status': 'OCUPADA' if predicao == 1 else 'VAZIA',
            'emoji': '🔴' if predicao == 1 else '🟢',
            'confianca': float(max(probabilidades)),
            'probabilidade_vazia': float(probabilidades[0]),
            'probabilidade_ocupada': float(probabilidades[1])
        }
        
    except Exception as e:
        adicionar_log(f"❌ Erro ao aplicar modelo: {e}")
        return None

def monitor_loop():
    """Loop principal de monitoramento (roda em thread separada)"""
    global ultimo_timestamp, dados_atuais, predicao_atual
    
    adicionar_log(f"🔄 Monitorando dispositivo: {DISPOSITIVO}")
    adicionar_log(f"⏰ Verificando a cada {INTERVALO} segundos")
    
    while True:
        try:
            # Consulta dados recentes
            dados = consultar_dados_recentes()
            
            if dados is None:
                adicionar_log("🔍 Consultando dados... 📭 Nenhum dado encontrado")
            elif ultimo_timestamp is not None and dados['timestamp'] <= ultimo_timestamp:
                adicionar_log("🔍 Consultando dados... ⏭️  Mesmo dado anterior")
            else:
                # Dados novos encontrados!
                adicionar_log("🔍 Consultando dados... 🆕 Novo dado encontrado!")
                ultimo_timestamp = dados['timestamp']
                dados_atuais = dados
                
                # Exibe dados dos sensores
                timestamp_str = dados['timestamp'].strftime('%H:%M:%S')
                adicionar_log(f"📊 Dados dos Sensores ({timestamp_str}):")
                adicionar_log(f"   🌡️  Temperature: {dados['Temperature']:.1f} °C")
                adicionar_log(f"   💧 Humidity: {dados['Humidity']:.1f} %")
                adicionar_log(f"   💡 Light: {dados['Light']:.1f} Lux")
                adicionar_log(f"   🌫️  CO2: {dados['CO2']:.1f} ppm")
                adicionar_log(f"   💨 HumidityRatio: {dados['HumidityRatio']:.6f}")
                
                # Aplica modelo ML
                predicao = aplicar_modelo(dados)
                predicao_atual = predicao
                
                if predicao:
                    adicionar_log("🤖 Resultado do Modelo:")
                    adicionar_log(f"   {predicao['emoji']} SALA {predicao['status']}")
                    adicionar_log(f"   📈 Confiança: {predicao['confianca']*100:.1f}%")
                    adicionar_log("   📊 Probabilidades:")
                    adicionar_log(f"      • Vazia: {predicao['probabilidade_vazia']*100:.1f}%")
                    adicionar_log(f"      • Ocupada: {predicao['probabilidade_ocupada']*100:.1f}%")
                    adicionar_log("")  # Linha em branco
            
            time.sleep(INTERVALO)
            
        except Exception as e:
            adicionar_log(f"❌ Erro no monitoramento: {e}")
            time.sleep(INTERVALO)

# Template HTML moderno para exibir os logs em tempo real
HTML_TEMPLATE = """
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Monitor de Ocupação IoT</title>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&family=JetBrains+Mono:wght@400;500&display=swap" rel="stylesheet">
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Inter', sans-serif;
            background: linear-gradient(135deg, #0f0f23 0%, #1a1a3e 100%);
            color: #ffffff;
            min-height: 100vh;
            overflow-x: hidden;
        }
        
        .container {
            max-width: 1400px;
            margin: 0 auto;
            padding: 20px;
            display: grid;
            grid-template-columns: 1fr 400px;
            gap: 30px;
            min-height: 100vh;
        }
        
        .main-content {
            display: flex;
            flex-direction: column;
            gap: 20px;
        }
        
        .header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            padding: 30px;
            border-radius: 20px;
            text-align: center;
            box-shadow: 0 10px 30px rgba(102, 126, 234, 0.3);
        }
        
        .header h1 {
            font-size: 28px;
            font-weight: 700;
            margin-bottom: 8px;
            background: linear-gradient(45deg, #ffffff, #e0e0e0);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
        }
        
        .header p {
            font-size: 16px;
            opacity: 0.9;
            font-weight: 300;
        }
        
        .console-container {
            background: #1e1e1e;
            border-radius: 15px;
            overflow: hidden;
            box-shadow: 0 10px 40px rgba(0, 0, 0, 0.3);
            border: 1px solid #333;
            flex: 1;
            display: flex;
            flex-direction: column;
        }
        
        .console-header {
            background: linear-gradient(90deg, #2d2d2d, #3a3a3a);
            padding: 15px 20px;
            border-bottom: 1px solid #444;
            display: flex;
            align-items: center;
            gap: 10px;
        }
        
        .console-dots {
            display: flex;
            gap: 8px;
        }
        
        .dot {
            width: 12px;
            height: 12px;
            border-radius: 50%;
        }
        
        .dot.red { background: #ff5f57; }
        .dot.yellow { background: #ffbd2e; }
        .dot.green { background: #28ca42; }
        
        .console-title {
            font-family: 'JetBrains Mono', monospace;
            font-size: 14px;
            color: #888;
            margin-left: 15px;
        }
        
        .console-log {
            background: #1a1a1a;
            padding: 20px;
            font-family: 'JetBrains Mono', monospace;
            font-size: 13px;
            line-height: 1.6;
            color: #e0e0e0;
            overflow-y: auto;
            flex: 1;
            max-height: 600px;
            scrollbar-width: thin;
            scrollbar-color: #444 #1a1a1a;
        }
        
        .console-log::-webkit-scrollbar {
            width: 8px;
        }
        
        .console-log::-webkit-scrollbar-track {
            background: #1a1a1a;
        }
        
        .console-log::-webkit-scrollbar-thumb {
            background: #444;
            border-radius: 4px;
        }
        
        .console-log::-webkit-scrollbar-thumb:hover {
            background: #555;
        }
        
        .log-entry {
            margin-bottom: 8px;
            padding: 2px 0;
            border-radius: 3px;
            transition: background-color 0.2s;
        }
        
        .log-entry:hover {
            background-color: rgba(255, 255, 255, 0.05);
        }
        
        .sidebar {
            display: flex;
            flex-direction: column;
            gap: 20px;
        }
        
        .status-card {
            background: linear-gradient(135deg, #2a2a2a, #3a3a3a);
            border-radius: 20px;
            padding: 25px;
            border: 1px solid #444;
            box-shadow: 0 10px 30px rgba(0, 0, 0, 0.2);
        }
        
        .status-indicator {
            text-align: center;
            margin-bottom: 20px;
        }
        
        .status-badge {
            display: inline-block;
            padding: 15px 30px;
            border-radius: 50px;
            font-size: 18px;
            font-weight: 600;
            margin-bottom: 15px;
            min-width: 200px;
            transition: all 0.3s ease;
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2);
        }
        
        .ocupada {
            background: linear-gradient(135deg, #ff6b6b, #ee5a24);
            color: white;
            box-shadow: 0 5px 25px rgba(255, 107, 107, 0.4);
        }
        
        .vazia {
            background: linear-gradient(135deg, #51cf66, #40c057);
            color: white;
            box-shadow: 0 5px 25px rgba(81, 207, 102, 0.4);
        }
        
        .confidence-bar {
            background: #2a2a2a;
            border-radius: 10px;
            height: 8px;
            overflow: hidden;
            margin: 10px 0;
        }
        
        .confidence-fill {
            height: 100%;
            background: linear-gradient(90deg, #667eea, #764ba2);
            border-radius: 10px;
            transition: width 0.5s ease;
        }
        
        .sensor-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 15px;
            margin-top: 20px;
        }
        
        .sensor-item {
            background: rgba(255, 255, 255, 0.05);
            padding: 15px;
            border-radius: 12px;
            border: 1px solid rgba(255, 255, 255, 0.1);
            text-align: center;
        }
        
        .sensor-icon {
            font-size: 24px;
            margin-bottom: 8px;
            display: block;
        }
        
        .sensor-label {
            font-size: 12px;
            color: #888;
            margin-bottom: 5px;
        }
        
        .sensor-value {
            font-size: 16px;
            font-weight: 600;
            color: #fff;
        }
        
        .probability-bars {
            margin-top: 20px;
        }
        
        .prob-item {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 10px;
        }
        
        .prob-bar {
            flex: 1;
            height: 6px;
            background: #2a2a2a;
            border-radius: 3px;
            margin: 0 15px;
            overflow: hidden;
        }
        
        .prob-fill {
            height: 100%;
            border-radius: 3px;
            transition: width 0.5s ease;
        }
        
        .prob-vazia { background: linear-gradient(90deg, #51cf66, #40c057); }
        .prob-ocupada { background: linear-gradient(90deg, #ff6b6b, #ee5a24); }
        
        .refresh-indicator {
            position: fixed;
            bottom: 20px;
            right: 20px;
            background: rgba(0, 0, 0, 0.8);
            color: white;
            padding: 10px 15px;
            border-radius: 25px;
            font-size: 12px;
            backdrop-filter: blur(10px);
            border: 1px solid rgba(255, 255, 255, 0.1);
        }
        
        .pulse {
            animation: pulse 2s infinite;
        }
        
        @keyframes pulse {
            0% { opacity: 1; }
            50% { opacity: 0.5; }
            100% { opacity: 1; }
        }
        
        @media (max-width: 1200px) {
            .container {
                grid-template-columns: 1fr;
                gap: 20px;
            }
            
            .sensor-grid {
                grid-template-columns: repeat(2, 1fr);
            }
        }
        
        @media (max-width: 768px) {
            .container {
                padding: 15px;
            }
            
            .header h1 {
                font-size: 24px;
            }
            
            .sensor-grid {
                grid-template-columns: 1fr;
            }
        }
    </style>
    <script>
        // Auto-refresh com indicador visual
        let countdown = 5;
        
        function updateCountdown() {
            const indicator = document.querySelector('.refresh-indicator');
            if (indicator) {
                indicator.textContent = `Atualizando em ${countdown}s`;
                countdown--;
                
                if (countdown < 0) {
                    indicator.textContent = 'Atualizando...';
                    location.reload();
                }
            }
        }
        
        // Scroll automático para o final do log
        function scrollToBottom() {
            const consoleLog = document.querySelector('.console-log');
            if (consoleLog) {
                consoleLog.scrollTop = consoleLog.scrollHeight;
            }
        }
        
        window.onload = function() {
            scrollToBottom();
            setInterval(updateCountdown, 1000);
        };
    </script>
</head>
<body>
    <div class="container">
        <div class="main-content">
            <div class="header">
                <h1>Monitor de Ocupação IoT</h1>
                <p>Sistema de detecção inteligente em tempo real</p>
            </div>
            
            <div class="console-container">
                <div class="console-header">
                    <div class="console-dots">
                        <div class="dot red"></div>
                        <div class="dot yellow"></div>
                        <div class="dot green"></div>
                    </div>
                    <div class="console-title">monitor_console.log</div>
                </div>
                
                <div class="console-log" id="console-log">
                    {% for log in mensagens_log %}
                    <div class="log-entry">{{ log }}</div>
                    {% endfor %}
                </div>
            </div>
        </div>
        
        <div class="sidebar">
            {% if dados_atuais and predicao_atual %}
            <div class="status-card">
                <div class="status-indicator">
                    <div class="status-badge {{ 'ocupada' if predicao_atual.ocupado else 'vazia' }}">
                        {{ predicao_atual.emoji }} SALA {{ predicao_atual.status }}
                    </div>
                    
                    <div style="font-size: 14px; color: #ccc; margin-bottom: 10px;">
                        Confiança: {{ "%.1f" | format(predicao_atual.confianca * 100) }}%
                    </div>
                    
                    <div class="confidence-bar">
                        <div class="confidence-fill" style="width: {{ predicao_atual.confianca * 100 }}%"></div>
                    </div>
                    
                    <div style="font-size: 12px; color: #888; margin-top: 10px;">
                        Última atualização: {{ dados_atuais.timestamp.strftime('%H:%M:%S') }}
                    </div>
                </div>
                
                <div class="probability-bars">
                    <div class="prob-item">
                        <span style="font-size: 12px;">Vazia</span>
                        <div class="prob-bar">
                            <div class="prob-fill prob-vazia" style="width: {{ predicao_atual.probabilidade_vazia * 100 }}%"></div>
                        </div>
                        <span style="font-size: 12px;">{{ "%.1f" | format(predicao_atual.probabilidade_vazia * 100) }}%</span>
                    </div>
                    
                    <div class="prob-item">
                        <span style="font-size: 12px;">Ocupada</span>
                        <div class="prob-bar">
                            <div class="prob-fill prob-ocupada" style="width: {{ predicao_atual.probabilidade_ocupada * 100 }}%"></div>
                        </div>
                        <span style="font-size: 12px;">{{ "%.1f" | format(predicao_atual.probabilidade_ocupada * 100) }}%</span>
                    </div>
                </div>
            </div>
            
            <div class="status-card">
                <h3 style="margin-bottom: 20px; font-size: 18px; color: #fff;">Sensores IoT</h3>
                
                <div class="sensor-grid">
                    <div class="sensor-item">
                        <span class="sensor-icon">🌡️</span>
                        <div class="sensor-label">Temperature</div>
                        <div class="sensor-value">{{ "%.1f" | format(dados_atuais.Temperature) }}°C</div>
                    </div>
                    
                    <div class="sensor-item">
                        <span class="sensor-icon">💧</span>
                        <div class="sensor-label">Humidity</div>
                        <div class="sensor-value">{{ "%.1f" | format(dados_atuais.Humidity) }}%</div>
                    </div>
                    
                    <div class="sensor-item">
                        <span class="sensor-icon">💡</span>
                        <div class="sensor-label">Light</div>
                        <div class="sensor-value">{{ "%.1f" | format(dados_atuais.Light) }} Lux</div>
                    </div>
                    
                    <div class="sensor-item">
                        <span class="sensor-icon">🌫️</span>
                        <div class="sensor-label">CO2</div>
                        <div class="sensor-value">{{ "%.1f" | format(dados_atuais.CO2) }} ppm</div>
                    </div>
                </div>
                
                <div style="margin-top: 15px; padding: 15px; background: rgba(255, 255, 255, 0.05); border-radius: 10px;">
                    <div class="sensor-label">HumidityRatio</div>
                    <div class="sensor-value">{{ "%.6f" | format(dados_atuais.HumidityRatio) }}</div>
                </div>
            </div>
            {% else %}
            <div class="status-card">
                <div class="status-indicator">
                    <div class="status-badge pulse" style="background: linear-gradient(135deg, #ffa726, #fb8c00);">
                        ⏳ Aguardando Dados...
                    </div>
                    
                    <div style="font-size: 14px; color: #ccc;">
                        Conectando aos sensores IoT
                    </div>
                </div>
            </div>
            {% endif %}
        </div>
    </div>
    
    <div class="refresh-indicator">
        Atualizando em 5s
    </div>
</body>
</html>
"""

@app.route('/')
def monitor_console():
    """Página principal do monitor console"""
    return render_template_string(
        HTML_TEMPLATE,
        mensagens_log=mensagens_log,
        dados_atuais=dados_atuais,
        predicao_atual=predicao_atual,
        intervalo=INTERVALO
    )

@app.route('/status')
def status_simples():
    """Endpoint simples para status atual"""
    if dados_atuais and predicao_atual:
        return {
            'status': predicao_atual['status'],
            'confianca': f"{predicao_atual['confianca']*100:.1f}%",
            'timestamp': dados_atuais['timestamp'].isoformat(),
            'temperatura': dados_atuais['Temperature'],
            'umidade': dados_atuais['Humidity']
        }
    else:
        return {'status': 'Aguardando dados...'}

if __name__ == '__main__':
    print("=== Flask Monitor Console - Ocupação IoT ===")
    print("Inicializando sistema...")
    
    if inicializar_sistema():
        print("Sistema inicializado com sucesso!")
        
        # Inicia thread de monitoramento
        monitor_thread = threading.Thread(target=monitor_loop, daemon=True)
        monitor_thread.start()
        
        print("\n🌐 Acesse: http://localhost:5001")
        print("📊 Status: http://localhost:5001/status")
        print("\nIniciando servidor Flask...")
        
        app.run(debug=False, host='0.0.0.0', port=5001, threaded=True)
    else:
        print("Falha na inicialização!")
        exit(1)