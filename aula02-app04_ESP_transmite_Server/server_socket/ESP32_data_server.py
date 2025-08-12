import socket
import json
import csv
from datetime import datetime
import pytz
import signal
import sys

HOST = "0.0.0.0"
PORT = 5000
CSV_FILE = "dados.csv"

br_tz = pytz.timezone("America/Sao_Paulo")

# Criação do cabeçalho do CSV se ele não existir
try:
    with open(CSV_FILE, "x", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["timestamp", "dispositivo", "temp", "umid", "hic", "motor_x", "motor_y", "motor_z"])
except FileExistsError:
    pass

def signal_handler(sig, frame):
    """Handler para capturar CTRL+C e fazer saída limpa"""
    print("\n\nCtrl+C detectado! Encerrando servidor...")
    print("Servidor encerrado com sucesso.")
    sys.exit(0)

# Registra o handler para CTRL+C
signal.signal(signal.SIGINT, signal_handler)

# Servidor TCP
try:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
        print(f"Servidor aguardando conexões na porta {PORT}...")
        print("Pressione Ctrl+C para parar o servidor")

        while True:
            try:
                conn, addr = s.accept()
                with conn:
                    print(f"Conectado por {addr}")
                    data = conn.recv(1024)
                    if not data:
                        continue

                    try:
                        payload = json.loads(data.decode())
                        print("Dados recebidos:", payload)
                        timestamp = datetime.now(br_tz).isoformat()

                        with open(CSV_FILE, "a", newline="") as f:
                            writer = csv.writer(f)
                            writer.writerow([
                                timestamp,
                                payload.get("device"),
                                payload.get("temp"),
                                payload.get("umid"),
                                payload.get("hic"),
                                payload.get("motor_accel_x"),
                                payload.get("motor_accel_y"),
                                payload.get("motor_accel_z")
                            ])
                        print("Dados gravados no CSV.")

                    except json.JSONDecodeError:
                        print("Erro: JSON inválido")
                        
            except socket.error as e:
                print(f"Erro de socket: {e}")
                continue

except KeyboardInterrupt:
    print("\n\nCtrl+C detectado! Encerrando servidor...")
    print("Servidor encerrado com sucesso.")
except Exception as e:
    print(f"Erro inesperado: {e}")
finally:
    print("Limpeza finalizada.")