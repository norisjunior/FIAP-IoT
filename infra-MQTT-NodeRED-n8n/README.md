# Inicialização das seguintes plataformas de serviços IoT

- MQTT Broker local (mosquitto)
- Node-RED (com plugins para MQTT, dashboard, telegram, entre outros)
- n8n
- grafana

Script completo que cria todos os arquivos de configuração e sobe os serviços: MQTT Broker, Node-RED, n8n e grafana.

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
sudo ./start-all.sh
```

O script vai:
- Criar os diretórios `~/mqtt-broker`, `~/nodered` e `~/n8n`
- Criar todos os arquivos de configuração (docker-compose.yml, Dockerfile, .env, etc)
- Fazer build e subir todos os containeres

## Serviços disponíveis

- **MQTT Broker**: `localhost:1883` (MQTT) e `localhost:9001` (WebSocket)
- **Node-RED**: http://localhost:1880
- **n8n**: http://localhost:5678
- **grafana**: http://localhost:3000


- No ESP32, ao usar MQTT: host.wokwi.internal, porta 1883
- No Node-RED e n8n, quando se referir ao MQTT local: host.docker.internal, porta 1883

## Parar todos os serviços

```bash
./stop-all.sh
```

Ou manualmente em cada diretório:
```bash
cd IoTStack/mqtt-broker && sudo docker compose down
cd IoTStack/nodered && sudo docker compose down
cd IoTStack/n8n && sudo docker compose down
cd IoTStack/grafana && sudo docker compose down
```



## Troubleshooting

Caso haja algum erro na inicialização de contêineres, pode ser que algum tenha ficado "preso" ou não tenha sido encerrado adequadamente.

Para solucionar:
- Acesse o Docker Desktop
- Verifique se há algum conteiner em execução
    - Caso haja, pare/desligue
- Outra alternativa é a exclusão do conteiner
    - o script (start-all.sh) baixará e inicializará novamente
- Execute novamente o passo relativo ao script ```start-all.sh```

