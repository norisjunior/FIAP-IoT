# Deploy N8N na AWS (IP:porta para ensino)

## 1. Criar instância EC2

### Configuração:
- **AMI:** Ubuntu 22.04 LTS
- **Tipo:** t3.micro (Free Tier) ou t3.small
- **Storage:** 20GB GP3
- **Key Pair:** Crie ou use existente para SSH

## 2. Configurar Security Group

**MUITO IMPORTANTE:** Configure as regras de entrada:

| Tipo | Protocolo | Porta | Origem    | Descrição             |
|------|-----------|-------|-----------|-----------------------|
| SSH  | TCP       | 22    | Seu IP    | SSH access            |
| HTTP | TCP       | 5678  | 0.0.0.0/0 | N8N Web UI            |
| HTTP | TCP       | 80    | 0.0.0.0/0 | HTTP geral (opcional) |

### Via Console AWS:
1. EC2 > Security Groups
2. Selecione o SG da instância
3. Inbound Rules > Edit
4. Add Rule: Custom TCP, Port 5678, Source 0.0.0.0/0


## 3. Preparar a instância

```bash
# Conectar à instância
ssh -i sua_chave.pem ubuntu@52.67.123.456

# Atualizar sistema
sudo apt update && sudo apt upgrade -y

# Instalar Docker
curl -fsSL https://get.docker.com -o get-docker.sh
sudo sh get-docker.sh
sudo usermod -aG docker ubuntu

# Instalar Docker Compose
sudo apt install docker-compose-plugin -y

# Logout e login novamente
exit
# ssh novamente...
```

## 4. Obter o IP público da instância

```bash
# Dentro da instância AWS
curl -s http://169.254.169.254/latest/meta-data/public-ipv4

## 5. Configurar N8N

```bash
# Criar pasta do projeto
mkdir n8n-aws && cd n8n-aws

# Criar arquivos (cole o conteúdo dos arquivos)
vim docker-compose.yml
vim .env

# IMPORTANTE: Atualizar .env com o IP correto
sed -i 's/52.67.123.456/SEU_IP_PUBLICO_REAL/g' .env

# OU editar manualmente:
vim .env  # Altere AWS_PUBLIC_IP=SEU_IP_REAL
```

## 6. Configurar permissões

```bash
# Criar pastas e definir permissões
mkdir -p n8n_data pg_data
sudo chown -R 1000:1000 n8n_data
sudo chown -R 999:999 pg_data
chmod -R 755 n8n_data pg_data
```

## 7. Gerar chave de criptografia

```bash
# Gerar nova chave
openssl rand -base64 32

# Atualizar no .env
vim .env  # Substitua N8N_ENCRYPTION_KEY
```

## 8. Deploy

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

## 9. Testar acesso

```bash
# Teste local na instância
curl http://localhost:5678

# Teste por IP público (do seu computador)
curl http://52.67.123.456:5678

# Abrir no navegador:
# http://52.67.123.456:5678
```

## 10. Acesso

```
URL de acesso: http://52.67.123.456:5678
Usuário: (crie no primeiro acesso)
```

## 11. Comandos úteis para gerenciamento

```bash
# Ver logs em tempo real
docker compose logs -f

# Reiniciar N8N
docker compose restart n8n

# Parar tudo
docker compose down

# Limpar logs do Docker (se ficar pesado)
docker system prune -f

# Ver uso de recursos
docker stats
```

## 12. Troubleshooting

```bash
# Se não conseguir acessar:
# 1. Verificar Security Group

# 2. Verificar se containers estão rodando
docker compose ps

# 3. Verificar se porta está aberta
sudo ss -tlnp | grep :5678

# 4. Testar conectividade local
curl http://localhost:5678

# 5. Verificar logs de erro
docker compose logs n8n | tail -50
```

## 13. Máquinas com pouca RAM
```
#!/bin/bash

sudo fallocate -l 16G /swapfile
sudo chmod 600 /swapfile
ls -hl /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile
sudo swapon --show
```