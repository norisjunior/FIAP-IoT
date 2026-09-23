# CloudAI — a qualidade do ar classificada na nuvem

O ESP32 lê doze sensores de poluentes, publica os valores por MQTT, e uma rede
neural servida por API devolve em qual das três faixas o ar está:
`Aceitável`, `Ruim` ou `Perigoso`.

```text
ESP32 (Wokwi) ─► MQTT ─► n8n ─► API (.keras) ─► n8n ─► Telegram
```

**Roda inteiro no Wokwi.** Não é preciso montar hardware nenhum: os doze
sensores são potenciômetros no simulador, e você mexe neles para ver a
classificação mudar.

## 1) A plataforma

Entre no diretório `IoT-platform` que você recebeu e suba a stack:

```bash
wsl -d ubuntu
cd ~/FIAP-IoT/IoT-platform/
sudo ./start-linux.sh
docker ps
```

Devem subir: MQTT Broker, n8n, Node-RED, InfluxDB e Grafana. Desta lista, este
app usa o **Mosquitto** e o **n8n**.

## 2) A API

```bash
cd ../api

python -m venv venv
venv\Scripts\activate        # Windows
source venv/bin/activate     # Linux / macOS

pip install -r requirements.txt
uvicorn service_app:app --reload --port 8000
```

Os arquivos `modelo_aqi_nn.keras` e `preprocess_aqi.pkl` precisam estar em
`api/`. Eles já vêm no projeto; para gerar os seus, use o notebook de
treinamento em `colab/`.

Teste antes de ligar qualquer outra coisa:

```bash
curl -X POST http://localhost:8000/predict -H "Content-Type: application/json" \
  -d "{\"PM2_5\":342,\"PM10\":477,\"NO\":10,\"NO2\":51,\"NOx\":40,\"NH3\":42,\"CO\":1.7,\"SO2\":17,\"O3\":91,\"Benzene\":5.2,\"Toluene\":29,\"Xylene\":0.5}"
```

Resposta esperada: `{"class":"Perigoso","probabilities":{...}}`. Documentação
automática em `http://localhost:8000/docs`.

## 3) O fluxo

Importe `n8n/Fluxo-predict.json` (n8n em `http://localhost:5678`) e configure as
credenciais de **MQTT** e **Telegram**.

| # | Nó | O que faz |
|---|---|---|
| 1 | `FIAPIoT/aqi/dados` | recebe os doze valores |
| 2 | `Code (gera JSON)` | converte a mensagem MQTT em JSON |
| 3 | `Verifica se medições são float` | barra `NaN` antes de gastar a chamada |
| 4 | `Predict AQI` | `POST` para a API |
| 5 | `Mensagem de Informação` | Telegram, toda predição |
| 6 | `Se condições são perigosas` | compara a classe com `Perigoso` |
| 7 | `PERIGOSO: Mensagem de ALERTA` | Telegram, só no caso crítico |

No nó **Predict AQI**, a URL é `http://host.docker.internal:8000/predict` — com
o n8n em contêiner e a API no seu computador, `localhost` de dentro do n8n não
acharia ninguém. Ative o workflow.

> **O nó 6 compara com a palavra `Perigoso`.** Esse é o contrato com o
> treinamento: se o modelo for retreinado com outros nomes de classe, este ramo
> para de disparar sem dar erro nenhum. Os três nomes estão fixados no notebook.

## 4) O dispositivo

```bash
cd device
pio run
```

Abra a simulação com a extensão Wokwi do VSCode. Mexa nos potenciômetros para
simular condições diferentes de ar. No Serial Monitor:

```text
SUCESSO: Dados AQI enviados para o MQTT Broker
```

Tópico de publicação: `FIAPIoT/aqi/dados`, uma mensagem com os doze poluentes.

## 5) Conferir o caminho inteiro

| Onde | O que esperar |
|---|---|
| Serial Monitor | `SUCESSO: Dados AQI enviados para o MQTT Broker` |
| Terminal da API | `POST /predict HTTP/1.1 200 OK` |
| n8n, aba Executions | execuções bem-sucedidas |
| Telegram | INFO em toda predição, ALERTA quando der `Perigoso` |

Se quiser ver só o que sai do dispositivo, sem n8n e sem API:

```bash
mosquitto_sub -h localhost -t "FIAPIoT/aqi/dados" -v
```
