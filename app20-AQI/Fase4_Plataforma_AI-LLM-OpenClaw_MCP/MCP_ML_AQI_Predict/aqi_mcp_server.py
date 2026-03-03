# aqi_mcp_server.py
import os
import requests
import psycopg2
import paho.mqtt.client as mqtt

from fastapi import FastAPI, HTTPException
from pydantic import BaseModel

app = FastAPI(title="AQI MCP (Toolbox)")

# ====== CONFIG (HOST) ======
PG_HOST = os.getenv("PG_HOST", "127.0.0.1")
PG_PORT = int(os.getenv("PG_PORT", "5432"))
PG_DB   = os.getenv("PG_DB", "n8n")
PG_USER = os.getenv("PG_USER", "n8nuser")
PG_PASS = os.getenv("PG_PASS", "minha_senha_super_secreta")

PREDICT_URL = os.getenv("PREDICT_URL", "http://127.0.0.1:8000/predict")

MQTT_HOST = os.getenv("MQTT_HOST", "127.0.0.1")
MQTT_PORT = int(os.getenv("MQTT_PORT", "1883"))

def pg_conn():
    return psycopg2.connect(
        host=PG_HOST, port=PG_PORT, dbname=PG_DB, user=PG_USER, password=PG_PASS
    )

class HistoricoReq(BaseModel):
    date: str      # "AAAA-MM-DD"
    periodo: str   # "madrugada" | "manhã" | "tarde" | "noite"

class PredictReq(BaseModel):
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

class MqttPubReq(BaseModel):
    topic: str
    payload: str

@app.get("/health")
def health():
    return {"ok": True}

@app.get("/tools/aqi_atual")
def aqi_atual():
    try:
        with pg_conn() as conn:
            with conn.cursor() as cur:
                cur.execute("""
                    SELECT class, timestamp_medicao, periodo
                    FROM aqi_medicoes
                    ORDER BY created_at DESC
                    LIMIT 1;
                """)
                row = cur.fetchone()
        if not row:
            return {"found": False}
        return {"found": True, "class": row[0], "timestamp": row[1], "periodo": row[2]}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/tools/aqi_historico")
def aqi_historico(req: HistoricoReq):
    try:
        with pg_conn() as conn:
            with conn.cursor() as cur:
                cur.execute("""
                    SELECT class, timestamp_medicao, periodo
                    FROM aqi_medicoes
                    WHERE DATE(timestamp_medicao::timestamp) = %s
                      AND periodo = %s
                    ORDER BY created_at DESC
                    LIMIT 1;
                """, (req.date, req.periodo))
                row = cur.fetchone()
        if not row:
            return {"found": False, "date": req.date, "periodo": req.periodo}
        return {"found": True, "class": row[0], "timestamp": row[1], "periodo": row[2]}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/tools/aqi_predict")
def aqi_predict(req: PredictReq):
    try:
        r = requests.post(PREDICT_URL, json=req.model_dump(), timeout=10)
        r.raise_for_status()
        return r.json()
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"predict proxy error: {e}")

@app.post("/tools/mqtt_publish")
def mqtt_publish(req: MqttPubReq):
    try:
        c = mqtt.Client()
        c.connect(MQTT_HOST, MQTT_PORT, 60)
        c.publish(req.topic, req.payload)
        c.disconnect()
        return {"published": True, "topic": req.topic}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"mqtt publish error: {e}")