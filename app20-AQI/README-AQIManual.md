# App 20 - Serviço de Índice de Qualidade do Ar (AQI) aplicando Machine Learning (Redes neurais)

## 1. Treinar o modelo de ML (pipeline)

* Realize o treinamento da Rede Neural para classificação do **Índice de Qualidade do Ar (AQI)**.
* Use o pipeline que normaliza os dados e salva o modelo e o scaler.
* Ao final, salve os arquivos gerados:

  * `modelo_aqi_nn.keras`
  * `preprocess_aqi.pkl`

> O treinamento (e a geração do modelo) pode ser feito no link do Colab apresentado em sala de aula.

---

## 2. Configurar e executar o ESP32

* Compile e execute o ESP32 + Wokwi (`iot-app20.ino`)
* Ele deve publicar as medições no **tópico MQTT**:

  ```
  FIAPIoT/aqi/dados
  ```
* Campos publicados:

  ```
  PM2.5, PM10, NO, NO2, NOx, NH3, CO, SO2, O3, Benzene, Toluene, Xylene
  ```
* Cada valor pode ser simulado com potenciômetros no Wokwi.

---

## 3. Iniciar a plataforma IoT

* Plataforma IoT foi apresentada nas aulas anteriores

Acesse: https://github.com/norisjunior/FIAP-IoT/tree/main/IoT-platform

---

## 4. Subir o serviço de inferência (Flask / Uvicorn)

- Acesse o diretório da aplicação
```
cd FIAP-IoT/app20-AQI/AQI_ML_app
```

* Crie um virtualenv:
```
python -m venv venv
```

* Ative o virtualenv criado:
-> Windows:
```
.\venv\Scripts\Activate.ps1
```

-> Linux
```
source venv/bin/activate
```

* Atualize o pip:
```
python -m pip install --upgrade pip
```

* Instale as dependências:
```
pip install -r requirements.txt
```

* Execute o servidor:

```
uvicorn service_app:app --reload --port 8000
```

* Teste a API com:

```
curl -X POST http://localhost:8000/predict -H "Content-Type: application/json" -d '{
  "PM2_5": 342, "PM10": 477, "NO": 10, "NO2": 51, "NOx": 40,
  "NH3": 42, "CO": 1.7, "SO2": 17, "O3": 91,
  "Benzene": 5.2, "Toluene": 29, "Xylene": 0.5
}'
```

* Saída esperada:

```
{"class":"Perigoso","probabilities":{"Aceitável":3.5151686006429372e-06,"Perigoso":0.8978066444396973,"Ruim":0.102189801633358}}
```

---

## 5. Configurar o n8n

* Importe o arquivo `Fluxo-n8n.json`.
* Ajuste o nó **MQTT Trigger** para conectar em:

  ```
  Host: mqtt-broker
  Port: 1883
  ```
* Ajuste o **HTTP Request** para:

  ```
  URL: http://host.docker.internal:8000/predict
  ```
* No nó final, configure o envio para o Telegram:

  * Casos `Aceitável` e `Ruim` → INFO
  * Caso `Perigoso` → CRITICAL

---

## 6. Executar o fluxo completo

1. **Inicie a Plataforma IoT**
2. **Execute o serviço Flask**
3. **Abra o n8n e inicie o workflow**
4. **Ative o ESP32 (Wokwi)**

Cada nova medição publicada pelo ESP32 será processada pelo n8n, que consultará o modelo via Flask e enviará o alerta correspondente pelo Telegram.

---

## 7. Entendimento do Ciclo

```
ESP32 → MQTT → n8n → Flask (ML) → Telegram
```

No próximo passo será mostrado como empacotar tudo isso em containers Docker, automatizando o processo.
