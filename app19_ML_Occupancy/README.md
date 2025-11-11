# App 19


## 1. Treinar modelo de ML (pipeline)

- Realizar o treinamento do modelo de ML usando o pipeline com GridSearchCV, que apresentará o melhor modelo.
- No momento de construção deste app, o melhor modelo gerado (2025), foi o XGBoost (arquivo: modelo_ocupacao_best_xgbclassifier.pkl)

## 2. Compilar e executar o ESP32

- O ESP32 enviará informações de Temperature, Humidity, HumidityRatio, CO2, Light para o InfluxDB

## 3. Inicialização do servidor que obterá os dados do InfluxDB e executará o modelode ML em novos dados coletados continuamente

- Crie um virtualenv:
```
python -m venv occypancyEnv
```
- Ative o virtualenv criado:
```
.\occupancyEnv\Scripts\Activate.ps1
```

- Atualize o pip:
```
python -m pip install --upgrade pi
```

- Instale as dependências:
```
pip install -r requirements.txt
```

- Observe, com cautela, o app "ML_app.py"
    - Caso queira, abra um colab e copie e cole cada célula
    - Insira no colab o arquivo "modelo_ocupacao_best_xgbclassifier.pkl", pois assim o colab poderá carregar o modelo e executá-lo
    - Observe o carregamento do modelo, a conexão com o InfluxDB, a estruturação do comando SQL, a estruturação do dataframe (df), a aplicação do método .predict nos dados recentemente coletados.

## 4. Execute a aplicação que roda no modo "console"
```
python appConsoleOccupancy.py
```
Ela tem uma saída parecida com essa:

🚀 Monitor de Ocupação de Sala usando ML em dados IoT
==========================================================
📦 Carregando modelo...
✅ Modelo carregado: modelo_ocupacao_best_xgbclassifier.pkl
🌐 Conectando ao InfluxDB...
✅ Conectado ao InfluxDB
🔄 Monitorando dispositivo: Noris_ESP32_Aula13
⏰ Verificando a cada 10 segundos

🔍 Consultando dados... 🆕 Novo dado encontrado!

📊 Dados dos Sensores (02:58:02):
   🌡️  Temperature: 24.6 °C
   💧 Humidity: 40.5 %
   💡 Light: 216.7 Lux
   🌫️  CO2: 1323.7 ppm
   💨 HumidityRatio: 0.004043

🤖 Resultado do Modelo:
   🟢 SALA VAZIA
   📈 Confiança: 74.0%
   📊 Probabilidades:
      • Vazia: 74.0%
      • Ocupada: 26.0%

## 5. Execute a aplicação que roda no modo "web"
```
python appWebOccupancy.py
```
