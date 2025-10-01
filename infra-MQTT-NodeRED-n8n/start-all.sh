#!/bin/bash

# Script completo para configurar e subir MQTT Broker, Node-RED e n8n
# WSL2 Ubuntu

set -e

echo "================================================"
echo "Configurando e iniciando todos os serviços..."
echo "================================================"

mkdir IoTStack
cd IoTStack

# ============================================
# MQTT BROKER
# ============================================
echo ""
echo "[1/3] Configurando MQTT Broker..."

mkdir -p mqtt-broker
cd mqtt-broker

# Criar docker-compose.yml
cat > docker-compose.yml << 'EOF'
services:
  mosquitto:
    image: eclipse-mosquitto:latest
    container_name: mqtt-broker
    ports:
      - "1883:1883"
      - "9001:9001"
    volumes:
      - ./mosquitto.conf:/mosquitto/config/mosquitto.conf
    restart: unless-stopped
EOF

# Criar mosquitto.conf
cat > mosquitto.conf << 'EOF'
listener 1883
allow_anonymous true
EOF

echo "✓ Arquivos do MQTT Broker criados"
echo "  Subindo MQTT Broker..."
sudo docker compose up -d
echo "✓ MQTT Broker iniciado"
cd ..


# ============================================
# NODE-RED
# ============================================
echo ""
echo "[2/3] Configurando Node-RED..."

mkdir -p nodered
cd nodered

# Criar docker-compose.yml
cat > docker-compose.yml << 'EOF'
services:
  node-red:
    image: norisjunior/nodered-norisiot:latest
    container_name: NorisNodeRED
    ports:
      - "1880:1880"
      - "3456:3456"
    environment:
      - TZ=America/Sao_Paulo
    restart: unless-stopped
EOF

echo "✓ Arquivos do Node-RED criados"
echo "  Subindo Node-RED..."
sudo docker compose up -d
echo "✓ Node-RED iniciado"
cd ..

# ============================================
# N8N
# ============================================
echo ""
echo "[3/3] Configurando n8n..."

mkdir -p n8n
cd n8n

# Criar estrutura de diretórios
mkdir -p ./n8n_data ./pg_data
sudo chown -R 1000:1000 ./n8n_data ./pg_data
sudo chmod -R 755 ./n8n_data ./pg_data

# Criar arquivo .env
cat > .env << 'EOF'
# Configurações do Banco de Dados
POSTGRES_DB=n8n
POSTGRES_USER=n8nuser
POSTGRES_PASSWORD=minha_senha_super_secreta

# Configurações do N8N para LOCALHOST
N8N_DOMAIN=localhost
N8N_PROTOCOL=http
N8N_PORT=5678

# Chave de criptografia (gere uma nova com: openssl rand -base64 32)
N8N_ENCRYPTION_KEY=XiCegB07AWfY5H7DxNrVrHkUuEe9uUs3
EOF

# Criar docker-compose.yml
cat > docker-compose.yml << 'EOF'
services:
  postgres:
    image: postgres:15
    restart: always
    environment:
      POSTGRES_DB: ${POSTGRES_DB:-n8n}
      POSTGRES_USER: ${POSTGRES_USER:-n8nuser}
      POSTGRES_PASSWORD: ${POSTGRES_PASSWORD}
    volumes:
      - ./pg_data:/var/lib/postgresql/data
    networks:
      - n8n_backend
    healthcheck:
      test: ['CMD-SHELL', 'pg_isready -U ${POSTGRES_USER:-n8nuser} -d ${POSTGRES_DB:-n8n}']
      interval: 5s
      timeout: 5s
      retries: 5

  n8n:
    image: n8nio/n8n:latest
    restart: always
    user: "1000:1000"  # Força o usuário correto
    environment:
      DB_TYPE: postgresdb
      DB_POSTGRESDB_HOST: postgres
      DB_POSTGRESDB_PORT: 5432
      DB_POSTGRESDB_DATABASE: ${POSTGRES_DB:-n8n}
      DB_POSTGRESDB_USER: ${POSTGRES_USER:-n8nuser}
      DB_POSTGRESDB_PASSWORD: ${POSTGRES_PASSWORD}
      N8N_HOST: ${N8N_DOMAIN}
      N8N_PORT: 5678
      N8N_PROTOCOL: ${N8N_PROTOCOL}
      WEBHOOK_URL: ${N8N_PROTOCOL}://${N8N_DOMAIN}:${N8N_PORT}
      GENERIC_TIMEZONE: America/Sao_Paulo
      N8N_ENCRYPTION_KEY: ${N8N_ENCRYPTION_KEY}
      N8N_LOG_LEVEL: info
      N8N_RUNNERS_ENABLED: true
    ports:
      - '${N8N_PORT}:5678'  # Expõe para qualquer interface
    volumes:
      - ./n8n_data:/home/node/.n8n
    networks:
      - n8n_backend
    depends_on:
      postgres:
        condition: service_healthy

networks:
  n8n_backend:
    driver: bridge
EOF

echo "✓ Arquivos do n8n criados"
echo "  Subindo n8n..."
sudo docker compose up -d
echo "✓ n8n iniciado"
cd ..


# ============================================
# FINALIZAÇÃO
# ============================================
echo ""
echo "================================================"
echo "Todos os serviços configurados e iniciados!"
echo "================================================"
echo ""
echo "MQTT Broker: porta 1883 (MQTT) e 9001 (WebSocket)"
echo "Node-RED:    http://localhost:1880"
echo "n8n:         http://localhost:5678"
echo ""
echo "Verificar logs:"
echo "  sudo docker logs mqtt-broker"
echo "  sudo docker logs NorisNodeRED"
echo "  sudo docker logs n8n-n8n-1"
echo ""
echo "Diretórios criados:"
echo "  ~/mqtt-broker"
echo "  ~/nodered"
echo "  ~/n8n"
echo ""