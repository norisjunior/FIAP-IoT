# Instalar MQTT Broker (Mosquitto) usando Docker

## Primeiro passo: Criar estrutura de diretórios
```
mkdir -p ~/mqtt-broker
cd ~/mqtt-broker
```

## Segundo passo: Criar arquivos de configuração

### Criar docker-compose.yml
```
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
```

### Criar arquivo mosquitto.conf (mesmo diretório do docker-compose)
```
listener 1883
allow_anonymous true
```

## Terceiro passo: Subir o broker
```
sudo docker compose up -d
```

## Testar conexão
```
sudo docker logs mqtt-broker
```

## Configuração no ESP32 + Wokwi + VSCode + PlatformIO
Host: host.wokwi.internal
Port: 1883



## Configuração no NodeRED
Host: host.docker.internal
Port: 1883



## Comandos úteis

### Parar o broker
```
sudo docker compose down
```

# Ver logs
```
sudo docker logs -f mqtt-broker
```

# Reiniciar
```
sudo docker compose restart
```
