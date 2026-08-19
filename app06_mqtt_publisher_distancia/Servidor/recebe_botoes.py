#!/usr/bin/env python3
"""
recebe_botoes.py - "Servidor" da demonstracao de MQTT (Aplicacoes 06 e 07)

Assina, com coringa, o que os grupos publicam e mostra no terminal:

  fiap/iot/<TURMA>/grupo/+/botao   -> ranking de quem apertou primeiro
  fiap/iot/<TURMA>/prof/dist       -> distancia publicada pelo professor

E a alternativa em codigo ao dashboard do Node-RED: serve para mostrar que o
"servidor" nao tem nada de especial - e so mais um cliente MQTT.

Uso:
    python recebe_botoes.py                       # broker em localhost
    python recebe_botoes.py 192.168.0.100         # broker em outra maquina
    python recebe_botoes.py 192.168.0.100 2026b   # turma diferente

Dependencia:
    pip install paho-mqtt
"""
import sys
from datetime import datetime

try:
    import paho.mqtt.client as mqtt
except ImportError:
    print("paho-mqtt nao esta instalado. Instale com: pip install paho-mqtt")
    sys.exit(1)


# --- Configuracoes (podem vir da linha de comando) -------------------------
BROKER = sys.argv[1] if len(sys.argv) > 1 else "localhost"
TURMA = sys.argv[2] if len(sys.argv) > 2 else "2026"
PORT = 1883

TOPICO_BOTAO = "fiap/iot/%s/grupo/+/botao" % TURMA
TOPICO_DIST = "fiap/iot/%s/prof/dist" % TURMA

# --- Estado ----------------------------------------------------------------
ranking = []   # ordem de chegada dos botoes
distancia = None


def desenhar():
    """Redesenha o painel inteiro a cada evento."""
    print("\033[2J\033[H", end="")  # limpa a tela
    print("=" * 58)
    print(" SERVIDOR MQTT - turma %s   (broker %s:%d)" % (TURMA, BROKER, PORT))
    print("=" * 58)

    if distancia is not None:
        print("\n Distancia do professor: %5.1f cm" % distancia)

    print("\n RANKING DOS BOTOES")
    print(" " + "-" * 40)
    if not ranking:
        print("   (ninguem apertou ainda)")
    for i, (grupo, horario) in enumerate(ranking, start=1):
        marca = "  <-- primeiro!" if i == 1 else ""
        print("   %2d.  Grupo %s   %s%s" % (i, grupo, horario, marca))

    print("\n Ctrl+C para encerrar.")


def on_connect(client, userdata, flags, reason_code, properties=None):
    print("Conectado ao broker %s:%d (rc=%s)" % (BROKER, PORT, reason_code))
    client.subscribe([(TOPICO_BOTAO, 0), (TOPICO_DIST, 0)])
    desenhar()


def on_message(client, userdata, msg):
    global distancia

    payload = msg.payload.decode(errors="replace")

    if msg.topic == TOPICO_DIST:
        try:
            distancia = float(payload)
        except ValueError:
            return

    elif msg.topic.endswith("/botao"):
        grupo = msg.topic.split("/")[4]
        if grupo not in [g for g, _ in ranking]:
            ranking.append((grupo, datetime.now().strftime("%H:%M:%S")))

    desenhar()


def main():
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    client.on_connect = on_connect
    client.on_message = on_message

    try:
        client.connect(BROKER, PORT, 60)
    except Exception as e:
        print("Falha ao conectar em %s:%d -> %s" % (BROKER, PORT, e))
        print("Verifique o IP, o firewall do notebook e se o broker esta no ar.")
        sys.exit(1)

    try:
        client.loop_forever()
    except KeyboardInterrupt:
        print("\nEncerrando.")
        client.disconnect()


if __name__ == "__main__":
    main()
