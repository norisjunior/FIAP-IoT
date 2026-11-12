import pyqtgraph as pg
from pyqtgraph.Qt import QtWidgets
import serial
from collections import deque

PORTA = "COM22"
BAUD = 115200
JANELA = 200

app = QtWidgets.QApplication([])

win = pg.GraphicsLayoutWidget(show=True, title="Serial Plot")
win.resize(900, 600)

plots = {}
curvas = {}
dados = {}

# DTR e RTS desligados ANTES de abrir: em placas com CH9102/CH340, com eles
# ligados (o padrao do pyserial) o ESP32 fica preso em reset e nada chega.
ser = serial.Serial()
ser.port = PORTA
ser.baudrate = BAUD
ser.timeout = 1
ser.dtr = False
ser.rts = False
ser.open()

cores = ["orange", "cyan", "magenta", "yellow", "lime", "red", "white"]
cor_index = 0

def criar_plot(tag):
    global cor_index

    if len(plots) > 0:
        win.nextRow()

    plot = win.addPlot()
    plot.setYRange(-4, 4)  # em g. Chacoalhando forte passa disso: aumente se precisar
    plot.showGrid(x=True, y=True)

    cor = cores[cor_index % len(cores)]
    cor_index += 1

    curva = plot.plot(pen=pg.mkPen(cor, width=2))

    plots[tag] = plot
    curvas[tag] = curva
    dados[tag] = deque([0] * JANELA, maxlen=JANELA)

    # título inicial
    plot.setTitle(f"<b>{tag}</b><br><span style='font-size:10pt'>---</span>")

def atualizar():
    while ser.in_waiting:
        linha = ser.readline().decode(errors="ignore").strip()

        if not linha.startswith(">"):
            continue

        try:
            tag, valor = linha[1:].split(":")
            tag = tag.strip()
            valor = float(valor.strip())
        except:
            continue

        if tag not in dados:
            criar_plot(tag)

        dados[tag].append(valor)
        curvas[tag].setData(list(dados[tag]))

        # atualiza título + subtítulo
        plots[tag].setTitle(
            f"<b>{tag}</b><br><span style='font-size:10pt'>{valor:.2f}</span>"
        )

timer = pg.QtCore.QTimer()
timer.timeout.connect(atualizar)
timer.start(10)

app.exec()