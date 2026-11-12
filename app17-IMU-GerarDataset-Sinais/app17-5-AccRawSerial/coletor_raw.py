"""
coletor_raw.py - le o "ax,ay,az" que o app17-5 imprime na Serial e grava um CSV
com timestamp e label.

Cada execucao e uma RODADA: uma condicao estavel do inicio ao fim.

Uso:
    python coletor_raw.py COM6 normal 60
    python coletor_raw.py COM6 anomalia 60
    python coletor_raw.py rfc2217://localhost:4000 normal 60   (Wokwi)

Argumentos: porta, label, segundos de coleta.
"""
import csv
import sys
import time
from datetime import datetime, timezone

import serial

PORTA    = sys.argv[1] if len(sys.argv) > 1 else "COM6"
LABEL    = sys.argv[2] if len(sys.argv) > 2 else "normal"
SEGUNDOS = int(sys.argv[3]) if len(sys.argv) > 3 else 60
DESCARTE = 5   # segundos iniciais jogados fora: voce ainda esta posicionando o sensor

arquivo = f"raw_{LABEL}_{datetime.now():%Y%m%d_%H%M%S}.csv"

# serial_for_url aceita tanto "COM6" quanto "rfc2217://localhost:4000" (Wokwi).
# DTR e RTS desligados ANTES de abrir: em placas com CH9102/CH340, com eles
# ligados (o padrao do pyserial) o ESP32 fica preso em reset e nada chega.
ser = serial.serial_for_url(PORTA, baudrate=115200, timeout=1, do_not_open=True)
ser.dtr = False
ser.rts = False
ser.open()

print(f"Rodada '{LABEL}' por {SEGUNDOS} s -> {arquivo}")
print(f"Descartando os primeiros {DESCARTE} s...")

inicio = time.time()
linhas = 0

with open(arquivo, "w", newline="") as f:
    csv_out = csv.writer(f)
    csv_out.writerow(["timestamp", "ax", "ay", "az", "label"])

    while time.time() - inicio < DESCARTE + SEGUNDOS:
        texto = ser.readline().decode(errors="ignore").strip()

        # So interessam as linhas com 3 numeros. Cabecalho, mensagens do
        # setup e linhas cortadas sao ignorados.
        partes = texto.split(",")
        if len(partes) != 3:
            continue
        try:
            ax, ay, az = (float(p) for p in partes)
        except ValueError:
            continue

        if time.time() - inicio < DESCARTE:
            continue

        # O horario e o do computador, no momento em que a linha chegou.
        agora = datetime.now(timezone.utc).isoformat(timespec="milliseconds")
        csv_out.writerow([agora, ax, ay, az, LABEL])
        linhas += 1

ser.close()
print(f"{linhas} leituras gravadas (~{linhas / SEGUNDOS:.0f} por segundo).")
