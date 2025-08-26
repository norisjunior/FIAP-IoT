# Instalar NodeRED usando docker (Raspberry Pi OS 64-bit)

## Primeiro passo: INSTALAR DOCKER no raspberry:
https://docs.docker.com/engine/install/debian/


## Segundo passo: COMPILAR NODERED
- Criar diretório (ex. IoTArchitecture/build/NodeRED)
- Entrar no diretório (cd IoTArchitecture/build/NodeRED) e criar esses dois arquivos:
    - Dockerfile (conteúdo do arquivo NodeRED-Dockerfile)
    - docker-compose.yml (conteúdo do arquivo NodeRED-docker-compose.yml)

Depois de criar os arquivos, inserir o conteúdo conforme consta no diretório "docker_configs"

Com os arquivos configurados:
sudo docker compose build

- Pegar o ID da imagem compilada:
sudo docker images
- Taggear
sudo docker image tag 11ca7bf0ba32 nodered-rasp

Usar o nome "nodered-rasp" no docker-compose.yml a ser criado adiante.

## Terceiro passo: ESTRUTURAR OS CONTAINERES DO NEAR EDGE:

- Criar e entrar no diretório (IoTArchitecture/NearEdge)
- Criar arquivo `docker-compose.yml`
- Usar as configurações que constam no arquivo NearEdge-docker-compose.yml


## Acesso aos recursos:

### NodeRED:
http://<IP>:1880

### InfluxDB local:
http://<IP>:8086