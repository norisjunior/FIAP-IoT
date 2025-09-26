# Deploy N8N na AWS (IP:porta para ensino)

## 1. Criar WSL (ubuntu, por exemplo)

### Criar estrutura de diretórios para o n8n:

```bash
cd ~
mkdir n8n
cd n8n
mkdir -p ./n8n_data ./pg_data
sudo chown -R 1000:1000 ./n8n_data ./pg_data
sudo chmod -R 755 ./n8n_data ./pg_data
```

## 2. Preparar a máquina

- Instalar Docker Desktop para Windows

```bash
# Atualizar sistema
sudo apt update && sudo apt upgrade -y

# Instalar Docker
curl -fsSL https://get.docker.com -o get-docker.sh
sudo sh get-docker.sh
sudo usermod -aG docker $USER

# Instalar Docker Compose
sudo apt install docker-compose-plugin -y

# Logout e login novamente
exit
# ssh novamente...
```

## 3. Criar arquivos (cole o conteúdo dos arquivos)
vim docker-compose.yml
vim .env


## 4. Gerar chave de criptografia

```bash
# Gerar nova chave
openssl rand -base64 32

# Atualizar no .env
vim .env  # Substitua N8N_ENCRYPTION_KEY
```

## 5. Deploy

```bash
# Verificar configuração
docker compose config

# Subir containers
docker compose up -d

# Verificar logs
docker compose logs -f n8n

# Verificar status
docker compose ps
```

## 6. Acesso

```
URL de acesso: http://localhost:5678
Usuário: (crie no primeiro acesso)
```