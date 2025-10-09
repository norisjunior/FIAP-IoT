# Inicialização da Plataforma IoT Completa

- MQTT Broker local (mosquitto)
- Node-RED (com plugins para MQTT, dashboard, telegram, entre outros)
- n8n
- InfluxDB
- Grafana

Script completo que cria todos os arquivos de configuração e sobe os serviços: MQTT Broker, Node-RED, n8n, InfluxDB e Grafana em um único docker-compose.

## Pré-requisitos

- WSL2 Ubuntu
- Docker e Docker Compose instalados

## Como usar

### 1. Clonar o git da disciplina
```bash
cd ~
git clone https://github.com/norisjunior/FIAP-IoT
```

### 2. Acessar o diretório que contém o script automático para inicialização dos serviços
```bash
cd FIAP-IoT/infra-MQTT-NodeRED-n8n/
```

### 3. Executar o script
```bash
sudo ./setup-platform.sh
```

O script vai:
- Criar o diretório `IoTStack` com toda a estrutura necessária
- Criar todos os arquivos de configuração (docker-compose.yml, mosquitto.conf, settings.js, .env)
- Subir todos os contêineres em uma única rede Docker

## Serviços disponíveis

- **MQTT Broker**: `localhost:1883` (MQTT) e `localhost:9001` (WebSocket)
- **Node-RED**: http://localhost:1880
  - Usuário: `admin`
  - Senha: `FIAPIoT`
- **n8n**: http://localhost:5678
  - Usuário/senha: gere ao acessar pela primeira vez
- **InfluxDB**: http://localhost:8086
  - Usuário: `admin`
  - Senha: `FIAP@123`
  - Organização: `fiapiot`
  - Bucket: `sensores`
  - Token: `TOKEN_SUPER_SECRETO_1234567890`
- **Grafana**: http://localhost:3000
  - Usuário/senha: `admin/admin` (troque ao acessar pela primeira vez)

## Conexões entre serviços

- **No ESP32 (Wokwi)**: `host.wokwi.internal:1883`
- **Entre contêineres** (Node-RED → MQTT, Node-RED → InfluxDB, Grafana → InfluxDB):
  - MQTT: `mosquitto:1883`
  - InfluxDB: `http://influxdb:8086`

## Parar todos os serviços

```bash
cd IoTStack
sudo docker compose down
```

## Ver logs dos serviços

```bash
sudo docker logs mqtt-broker
sudo docker logs NorisNodeRED
sudo docker logs n8n
sudo docker logs influxdb
sudo docker logs grafana
```

## Troubleshooting

Caso haja algum erro na inicialização de contêineres, pode ser que algum tenha ficado "preso" ou não tenha sido encerrado adequadamente.

Para solucionar:
- Acesse o Docker Desktop
- Verifique se há algum contêiner em execução
  - Caso haja, pare/desligue
- Outra alternativa é a exclusão do contêiner
  - O script (`setup-platform.sh`) baixará e inicializará novamente
- Execute novamente: `sudo ./setup-platform.sh`

## Estrutura de diretórios criada

```
IoTStack/
├── docker-compose.yml          # Orquestração de todos os serviços
├── mqtt-broker/
│   └── mosquitto.conf
├── nodered/
│   └── settings.js
├── n8n/
│   ├── .env
│   ├── n8n_data/              # Dados persistentes do n8n
│   └── pg_data/               # Dados do PostgreSQL
├── influxdb/                  # Volumes gerenciados pelo Docker
└── grafana/                   # Volumes gerenciados pelo Docker
```
