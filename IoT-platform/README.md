# Inicialização das seguintes plataformas de serviços IoT

- MQTT Broker local (mosquitto)
- Node-RED (com plugins para MQTT, dashboard, telegram, entre outros)
- n8n
- InfluxDB
- Grafana

Script completo que cria todos os arquivos de configuração e sobe os serviços: MQTT Broker, Node-RED, n8n, InfluxDB e Grafana.

## Pré-requisitos

- WSL2 Ubuntu
- Docker e Docker Compose instalados

## Como usar

### 0. Acessar a máquina ubuntu no WSL2 (exemplo com máquina ubuntu já instalada):
```
wsl -d ubuntu
```

- Caso precise/queira instalar a máquina ubuntu no WSL2:
```
wsl --install ubuntu
```


### 1. Clonar o git da disciplina
```bash
cd ~

git clone https://github.com/norisjunior/FIAP-IoT
```

### 2. Acessar o diretório que contém o script automático para inicialização dos serviços
```bash
cd FIAP-IoT/IoT-platform/
```

### 3. Iniciar todos os serviços como uma plataforma integrada
```bash
sudo ./start-linux.sh
```

O script vai:
- Criar o diretório `~/IoTStack` com subdiretórios para cada serviço
- Criar todos os arquivos de configuração (docker-compose.yml, settings.js, .env, etc)
- Fazer build e subir todos os containers em uma única rede Docker

## Serviços disponíveis

- **MQTT Broker**: `mqtt-broker:1883` (MQTT) e `mqtt-broker:9001` (WebSocket)
- **Node-RED**: http://localhost:1880 (usuário: admin / senha: FIAPIoT)
- **n8n**: http://localhost:5678 (usuário/senha: gere ao acessar pela primeira vez)
- **InfluxDB**: http://localhost:8086 (usuário: admin / senha: FIAP@123)
  - Org: fiapiot
  - Bucket: sensores
  - Token: TOKEN_SUPER_SECRETO_1234567890
- **Grafana**: http://localhost:3000 (usuário/senha: admin/admin)

## Configurações de rede

Quando os serviços precisarem se comunicar entre si, use os nomes dos containers:
- No _Node-RED_ ou _n8n_, para acessar o MQTT Broker: `mqtt-broker:1883`
- No _Node-RED_ ou _grafana_, para acessar o InfluxDB: `influxdb:8086`
- No ESP32 (Wokwi), para acessar o MQTT: `host.wokwi.internal:1883`

## Parar todos os serviços

```bash
cd ~/IoTStack && sudo docker compose down
```

Ou usar o script:
```bash
./stop-linux.sh
```

## Troubleshooting

Caso haja algum erro na inicialização de contêineres, pode ser que algum tenha ficado "preso" ou não tenha sido encerrado adequadamente.

Para solucionar:
- Acesse o Docker Desktop
- Verifique se há algum conteiner em execução
    - Caso haja, pare/desligue
- Outra alternativa é a exclusão do conteiner
    - o script (start-linux.sh) baixará e inicializará novamente
- Execute novamente o passo relativo ao script ```start-linux.sh```