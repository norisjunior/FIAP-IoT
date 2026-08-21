# =====================================================================
# coletor_raw.py — FORMA 2: le ax,ay,az da Serial e grava CSV
#
# Le o stream do app17-10 (linhas "ax,ay,az" a 100 Hz), adiciona o timestamp
# (hora do PC na recepcao) e a label desta rodada, e grava o CSV no formato
# da Sprint 3:  timestamp,ax,ay,az,label
#
# Estilo linear (igual ao app17-0-Plot): sem classes, configuracao no topo.
#
# CONEXAO (escolha UMA, no bloco abaixo):
#   (A) Wokwi: a Serial simulada e exposta via RFC2217 (wokwi.toml:
#       rfc2217ServerPort = 4000). Deixe a aba do simulador VISIVEL no VS Code.
#   (B) ESP32 fisico: a porta COM do PC (feche o Serial Monitor antes — so um
#       programa abre a porta por vez).
#
# Uma execucao = uma classe. Rode uma vez com 'normal' (MPU parado) e outra
# com 'anomalo' (chacoalhando/sacudindo o MPU).
# =====================================================================

import csv
from datetime import datetime
import serial  # pyserial

# ---- (A) Wokwi (padrao) ----
# PORTA = "rfc2217://localhost:4000"

# ---- (B) ESP32 fisico (comente A e descomente B) ----
PORTA = "COM7"

BAUD = 115200

# label desta rodada: 'normal' (motor parado) ou 'anomalo' (chacoalhando)
label = input("Label desta coleta (normal / anomalo): ").strip() or "normal"
arquivo = f"coleta_{label}_{datetime.now():%Y%m%d_%H%M%S}.csv"

ser = serial.serial_for_url(PORTA, baudrate=BAUD, timeout=1)
print(f"Conectado em {PORTA}.")
print(f"Gravando em {arquivo}. Pressione Ctrl+C para parar.")

n = 0
with open(arquivo, "w", newline="") as f:
    escritor = csv.writer(f)
    escritor.writerow(["timestamp", "ax", "ay", "az", "label"])  # cabecalho
    try:
        while True:
            linha = ser.readline().decode(errors="ignore").strip()
            if not linha:
                continue

            partes = linha.split(",")
            if len(partes) != 3:
                continue  # ignora o cabecalho "ax,ay,az" e linhas estranhas

            try:
                ax, ay, az = (float(p) for p in partes)
            except ValueError:
                continue  # linha nao numerica

            # timestamp = hora do PC na recepcao (limitacao: nao e o instante
            # exato da medicao no ESP32 — ha latencia Serial/USB).
            ts = datetime.now().isoformat(sep=" ", timespec="milliseconds")
            escritor.writerow([ts, f"{ax:.3f}", f"{ay:.3f}", f"{az:.3f}", label])

            n += 1
            if n % 100 == 0:
                print(f"{n} amostras...", end="\r")
    except KeyboardInterrupt:
        print(f"\nParado. {n} amostras salvas em {arquivo}.")
    finally:
        ser.close()
