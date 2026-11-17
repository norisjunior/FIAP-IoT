# coleta.py (app16-4): grava o dataset da caixa de fumaca e mostra o painel ao vivo no navegador
# Uso: python coleta.py                          (acha a placa sozinho)
#      python coleta.py COM27                    (porta escolhida)
#      python coleta.py --replay ARQUIVO.log [N] (repete uma coleta gravada, N vezes mais rapido)
# Grava: coleta-AAAAMMDD-HHMM.csv (o dataset), .log (tudo o que a placa mandou) e .png
# Ctrl+T e depois Ctrl+D no terminal: liga/desliga o DTR (duas vezes reinicia a placa)
# Parar: botao Parar no painel, ou Ctrl+C no terminal

import json
import sys
import threading
import time
import webbrowser
from datetime import datetime
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import parse_qs, urlparse

import serial
from serial.tools import list_ports

PASTA = Path(__file__).resolve().parent
VARIAVEIS = ["tvoc", "eco2", "raw_h2", "raw_ethanol", "pm2_5", "pm10", "nc0_5", "temp", "umid"]

# O dataset: as colunas do Kaggle, na mesma ordem, mais a rodada, o pm10 e o rotulo
COLUNAS = ["amostra", "rodada", "temp", "umid", "tvoc", "eco2", "raw_h2", "raw_ethanol", "pressao",
           "pm1_0", "pm2_5", "nc0_5", "nc1_0", "nc2_5", "pm10", "estimulo"]

amostras = []   # cada uma: [t, tvoc, eco2, ..., umid, estimulo] (o painel)
mensagens = []  # linhas que nao sao leitura: boot, ESTIMULO, erros
trava = threading.Lock()
parar = threading.Event()
inicio = time.monotonic()
ser = None
dataset = None
boot = False    # ao vivo, o que chega antes do boot e sobra de antes do reinicio


def leitura(linha):
    """Uma linha do CSV da placa (os 16 numeros separados por virgula), ou None."""
    campos = linha.split(",")
    if len(campos) != len(COLUNAS):
        return None
    try:
        return {c: float(v) for c, v in zip(COLUNAS, campos)}
    except ValueError:
        return None


def processa(linha, t):
    """Uma linha da placa: leitura (CSV) ou mensagem."""
    global boot
    if linha.startswith("ESP-ROM"):  # primeira linha do boot
        boot = True
    if not boot:
        return
    valores = leitura(linha)
    if valores:
        with trava:
            amostras.append([round(t, 1)] + [valores[n] for n in VARIAVEIS] + [int(valores["estimulo"])])
        if dataset:  # so ao vivo: o replay nao grava dataset
            dataset.write(linha.strip() + "\n")  # a linha como a placa mandou
            dataset.flush()
    elif linha.startswith("amostra,"):  # o cabecalho que a placa manda no boot: o .csv ja tem o seu
        return
    else:
        with trava:
            mensagens.append(linha)
        print(linha)


# Placa ao vivo

def acha_porta():
    placas = [p for p in list_ports.comports() if p.vid == 0x303A]  # 303A = Espressif
    if len(placas) == 1:
        return placas[0].device
    print("Nenhuma placa ESP32 conectada." if not placas else
          "Mais de uma placa ESP32: " + ", ".join(p.device for p in placas))
    print("Escolha a porta: python coleta.py COMxx  (lista: pio device list)")
    sys.exit(1)


def alterna_dtr():
    ser.dtr = not ser.dtr


def reinicia():
    alterna_dtr()
    time.sleep(0.3)
    alterna_dtr()


def le_serial(arq):
    while not parar.is_set():
        try:
            linha = ser.readline().decode(errors="replace").rstrip("\r\n")
        except serial.SerialException:
            print("[porta caiu; tentando reabrir]")
            ser.close()
            while not parar.is_set():
                time.sleep(0.5)
                try:
                    ser.open()
                    print("[porta reaberta]")
                    break
                except serial.SerialException:
                    pass
            continue
        if linha:
            arq.write(linha + "\n")
            arq.flush()
            processa(linha, time.monotonic() - inicio)


def teclado_terminal():
    try:
        import msvcrt
    except ImportError:
        return
    armado = False
    while not parar.is_set():
        if msvcrt.kbhit():
            c = msvcrt.getwch()
            if c == "\x14":                 # Ctrl+T
                armado = True
            elif c == "\x04" and armado:    # Ctrl+D
                alterna_dtr()
                print(f"[DTR {'ligado' if ser.dtr else 'desligado'}]")
                armado = False
            else:
                armado = False
        time.sleep(0.05)


# Coleta gravada

def le_arquivo(caminho, velocidade):
    n = 0
    for linha in open(caminho, encoding="utf-8"):
        if parar.is_set():
            return
        linha = linha.rstrip("\r\n")
        processa(linha, n)
        if leitura(linha):
            n += 1
            time.sleep(1 / velocidade)
    print("[fim da coleta gravada]")


# Painel no navegador

class Painel(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(PASTA / "painel"), **kwargs)

    def log_message(self, *args):
        pass

    def responde(self, corpo):
        dados = json.dumps(corpo).encode()
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(dados)))
        self.end_headers()
        self.wfile.write(dados)

    def do_GET(self):
        url = urlparse(self.path)
        if url.path != "/dados":
            return super().do_GET()
        q = parse_qs(url.query)
        desde = int(q.get("desde", ["0"])[0])
        msgs = int(q.get("msgs", ["0"])[0])
        with trava:
            self.responde({"amostras": amostras[desde:], "mensagens": mensagens[msgs:],
                           "origem": ORIGEM, "aovivo": ser is not None})

    def do_POST(self):
        if self.path == "/reiniciar" and ser is not None:
            reinicia()
        elif self.path == "/parar":
            parar.set()
            threading.Thread(target=servidor.shutdown).start()
        self.responde({"ok": True})


# Grafico final (PNG), com as mesmas cores do painel

PAINEIS = [("tvoc", "gás total (ppb)"), ("eco2", "CO₂ estimado (ppm)"),
           ("raw_h2", "cru H₂"), ("raw_ethanol", "cru etanol"),
           ("pm2_5", "partículas, massa (µg/m³)"), ("nc0_5", "partículas, contagem (/cm³)"),
           ("temp", "temperatura (°C)"), ("umid", "umidade (%)")]
CORES = {0: "#22c55e", 1: "#eab308", 2: "#a855f7", 3: "#ef4444", 4: "#3b82f6"}
NOMES = {0: "ar", 1: "álcool", 2: "vapor", 3: "fumaça", 4: "chama"}


def salva_png(caminho):
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    from matplotlib.patches import Patch

    ts = [a[0] for a in amostras]
    est = [a[-1] for a in amostras]
    col = {n: [a[1 + i] for a in amostras] for i, n in enumerate(VARIAVEIS)}
    fig, axs = plt.subplots(2, 4, figsize=(16, 7), sharex=True, layout="constrained")
    for ax, (nome, rotulo) in zip(axs.flat, PAINEIS):
        i0 = 0
        for i in range(1, len(ts) + 1):
            if i == len(ts) or est[i] != est[i0]:
                ax.axvspan(ts[i0], ts[min(i, len(ts) - 1)], color=CORES[est[i0]], alpha=0.14, lw=0)
                i0 = i
        ax.plot(ts, col[nome], color="#0f172a", lw=1.4)
        if nome == "pm2_5":
            ax.plot(ts, col["pm10"], color="#94a3b8", lw=1.2, ls="--")
        ax.set_title(f"{nome}  ·  {rotulo}", loc="left", fontsize=10, color="#0f172a")
        ax.grid(color="#e5e7eb", lw=0.8)
        ax.tick_params(colors="#6b7280", labelsize=8, length=0)
        for lado in ("top", "right", "left", "bottom"):
            ax.spines[lado].set_visible(False)
    for ax in axs[-1]:
        ax.set_xlabel("segundos", fontsize=8, color="#6b7280")
    fig.legend(handles=[Patch(color=CORES[e], alpha=0.35, label=NOMES[e]) for e in (0, 2, 4, 3, 1)],
               loc="upper right", ncol=5, frameon=False, fontsize=9)
    fig.suptitle(caminho.stem, x=0.01, ha="left", fontsize=11, color="#0f172a")
    fig.savefig(caminho, dpi=120, facecolor="white")


# Inicio

if len(sys.argv) > 1 and sys.argv[1] == "--replay":
    arquivo = Path(sys.argv[2])
    velocidade = float(sys.argv[3]) if len(sys.argv) > 3 else 1
    ORIGEM = f"{arquivo.name} (gravada, {velocidade:g}x)"
    boot = True
    threading.Thread(target=le_arquivo, args=(arquivo, velocidade), daemon=True).start()
    saida = None
else:
    porta = sys.argv[1] if len(sys.argv) > 1 else acha_porta()
    saida = PASTA / datetime.now().strftime("coleta-%Y%m%d-%H%M.log")
    ORIGEM = f"{saida.stem} · {porta}"
    ser = serial.Serial(porta, 115200, timeout=1)
    time.sleep(0.5)
    print("Reiniciando a placa: a coleta comeca no boot.")
    reinicia()  # o arquivo comeca no boot
    arq = open(saida, "w", encoding="utf-8")
    dataset = open(saida.with_suffix(".csv"), "w", encoding="utf-8")
    dataset.write(",".join(COLUNAS) + "\n")
    leitor = threading.Thread(target=le_serial, args=(arq,), daemon=True)
    leitor.start()
    threading.Thread(target=teclado_terminal, daemon=True).start()

servidor = ThreadingHTTPServer(("127.0.0.1", 0), Painel)
endereco = f"http://127.0.0.1:{servidor.server_port}/"
print(f"Painel: {endereco}   (Parar no painel ou Ctrl+C)")
webbrowser.open(endereco)
try:
    servidor.serve_forever()
except KeyboardInterrupt:
    pass

parar.set()
if saida:
    leitor.join(timeout=2)
    ser.close()
    arq.close()
    dataset.close()
    png = saida.with_suffix(".png")
    salva_png(png)
    print(f"Salvo: {saida.stem}.csv (o dataset), {saida.name} (tudo o que a placa mandou) e {png.name}")
