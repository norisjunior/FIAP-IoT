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
cd IoTStack
sudo docker compose down
cd ..

echo ""
echo "================================================"
echo "Todos os serviços foram parados!"
echo "================================================"
echo ""