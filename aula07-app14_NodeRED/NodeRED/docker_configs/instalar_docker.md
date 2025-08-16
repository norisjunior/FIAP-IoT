# Instalar NodeRED usando docker

## Liberar a porta 1880 para a instância EC2 na sua VPS
Pelo console da AWS:
EC2 → Security Groups → selecione o SG da instância → Edit inbound rules.
Add rule → Type: Custom TCP → Port range: 1880 → Source: My IP (recomendado) ou 0.0.0.0/0 (público) → Save rules.

## Primieiro passo: INSTALAR DOCKER

### Pré-requisitos
sudo apt-get update
sudo apt-get install -y ca-certificates curl gnupg


### Adicionar o repositório oficial do Docker
sudo install -m 0755 -d /etc/apt/keyrings
sudo curl -fsSL https://download.docker.com/linux/ubuntu/gpg -o /etc/apt/keyrings/docker.asc
sudo chmod a+r /etc/apt/keyrings/docker.asc

echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] \
  https://download.docker.com/linux/ubuntu \
  $(. /etc/os-release && echo ${UBUNTU_CODENAME:-$VERSION_CODENAME}) stable" | \
  sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

sudo apt-get update

### Instalar o Docker Engine, CLI, containerd e plugins (inclui Compose v2)
sudo apt-get install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin


### Habilitar e testar
sudo systemctl enable --now docker
sudo docker run --rm hello-world


## Segundo passo: COMPILAR NODERED
- Criar diretório (ex. IoTNodeRED)
- Entrar no diretório (cd IoTNodeRED) e criar esses dois arquivos:
    - Dockerfile
    - docker-compose.yml

Depois de criar os arquivos, inserir o conteúdo conforme consta no diretório "docker_configs"

Com os arquivos configurados:
sudo docker compose build
sudo docker compose up -d
