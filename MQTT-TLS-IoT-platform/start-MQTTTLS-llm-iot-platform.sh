#!/bin/bash

# Script completo para configurar e subir MQTT Broker (com TLS), Node-RED, n8n, Grafana e InfluxDB
# WSL2 Ubuntu - Docker Compose único
# MQTT: porta 1883 (user/pass) + porta 8883 (TLS + user/pass)
# Certificados gerados automaticamente em tempo de execução

set -e

echo "================================================"
echo "Configurando e iniciando plataforma IoT (TLS)..."
echo "================================================"

mkdir -p LLMIoTStack
cd LLMIoTStack

# ============================================
# ESTRUTURA DE DIRETÓRIOS
# ============================================
echo ""
echo "Criando estrutura de diretórios..."

mkdir -p mqtt-broker/certs
mkdir -p nodered
mkdir -p n8n/n8n_data n8n/pg_data
mkdir -p influxdb
mkdir -p grafana
mkdir -p ollama

sudo chown -R 1000:1000 ./n8n/n8n_data ./n8n/pg_data
sudo chmod -R 755 ./n8n/n8n_data ./n8n/pg_data

echo "✓ Estrutura criada"

# ============================================
# GERAÇÃO DE CERTIFICADOS TLS (em tempo de execução)
# ============================================
echo ""
echo "Gerando certificados TLS..."

# 1. CA (Autoridade Certificadora)
echo "  Gerando chave e certificado da CA..."
openssl genrsa -out mqtt-broker/certs/ca.key 2048 2>/dev/null
openssl req -new -x509 -days 3650 \
  -key mqtt-broker/certs/ca.key \
  -out mqtt-broker/certs/ca.crt \
  -subj "/CN=FIAP-IoT-CA/O=FIAP IoT/OU=Plataforma IoT/C=BR" 2>/dev/null
echo "  ✓ CA gerada (válida por 10 anos)"

# 2. Chave e CSR do servidor
echo "  Gerando chave do servidor MQTT..."
openssl genrsa -out mqtt-broker/certs/server.key 2048 2>/dev/null
openssl req -new \
  -key mqtt-broker/certs/server.key \
  -out mqtt-broker/certs/server.csr \
  -subj "/CN=mqtt-broker/O=FIAP IoT/OU=Broker/C=BR" 2>/dev/null

# 3. Assinar certificado do servidor com a CA (inclui SANs para localhost e IPs locais)
echo "  Assinando certificado do servidor com a CA..."
cat > mqtt-broker/certs/server_ext.cnf << 'EXTEOF'
[v3_req]
subjectAltName = IP:127.0.0.1,DNS:localhost,DNS:mqtt-broker
EXTEOF

openssl x509 -req -days 3650 \
  -in mqtt-broker/certs/server.csr \
  -CA mqtt-broker/certs/ca.crt \
  -CAkey mqtt-broker/certs/ca.key \
  -CAcreateserial \
  -out mqtt-broker/certs/server.crt \
  -extensions v3_req \
  -extfile mqtt-broker/certs/server_ext.cnf 2>/dev/null
echo "  ✓ Certificado do servidor assinado (válido por 10 anos)"

# Ajusta permissões para o usuário mosquitto (1883) dentro do container
sudo chown -R 1883:1883 mqtt-broker/certs/
sudo chmod 640 mqtt-broker/certs/ca.key mqtt-broker/certs/server.key
sudo chmod 644 mqtt-broker/certs/ca.crt mqtt-broker/certs/server.crt

echo "✓ Certificados TLS gerados em mqtt-broker/certs/"

# ============================================
# CONFIGURAÇÕES INDIVIDUAIS
# ============================================

# MQTT Broker
echo ""
echo "Configurando MQTT Broker com autenticação, ACL e TLS..."
cat > mqtt-broker/mosquitto.conf << 'EOF'
# ─── Listener não-TLS (porta padrão) ───────────────────────
listener 1883
persistence true
persistence_location /mosquitto/data/

# Autenticação
allow_anonymous false
password_file /mosquitto/config/passwd

# Controle de acesso por tópico
acl_file /mosquitto/config/acl

# ─── Listener TLS (porta 8883) ─────────────────────────────
listener 8883
cafile   /mosquitto/config/certs/ca.crt
certfile /mosquitto/config/certs/server.crt
keyfile  /mosquitto/config/certs/server.key
tls_version tlsv1.2

# Reutiliza autenticação e ACL no listener TLS
allow_anonymous false
password_file /mosquitto/config/passwd
acl_file /mosquitto/config/acl
EOF

cat > mqtt-broker/acl << 'EOF'
# Arquivo de ACL do Mosquitto
# Admin com acesso total (pré-configurado)
user admin
topic readwrite #
#
# Adicione regras de acesso abaixo seguindo o formato:
#   user <nome_do_usuario>
#   topic read|write|readwrite <topico>
#
# Exemplo:
#   user device1
#   topic write FIAPIoT/smartagro/sensores/#
EOF

# Cria arquivo de senhas vazio (será populado depois)
install -m 0600 /dev/null mqtt-broker/passwd

sudo chown 1883:1883 mqtt-broker/mosquitto.conf mqtt-broker/acl mqtt-broker/passwd

echo "✓ MQTT configurado (1883 user/pass | 8883 TLS + user/pass)"

# Node-RED
echo ""
echo "Configurando Node-RED..."
NODERED_PASSWORD="FIAPIoT"
echo "  Gerando hash da senha..."
NODERED_HASH=$(docker run --rm python:3.9-slim sh -c "pip install -q --no-warn-script-location bcrypt 2>/dev/null && python -c \"import bcrypt; print(bcrypt.hashpw(b'$NODERED_PASSWORD', bcrypt.gensalt(rounds=8)).decode())\"")

cat > nodered/settings.js << EOF
module.exports = {
    adminAuth: {
        type: "credentials",
        users: [{
            username: "admin",
            password: "$NODERED_HASH",
            permissions: "*"
        }]
    }
}
EOF
echo "✓ Node-RED configurado"

# n8n
echo ""
echo "Configurando n8n..."
cat > n8n/.env << 'EOF'
# Configurações do Banco de Dados
POSTGRES_DB=n8n
POSTGRES_USER=n8nuser
POSTGRES_PASSWORD=minha_senha_super_secreta

# Configurações do N8N para LOCALHOST
N8N_DOMAIN=localhost
N8N_PROTOCOL=http
N8N_PORT=5678

# Chave de criptografia
N8N_ENCRYPTION_KEY=XiCegB07AWfY5H7DxNrVrHkUuEe9uUs3
EOF
echo "✓ n8n configurado"

# PostgreSQL - Script de inicialização para pgvector
echo ""
echo "Configurando PostgreSQL com pgvector..."
mkdir -p n8n/pg_init
cat > n8n/pg_init/init.sql << 'EOF'
-- Habilita a extensão pgvector para suporte a vector database
CREATE EXTENSION IF NOT EXISTS vector;
EOF
echo "✓ PostgreSQL configurado com suporte a vector database"

# ============================================
# DOCKER COMPOSE ÚNICO
# ============================================
echo ""
echo "Criando docker-compose.yml único..."

cat > docker-compose.yml << 'EOF'
services:
  # Ollama
  ollama:
    image: ollama/ollama:latest
    container_name: ollama
    ports:
      - "11434:11434"
    volumes:
      - ./ollama:/root/.ollama
    restart: unless-stopped
    networks:
      - iot-network
    entrypoint: ["/bin/bash", "-c"]
    command: >
      "ollama serve &
      pid=$!;
      sleep 5;
      ollama pull llama3.2;
      ollama pull nomic-embed-text;
      wait $pid"

  # MQTT Broker
  mosquitto:
    image: eclipse-mosquitto:latest
    container_name: mqtt-broker
    ports:
      - "1883:1883"   # MQTT sem TLS (user/pass)
      - "8883:8883"   # MQTT com TLS
      - "9001:9001"
    volumes:
      - ./mqtt-broker:/mosquitto/config
    restart: unless-stopped
    networks:
      - iot-network
    depends_on:
      - ollama

  # Node-RED
  node-red:
    image: norisjunior/nodered-norisiot:latest
    container_name: NorisNodeRED
    ports:
      - "1880:1880"
      - "3456:3456"
    environment:
      - TZ=America/Sao_Paulo
    volumes:
      - ./nodered/settings.js:/data/settings.js
    restart: unless-stopped
    networks:
      - iot-network
    depends_on:
      - mosquitto
      - influxdb

  # PostgreSQL para n8n e vector database
  postgres:
    image: pgvector/pgvector:pg15
    container_name: n8n-postgres
    restart: always
    environment:
      POSTGRES_DB: n8n
      POSTGRES_USER: n8nuser
      POSTGRES_PASSWORD: minha_senha_super_secreta
    volumes:
      - ./n8n/pg_data:/var/lib/postgresql/data
      - ./n8n/pg_init:/docker-entrypoint-initdb.d
    networks:
      - iot-network
    healthcheck:
      test: ['CMD-SHELL', 'pg_isready -U n8nuser -d n8n']
      interval: 5s
      timeout: 5s
      retries: 5

  # n8n
  n8n:
    image: n8nio/n8n:latest
    container_name: n8n
    restart: always
    user: "1000:1000"
    environment:
      DB_TYPE: postgresdb
      DB_POSTGRESDB_HOST: postgres
      DB_POSTGRESDB_PORT: 5432
      DB_POSTGRESDB_DATABASE: n8n
      DB_POSTGRESDB_USER: n8nuser
      DB_POSTGRESDB_PASSWORD: minha_senha_super_secreta
      N8N_HOST: localhost
      N8N_PORT: 5678
      N8N_PROTOCOL: http
      WEBHOOK_URL: http://localhost:5678
      GENERIC_TIMEZONE: America/Sao_Paulo
      N8N_ENCRYPTION_KEY: XiCegB07AWfY5H7DxNrVrHkUuEe9uUs3
      N8N_LOG_LEVEL: info
      N8N_RUNNERS_ENABLED: true
      N8N_SECURE_COOKIE: false
    ports:
      - '5678:5678'
    volumes:
      - ./n8n/n8n_data:/home/node/.n8n
    networks:
      - iot-network
    depends_on:
      postgres:
        condition: service_healthy
      ollama:
        condition: service_started

  # InfluxDB
  influxdb:
    image: influxdb:2.7
    container_name: influxdb
    ports:
      - "8086:8086"
    environment:
      - DOCKER_INFLUXDB_INIT_MODE=setup
      - DOCKER_INFLUXDB_INIT_USERNAME=admin
      - DOCKER_INFLUXDB_INIT_PASSWORD=FIAP@123
      - DOCKER_INFLUXDB_INIT_ORG=fiapiot
      - DOCKER_INFLUXDB_INIT_BUCKET=sensores
      - DOCKER_INFLUXDB_INIT_RETENTION=30d
      - DOCKER_INFLUXDB_INIT_ADMIN_TOKEN=TOKEN_SUPER_SECRETO_1234567890
    volumes:
      - influxdb-data:/var/lib/influxdb2
      - influxdb-config:/etc/influxdb2
    restart: unless-stopped
    networks:
      - iot-network

  # Grafana
  grafana:
    image: grafana/grafana:latest
    container_name: grafana
    ports:
      - '3000:3000'
    restart: unless-stopped
    volumes:
      - grafana-storage:/var/lib/grafana
    networks:
      - iot-network
    depends_on:
      - influxdb

networks:
  iot-network:
    driver: bridge

volumes:
  influxdb-data:
  influxdb-config:
  grafana-storage:
EOF

echo "✓ docker-compose.yml criado"

# ============================================
# INICIALIZAR SERVIÇOS
# ============================================
echo ""
echo "Atualizando imagem do n8n..."
sudo docker pull n8nio/n8n:latest

echo ""
echo "Subindo todos os serviços..."
sudo docker compose up -d

echo ""
echo "Criando usuário admin no MQTT Broker..."
sleep 10
sudo docker exec mqtt-broker mosquitto_passwd -b /mosquitto/config/passwd admin FIAP1234
sleep 5
sudo docker restart mqtt-broker
echo "✓ Usuário MQTT admin criado (admin/FIAP1234)"

# ============================================
# RESUMO FINAL
# ============================================
echo ""
echo "================================================"
echo "Plataforma IoT (TLS) configurada e iniciada!"
echo "================================================"
echo ""
echo "Ollama:      http://localhost:11434 (Servidor LLM local)"
echo "Node-RED:    http://localhost:1880 (usuário: admin / senha: $NODERED_PASSWORD)"
echo "n8n:         http://localhost:5678 (usuário/senha: gere ao acessar pela primeira vez)"
echo "PostgreSQL:  localhost:5432 (n8n + vector database com pgvector habilitado)"
echo "  Host: n8n-postgres"
echo "  Database: n8n"
echo "  User: n8nuser"
echo "  Password: minha_senha_super_secreta"
echo "InfluxDB:    http://localhost:8086 (usuário: admin / senha: FIAP@123)"
echo "  Org:       fiapiot"
echo "  Bucket:    sensores"
echo "  Token:     TOKEN_SUPER_SECRETO_1234567890"
echo "Grafana:     http://localhost:3000 (usuário/senha: admin/admin)"
echo ""
echo "MQTT Broker:"
echo "  Porta 1883 → sem TLS  (user/pass)"
echo "  Porta 8883 → com TLS  (user/pass)"
echo "  Usuário admin: admin / FIAP1234"
echo ""
echo "Mosquitto MQTT Broker - Orientações para criação de users/ACL:"
echo ""
echo "  ┌─────────────────────────────────────────────────────────────┐"
echo "  │  PASSO 1 - Criar usuários e senhas no broker               │"
echo "  ├─────────────────────────────────────────────────────────────┤"
echo "  │                                                             │"
echo "  │    sudo docker exec -it mqtt-broker \\                       │"
echo "  │      mosquitto_passwd /mosquitto/config/passwd device1      │"
echo "  │    sudo docker exec -it mqtt-broker \\                       │"
echo "  │      mosquitto_passwd /mosquitto/config/passwd device2      │"
echo "  │                                                             │"
echo "  └─────────────────────────────────────────────────────────────┘"
echo ""
echo "  ┌─────────────────────────────────────────────────────────────┐"
echo "  │  PASSO 2 - Configurar ACL (quem publica/assina onde)       │"
echo "  ├─────────────────────────────────────────────────────────────┤"
echo "  │  Edite o arquivo mqtt-broker/acl e adicione as regras:      │"
echo "  │                                                             │"
echo "  │    user device1                                             │"
echo "  │    topic write FIAPIoT/smartagro/#                          │"
echo "  │                                                             │"
echo "  │    user device2                                             │"
echo "  │    topic read FIAPIoT/smartagro/cmd/#                       │"
echo "  └─────────────────────────────────────────────────────────────┘"
echo ""
echo "  ┌─────────────────────────────────────────────────────────────┐"
echo "  │  PASSO 3 - Reiniciar o broker para aplicar as mudanças      │"
echo "  ├─────────────────────────────────────────────────────────────┤"
echo "  │    sudo docker restart mqtt-broker                          │"
echo "  └─────────────────────────────────────────────────────────────┘"
echo ""
echo "════════════════════════════════════════════════════════════════"
echo "  CERTIFICADO TLS - Chave Pública da CA (para os alunos)"
echo "════════════════════════════════════════════════════════════════"
echo ""
echo "  Os alunos devem copiar o conteúdo abaixo e salvar como ca.crt"
echo "  para configurar o cliente MQTT com TLS."
echo ""
echo "  Comando para exibir a chave pública da CA a qualquer momento:"
echo "    cat LLMIoTStack/mqtt-broker/certs/ca.crt"
echo ""
echo "  ── Início do certificado CA ────────────────────────────────"
cat mqtt-broker/certs/ca.crt
echo "  ── Fim do certificado CA ───────────────────────────────────"
echo ""
echo "  Exemplo de conexão com TLS usando mosquitto_pub/sub:"
echo "    mosquitto_pub -h localhost -p 8883 \\"
echo "      --cafile ca.crt \\"
echo "      -u admin -P FIAP1234 \\"
echo "      -t FIAPIoT/teste -m 'hello TLS'"
echo ""
echo "Verificar logs:"
echo "  sudo docker logs ollama"
echo "  sudo docker logs mqtt-broker"
echo "  sudo docker logs NorisNodeRED"
echo "  sudo docker logs n8n"
echo "  sudo docker logs n8n-postgres"
echo "  sudo docker logs influxdb"
echo "  sudo docker logs grafana"
echo ""
echo "Parar todos os serviços:"
echo "  cd LLMIoTStack && sudo docker compose down"
echo ""
