#!/bin/bash

# Script para parar MQTT Broker, Node-RED e n8n
# WSL2 Ubuntu

set -e

echo "================================================"
echo "Parando todos os serviços..."
echo "================================================"

cd IoTStack

# n8n
echo ""
echo "[1/3] Parando n8n..."
cd n8n
sudo docker compose down
echo "✓ n8n parado"
cd ..

# Node-RED
echo ""
echo "[2/3] Parando Node-RED..."
cd nodered
sudo docker compose down
echo "✓ Node-RED parado"
cd ..

# MQTT Broker
echo ""
echo "[3/3] Parando MQTT Broker..."
cd mqtt-broker
sudo docker compose down
echo "✓ MQTT Broker parado"
cd ..

echo ""
echo "================================================"
echo "Todos os serviços foram parados!"
echo "================================================"
echo ""