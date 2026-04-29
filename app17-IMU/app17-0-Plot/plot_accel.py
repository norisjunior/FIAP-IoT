import pyqtgraph as pg
from pyqtgraph.Qt import QtWidgets
import serial
from collections import deque

PORTA = "COM6"
BAUD = 115200

app = QtWidgets.QApplication([])

janela = 200

dados_x = deque([0]*janela, maxlen=janela)
dados_y = deque([0]*janela, maxlen=janela)
dados_z = deque([0]*janela, maxlen=janela)

win = pg.GraphicsLayoutWidget(show=True, title="Dados do acelerômetro")
win.resize(800, 600)

# --- X ---
plot_x = win.addPlot(title="X")
plot_x.setYRange(-12, 12)
curva_x = plot_x.plot(pen=pg.mkPen('orange', width=2))

label_x = pg.LabelItem(justify='left')
win.addItem(label_x)
win.nextRow()

# --- Y ---
plot_y = win.addPlot(title="Y")
plot_y.setYRange(-12, 12)
curva_y = plot_y.plot(pen=pg.mkPen('orange', width=2))

label_y = pg.LabelItem(justify='left')
win.addItem(label_y)
win.nextRow()

# --- Z ---
plot_z = win.addPlot(title="Z")
plot_z.setYRange(-12, 12)
curva_z = plot_z.plot(pen=pg.mkPen('orange', width=2))

label_z = pg.LabelItem(justify='left')
win.addItem(label_z)

ser = serial.Serial(PORTA, BAUD, timeout=1)

buffer = {}

def atualizar():
    while ser.in_waiting:
        linha = ser.readline().decode(errors="ignore").strip()

        if linha.startswith(">acc_x:"):
            buffer["x"] = float(linha.split(":")[1])

        elif linha.startswith(">acc_y:"):
            buffer["y"] = float(linha.split(":")[1])

        elif linha.startswith(">acc_z:"):
            buffer["z"] = float(linha.split(":")[1])

        if "x" in buffer and "y" in buffer and "z" in buffer:
            dados_x.append(buffer["x"])
            dados_y.append(buffer["y"])
            dados_z.append(buffer["z"])
            buffer.clear()

    curva_x.setData(dados_x)
    curva_y.setData(dados_y)
    curva_z.setData(dados_z)

    # subtítulos (valores atuais)
    if dados_x:
        label_x.setText(f"X atual: {dados_x[-1]:.2f} m/s²")
        label_y.setText(f"Y atual: {dados_y[-1]:.2f} m/s²")
        label_z.setText(f"Z atual: {dados_z[-1]:.2f} m/s²")

timer = pg.QtCore.QTimer()
timer.timeout.connect(atualizar)
timer.start(10)

app.exec()