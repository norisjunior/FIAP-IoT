import time

import numpy as np
import requests
import sounddevice as sd

URL = "http://localhost:1337/api/features"
INFO = requests.get("http://localhost:1337/api/info").json()["modelParameters"]
TAXA = 16000          # a mesma do impulse
JANELA = INFO["input_features_count"]  # ~1,5 s: o tamanho que o modelo pede
PASSO = 8000          # classifica a cada 0,5 s
CLASSE = "WakeWord"
LIMIAR = 0.80
PAUSA = 1.5           # segundos sem disparar de novo

janela = np.zeros(JANELA, dtype=np.int16)
ultimo_disparo = 0

with sd.InputStream(samplerate=TAXA, channels=1, dtype="int16") as microfone:
    print("Ouvindo... (Ctrl+C para sair)")
    while True:
        bloco, _ = microfone.read(PASSO)
        janela = np.concatenate([janela[PASSO:], bloco[:, 0]])
        if not janela.any():
            continue  # janela toda zerada: microfone mudo, nada a classificar

        resposta = requests.post(URL, json={"features": janela.tolist()})
        resposta.raise_for_status()
        scores = resposta.json()["result"]["classification"]
        print("  ".join(f"{classe}: {valor:.2f}" for classe, valor in scores.items()))

        if scores[CLASSE] >= LIMIAR and time.time() - ultimo_disparo > PAUSA:
            ultimo_disparo = time.time()
            print(">>> WAKE WORD! <<<")
