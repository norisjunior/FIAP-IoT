#!/usr/bin/env python3
"""
enviaComandoMQTT.py
Demonstração de publisher MQTT.
Valores padrão baseados na aplicação Irrigação Inteligente:
  Broker: definido em `Device1-TinyML` e `Device2-BombaAgua`
  Tópico: FIAPIoT/smartagro/cmd/local
  Payload: JSON com campos `dispositivo` e `bomba`
"""
import sys

try:
    import paho.mqtt.client as mqtt
except Exception:
    print("paho-mqtt não está instalado. Instale com: pip install paho-mqtt")
    sys.exit(1)


# --- Configurações (edite aqui) -------------------------------------------
# Broker e porta (copiado dos .ino: MQTT_HOST / MQTT_PORT)
BROKER = "host.wokwi.internal"
PORT = 1883

# Tópico alvo (Device1 publica em "FIAPIoT/smartagro/cmd/local"; Device2 assina coringa)
TOPIC = "FIAPIoT/smartagro/cmd/local"

# Mensagem (payload JSON). Device2 espera: {"dispositivo": <string>, "bomba": <bool>}
# Ajuste `DISPOSITIVO_ID` e `BOMBA_LIGADA` conforme desejar demonstrar
DISPOSITIVO_ID = "ATACANTE01"
BOMBA_LIGADA = True

import json
MESSAGE = json.dumps({"dispositivo": DISPOSITIVO_ID, "bomba": BOMBA_LIGADA})


def main():
    client = mqtt.Client()
    try:
        client.connect(BROKER, PORT, 60)
        client.loop_start()
        result = client.publish(TOPIC, MESSAGE)
        try:
            result.wait_for_publish()
        except Exception:
            pass
        print(f"Publicado em {BROKER}:{PORT} → {TOPIC} : {MESSAGE}")
    except Exception as e:
        print("Falha ao conectar/publicar:", e)
    finally:
        try:
            client.loop_stop()
            client.disconnect()
        except Exception:
            pass


if __name__ == "__main__":
    main()
