# Roteiro: subir a plataforma e acessar o Node-RED

## 1. Entrar na pasta

Abra o terminal na pasta `IoT-platform`.

```
cd IoT-platform
```

## 2. Criar o `.env`

Duplique `.env.exemplo` e renomeie a cópia para `.env`.

## 3. Subir a plataforma

Inicie todos os serviços.

```
docker compose up -d
```

## 4. Conferir os contêineres

Todos devem aparecer como `running`.

```
docker compose ps
```

## 5. Ver a senha do Node-RED

A senha está no comentário do `.env`.

```
more .env
```

## 6. Acessar o Node-RED

Abra http://localhost:1880 e entre com `admin` / `FIAPIoT`.

## 7. Derrubar a plataforma

Pare todos os serviços; os dados continuam salvos.

```
docker compose down
```
