# app.py
import os
import joblib, numpy as np
from fastapi import FastAPI
from pydantic import BaseModel
from tensorflow import keras

app = FastAPI()

# Carrega apenas 1x (rápido nas requisições)
#MODEL_PATH = "modelo_aqi_nn.keras"
#PREP_PATH  = "preprocess_aqi.pkl"
MODEL_PATH = os.getenv("MODEL_PATH", "modelo_aqi_nn.keras")
PREP_PATH  = os.getenv("PREP_PATH",  "preprocess_aqi.pkl")
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
    x = np.array([[float(payload[col]) for col in FEATURES]], dtype=np.float32)
    x_std = scaler.transform(x).astype(np.float32)
    proba = model(x_std, training=False).numpy()[0]
    idx = int(np.argmax(proba))
    return {
        "class": classes[idx],
        "probabilities": {cls: float(p) for cls, p in zip(classes, proba)}
    }
