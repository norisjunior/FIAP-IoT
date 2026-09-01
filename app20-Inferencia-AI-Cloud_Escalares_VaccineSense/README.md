# Aplicação 19 - Vaccine Sense: predição

O app18 terminou com um CSV rotulado. Neste app nós treinamos o modelo e usamos
esse modelo para classificar a última medição da caixa.

```text
ESP32 → MQTT → Node-RED → InfluxDB → Python → modelo.predict()
```

Até o InfluxDB, tudo é igual ao app18.

## 1. Treinar o modelo

Abra no Google Colab:

```text
colab/app19_treinamento_vaccinesense.ipynb
```

Envie o `vaccinesense_dataset.csv` gerado no app18, execute as células e baixe:

```text
modelo_vaccinesense.pkl
```

Coloque o arquivo dentro da pasta `app`.

## 2. Enviar as medições

O firmware, o circuito e o fluxo Node-RED já estão neste projeto. São os mesmos
do app18.

Compile o ESP32, importe `nodered/vaccinesense-ingestao.json` e inicie a coleta
pelo botão da caixa.

## 3. Executar o Python

Na pasta `app`, preencha as configurações do InfluxDB nos dois arquivos Python.

```bash
pip install -r requirements.txt
python appConsoleVaccineSense.py
```

A aplicação pega a última medição e executa:

```python
predicao = modelo.predict(medicao)[0]
```

O resultado será:

```text
TRANSPORTE OK
```

ou:

```text
CARGA EM PERIGO
```

Para ver o mesmo resultado no navegador:

```bash
python appWebVaccineSense.py
```

Acesse `http://localhost:5000`.

## Próximo passo

No app20, a aplicação Python vira um serviço para o n8n consultar e acionar o
buzzer por MQTT.
