import time
from pathlib import Path

import numpy as np
import requests
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.responses import FileResponse

URL = "http://localhost:1337/api/features"
INFO = requests.get("http://localhost:1337/api/info").json()["modelParameters"]
JANELA = INFO["input_features_count"]  # ~1,5 s: o tamanho que o modelo pede
PASSO = 8000          # classifica a cada 0,5 s
CLASSE = "WakeWord"
LIMIAR = 0.80
PAUSA = 5           # segundos sem disparar de novo

app = FastAPI()


@app.get("/")
def pagina():
    return FileResponse(Path(__file__).parent / "celular.html")


@app.websocket("/ouvir")
async def ouvir(ws: WebSocket):
    await ws.accept()
    print("Celular conectado")
    janela = np.zeros(0, dtype=np.int16)
    novas = 0
    ultimo_disparo = 0
    try:
        while True:
            bloco = np.frombuffer(await ws.receive_bytes(), dtype=np.int16)
            janela = np.concatenate([janela, bloco])[-JANELA:]
            novas += len(bloco)
            if len(janela) < JANELA or novas < PASSO:
                continue  # janela ainda incompleta, ou menos de 0,5 s de áudio novo
            novas = 0
            if not janela.any():
                continue  # janela toda zerada: microfone mudo, nada a classificar

            resposta = requests.post(URL, json={"features": janela.tolist()})
            resposta.raise_for_status()
            scores = resposta.json()["result"]["classification"]
            print("  ".join(f"{classe}: {valor:.2f}" for classe, valor in scores.items()))

            disparou = scores[CLASSE] >= LIMIAR and time.time() - ultimo_disparo > PAUSA
            if disparou:
                ultimo_disparo = time.time()
                print(">>> WAKE WORD! <<<")
            await ws.send_json({"scores": scores, "disparou": disparou})
    except WebSocketDisconnect:
        print("Celular desconectado")
