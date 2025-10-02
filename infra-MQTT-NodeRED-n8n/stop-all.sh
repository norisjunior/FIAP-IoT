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
echo "[1/4] Parando n8n..."
cd n8n
sudo docker compose down
echo "✓ n8n parado"
cd ..

# Node-RED
echo ""
echo "[2/4] Parando Node-RED..."
cd nodered
sudo docker compose down
echo "✓ Node-RED parado"
cd ..

# MQTT Broker
echo ""
echo "[3/4] Parando MQTT Broker..."
cd mqtt-broker
sudo docker compose down
echo "✓ MQTT Broker parado"
cd ..

# Grafana
echo ""
echo "[4/4] Parando Grafana..."
cd grafana
sudo docker compose down
echo "✓ Grafana parado"
cd ../..


echo ""
echo "================================================"
echo "Todos os serviços foram parados!"
echo "================================================"
echo ""