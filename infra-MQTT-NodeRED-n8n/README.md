# Inicialização das seguintes plataformas de serviços IoT

- MQTT Broker local (mosquitto)
- Node-RED (com plugins para MQTT, dashboard, telegram, entre outros)
- n8n

Script completo que cria todos os arquivos de configuração e sobe os serviços: MQTT Broker, Node-RED e n8n.

## Pré-requisitos

- WSL2 Ubuntu
- Docker e Docker Compose instalados

## Como usar

### 1. Salvar o script
```bash
cd ~
nano setup-all.sh
# Cole o conteúdo do script e salve (Ctrl+O, Enter, Ctrl+X)
```

### 2. Dar permissão de execução
```bash
chmod +x setup-all.sh
```

### 3. Executar
```bash
./setup-all.sh
```

O script vai:
- Criar os diretórios `~/mqtt-broker`, `~/nodered` e `~/n8n`
- Criar todos os arquivos de configuração (docker-compose.yml, Dockerfile, .env, etc)
- Fazer build e subir todos os containeres

## Serviços disponíveis

- **MQTT Broker**: `localhost:1883` (MQTT) e `localhost:9001` (WebSocket)
- **Node-RED**: http://localhost:1880
- **n8n**: http://localhost:5678


- No ESP32, ao usar MQTT: host.wokwi.internal, porta 1883
- No Node-RED e n8n, quando se referir ao MQTT local: host.docker.internal, porta 1883

## Parar todos os serviços

```bash
./stop-all.sh
```

Ou manualmente em cada diretório:
```bash
cd ~/mqtt-broker && sudo docker compose down
cd ~/nodered && sudo docker compose down
cd ~/n8n && sudo docker compose down
```



