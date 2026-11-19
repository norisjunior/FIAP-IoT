# app.py
import joblib, numpy as np
from fastapi import FastAPI
from pydantic import BaseModel
from tensorflow import keras
import tensorflow as tf
import pandas as pd

app = FastAPI()


# ===== CONFIGURAÇÃO GPU =====
print("\n" + "="*60)
print("Iniciando aplicação AQI com TensorFlow", tf.__version__)
print("="*60)

gpus = tf.config.list_physical_devices('GPU')
if gpus:
    print(f"GPU HABILITADA: {gpus[0].name}")
    print(f"   Dispositivo: NVIDIA GeForce GTX 1650 (4GB)")
    try:
        # Permitir crescimento dinâmico de memória
        for gpu in gpus:
            tf.config.experimental.set_memory_growth(gpu, True)
        print("--> Memória GPU configurada (crescimento dinâmico)")
    except RuntimeError as e:
        print(f"!! Erro ao configurar GPU: {e}")
else:
    print("--> GPU não detectada - usando CPU")
print("="*60 + "\n")

# Carrega apenas 1x (rápido nas requisições)
MODEL_PATH = "modelo_aqi_nn.keras"
PREP_PATH  = "preprocess_aqi.pkl"
model = keras.models.load_model(MODEL_PATH)
prep  = joblib.load(PREP_PATH)

FEATURES = prep["feature_columns"]
scaler   = prep["scaler"]
classes  = prep["classes"]

# warm-up para shape fixo
_ = model(np.zeros((1, len(FEATURES)), dtype=np.float32), training=False)

class AQIItem(BaseModel):
    PM2_5: float
    PM10: float
    NO: float
    NO2: float
    NOx: float
    NH3: float
    CO: float
    SO2: float
    O3: float
    Benzene: float
    Toluene: float
    Xylene: float

@app.post("/predict")
def predict(item: AQIItem):
    # mapeia nomes do JSON do MQTT → nomes do treino
    payload = {
        "PM2_5": item.PM2_5, "PM10": item.PM10, "NO": item.NO, "NO2": item.NO2, "NOx": item.NOx,
        "NH3": item.NH3, "CO": item.CO, "SO2": item.SO2, "O3": item.O3,
        "Benzene": item.Benzene, "Toluene": item.Toluene, "Xylene": item.Xylene
    }
    #x = np.array([[float(payload[col]) for col in FEATURES]], dtype=np.float32)
    #x_std = scaler.transform(x).astype(np.float32)

    # Cria uma lista de valores na ordem correta
    values = [float(payload[col]) for col in FEATURES]
    
    # 1. Cria um DataFrame com os nomes das FEATURES
    # Isso resolve o warning!
    x_df = pd.DataFrame([values], columns=FEATURES) 
    
    # 2. Transforma o DataFrame
    x_std = scaler.transform(x_df).astype(np.float32)

    proba = model(x_std, training=False).numpy()[0]
    idx = int(np.argmax(proba))

    # Estrutura do resultado
    result = {
        "class": classes[idx],
        "probabilities": {cls: float(p) for cls, p in zip(classes, proba)}
    }

    # Mostra resultado no terminal
    print("--- Resultado da Predição AQI ---")
    print(result)
    print("---------------------------------")    

    return result
