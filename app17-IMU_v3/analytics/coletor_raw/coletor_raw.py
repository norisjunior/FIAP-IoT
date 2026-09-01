# =====================================================================
# coletor_raw.py — FORMA 2: le ax,ay,az da Serial e grava CSV
#
# Le o stream do app17-10 (linhas "ax,ay,az", continuo), adiciona o timestamp
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
# UMA SESSAO = UMA CLASSE, com VARIAS RODADAS. O app17-10 streama continuamente
# e nao tem botao (montagem minima, de proposito), entao quem delimita cada
# rodada e o OPERADOR, pelo teclado: ENTER inicia, ENTER de novo encerra a
# rodada e abre a proxima — sem reiniciar o script. Ctrl+C encerra a sessao.
# Isso deixa barato coletar 5 rodadas por classe, que e o que o notebook 2.5
# precisa para o LeaveOneGroupOut e a curva de aprendizado por rodada.
# =====================================================================

import csv
import threading
from datetime import datetime
import serial  # pyserial

# ---- (A) Wokwi (padrao) ----
PORTA = "rfc2217://localhost:4000"

# ---- (B) ESP32 fisico (comente A e descomente B) ----
# PORTA = "COM6"

BAUD = 115200

# Lista fechada: evita digitar "anomalia" por engano quando a classe e
# "anomalo" (o notebook 2.5 descartaria a rodada inteira em silencio).
LABELS_VALIDOS = [
    "normal", "anomalo",
    "desligado", "operando", "inclinado_frente", "inclinado_tras", "anomalia",
]

label = ""
while label not in LABELS_VALIDOS:
    label = input(f"Label desta sessao {LABELS_VALIDOS}: ").strip()
    if label not in LABELS_VALIDOS:
        print(f"Label invalido. Use um destes: {', '.join(LABELS_VALIDOS)}")

ser = serial.serial_for_url(PORTA, baudrate=BAUD, timeout=1)
print(f"Conectado em {PORTA}.")
print(f"Classe da sessao: {label}")
print("Cada rodada: ENTER inicia, ENTER de novo encerra e abre a proxima.")
print("Ctrl+C encerra a sessao inteira.\n")


def aguardar_enter(evento):
    # Roda em thread separada so para nao bloquear a leitura da Serial
    # enquanto esperamos o ENTER que encerra a rodada.
    input()
    evento.set()


rodada = 0
arquivo = None  # referencia da rodada aberta, para fechar direito se vier Ctrl+C no meio dela
try:
    while True:
        rodada += 1
        input(f"[rodada {rodada:02d}] posicione o sensor e pressione ENTER para iniciar...")

        nome_arquivo = f"coleta_{label}_{rodada:02d}_{datetime.now():%Y%m%d_%H%M%S}.csv"
        arquivo = open(nome_arquivo, "w", newline="")
        escritor = csv.writer(arquivo)
        escritor.writerow(["timestamp", "ax", "ay", "az", "label"])
        print(f"Gravando rodada {rodada:02d} em {nome_arquivo}. ENTER encerra a rodada.")

        fim_rodada = threading.Event()
        threading.Thread(target=aguardar_enter, args=(fim_rodada,), daemon=True).start()

        n = 0
        while not fim_rodada.is_set():
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
                print(f"rodada {rodada:02d}: {n} amostras...", end="\r")

        arquivo.close()
        arquivo = None
        print(f"\nRodada {rodada:02d} encerrada: {n} amostras em {nome_arquivo}")
except KeyboardInterrupt:
    if arquivo is not None and not arquivo.closed:
        arquivo.close()  # rodada em andamento: fecha para nao perder o buffer
        print(f"\nRodada {rodada:02d} interrompida, mas salva em {nome_arquivo}")
    print(f"Sessao encerrada. {rodada} rodada(s) para a classe '{label}'.")
finally:
    ser.close()
