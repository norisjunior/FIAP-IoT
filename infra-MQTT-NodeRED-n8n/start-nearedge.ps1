# Script completo para configurar e subir MQTT Broker, Node-RED, n8n, Grafana e InfluxDB
# Windows 11 - Docker Compose unico

$ErrorActionPreference = "Stop"

Write-Host "================================================" -ForegroundColor Cyan
Write-Host "Configurando e iniciando plataforma IoT..." -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan

if (Test-Path "IoTStack") {
    Write-Host "Removendo diretorio IoTStack existente..." -ForegroundColor Yellow
    Remove-Item -Path "IoTStack" -Recurse -Force
}

New-Item -ItemType Directory -Path "IoTStack" | Out-Null
Set-Location "IoTStack"

# ============================================
# ESTRUTURA DE DIRETORIOS
# ============================================
Write-Host ""
Write-Host "Criando estrutura de diretorios..." -ForegroundColor Yellow

New-Item -ItemType Directory -Path "mqtt-broker" -Force | Out-Null
New-Item -ItemType Directory -Path "nodered" -Force | Out-Null
New-Item -ItemType Directory -Path "n8n\n8n_data" -Force | Out-Null
New-Item -ItemType Directory -Path "n8n\pg_data" -Force | Out-Null
New-Item -ItemType Directory -Path "influxdb" -Force | Out-Null
New-Item -ItemType Directory -Path "grafana" -Force | Out-Null

Write-Host "[OK] Estrutura criada" -ForegroundColor Green

# ============================================
# CONFIGURACOES INDIVIDUAIS
# ============================================

# MQTT Broker
#Write-Host ""
#Write-Host "Configurando MQTT Broker..." -ForegroundColor Yellow
#$mosquittoConf = @"
#listener 1883
#allow_anonymous true
#"@
#$mosquittoConf | Out-File -FilePath "mqtt-broker\mosquitto.conf" -Encoding UTF8
#Write-Host "[OK] MQTT configurado" -ForegroundColor Green
# MQTT Broker
Write-Host ""
Write-Host "Configurando MQTT Broker..." -ForegroundColor Yellow
$mosquittoConf = @"
listener 1883
allow_anonymous true
"@
[System.IO.File]::WriteAllText("$PWD\mqtt-broker\mosquitto.conf", $mosquittoConf, [System.Text.UTF8Encoding]::new($false))
Write-Host "[OK] MQTT configurado" -ForegroundColor Green


# Node-RED
Write-Host ""
Write-Host "Configurando Node-RED..." -ForegroundColor Yellow
$NODERED_PASSWORD = "FIAPIoT"
Write-Host "  Gerando hash da senha..." -ForegroundColor Gray

# Criar script Python temporario
$pythonScript = @"
import bcrypt
import sys
password = sys.argv[1]
hash_result = bcrypt.hashpw(password.encode(), bcrypt.gensalt(rounds=8))
print(hash_result.decode())
"@
$pythonScript | Out-File -FilePath "gen_hash.py" -Encoding UTF8

# Executar via Docker
$NODERED_HASH = docker run --rm -v "${PWD}:/work" -w /work python:3.9-slim sh -c "pip install -q bcrypt 2>/dev/null && python gen_hash.py $NODERED_PASSWORD"

# Remover script temporario
Remove-Item "gen_hash.py" -Force

$noderedSettings = @"
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
"@
$noderedSettings | Out-File -FilePath "nodered\settings.js" -Encoding UTF8
Write-Host "[OK] Node-RED configurado" -ForegroundColor Green

# n8n
Write-Host ""
Write-Host "Configurando n8n..." -ForegroundColor Yellow
$n8nEnv = @"
POSTGRES_DB=n8n
POSTGRES_USER=n8nuser
POSTGRES_PASSWORD=minha_senha_super_secreta
N8N_DOMAIN=localhost
N8N_PROTOCOL=http
N8N_PORT=5678
N8N_ENCRYPTION_KEY=XiCegB07AWfY5H7DxNrVrHkUuEe9uUs3
"@
$n8nEnv | Out-File -FilePath "n8n\.env" -Encoding UTF8
Write-Host "[OK] n8n configurado" -ForegroundColor Green

# ============================================
# DOCKER COMPOSE UNICO
# ============================================
Write-Host ""
Write-Host "Criando docker-compose.yml unico..." -ForegroundColor Yellow

$dockerCompose = @"
services:
  mosquitto:
    image: eclipse-mosquitto:latest
    container_name: mqtt-broker
    ports:
      - "1883:1883"
      - "9001:9001"
    volumes:
      - ./mqtt-broker/mosquitto.conf:/mosquitto/config/mosquitto.conf
    restart: unless-stopped
    networks:
      - iot-network

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

  postgres:
    image: postgres:15
    container_name: n8n-postgres
    restart: always
    environment:
      POSTGRES_DB: n8n
      POSTGRES_USER: n8nuser
      POSTGRES_PASSWORD: minha_senha_super_secreta
    volumes:
      - ./n8n/pg_data:/var/lib/postgresql/data
    networks:
      - iot-network
    healthcheck:
      test: ['CMD-SHELL', 'pg_isready -U n8nuser -d n8n']
      interval: 5s
      timeout: 5s
      retries: 5

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
    ports:
      - '5678:5678'
    volumes:
      - ./n8n/n8n_data:/home/node/.n8n
    networks:
      - iot-network
    depends_on:
      postgres:
        condition: service_healthy

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
"@
$dockerCompose | Out-File -FilePath "docker-compose.yml" -Encoding UTF8

Write-Host "[OK] docker-compose.yml criado" -ForegroundColor Green

# ============================================
# INICIALIZAR SERVICOS
# ============================================
Write-Host ""
Write-Host "Subindo todos os servicos..." -ForegroundColor Yellow
docker compose up -d

Write-Host ""
Write-Host "================================================" -ForegroundColor Cyan
Write-Host "Plataforma IoT configurada e iniciada!" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "MQTT Broker: porta 1883 (MQTT) e 9001 (WebSocket)" -ForegroundColor White
Write-Host "Node-RED:    http://localhost:1880 (usuario: admin / senha: $NODERED_PASSWORD)" -ForegroundColor White
Write-Host "n8n:         http://localhost:5678 (usuario/senha: gere ao acessar)" -ForegroundColor White
Write-Host "InfluxDB:    http://localhost:8086 (usuario: admin / senha: FIAP@123)" -ForegroundColor White
Write-Host "  Org:       fiapiot" -ForegroundColor Gray
Write-Host "  Bucket:    sensores" -ForegroundColor Gray
Write-Host "  Token:     TOKEN_SUPER_SECRETO_1234567890" -ForegroundColor Gray
Write-Host "Grafana:     http://localhost:3000 (usuario/senha: admin/admin)" -ForegroundColor White
Write-Host ""
Write-Host "Verificar logs:" -ForegroundColor Yellow
Write-Host "  docker logs mqtt-broker" -ForegroundColor Gray
Write-Host "  docker logs NorisNodeRED" -ForegroundColor Gray
Write-Host "  docker logs n8n" -ForegroundColor Gray
Write-Host "  docker logs influxdb" -ForegroundColor Gray
Write-Host "  docker logs grafana" -ForegroundColor Gray
Write-Host ""
Write-Host "Parar todos os servicos:" -ForegroundColor Yellow
Write-Host "  cd IoTStack" -ForegroundColor Gray
Write-Host "  docker compose down" -ForegroundColor Gray
Write-Host ""