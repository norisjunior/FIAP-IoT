# Plataforma IoT

Mosquitto, Node-RED, n8n com PostgreSQL, InfluxDB e Grafana em um único Compose.
Os scripts apenas iniciam ou param serviços, sem gerar senhas ou reescrever arquivos.

## Implantação

Requisitos: Docker e Docker Compose, com permissão de acesso ao Docker. No Windows,
use Docker Desktop com contêineres Linux.

1. Copie este diretório completo para o servidor, incluindo arquivos ocultos.
2. Edite `.env`. Em um clone do Git, crie-o com `cp .env.exemplo .env`.
3. Execute dentro desta pasta:

```bash
bash start-linux.sh
```

Se o servidor exigir acesso administrativo, use `sudo bash start-linux.sh`.
No Windows, execute `./start-windows.ps1` no PowerShell.
Os scripts encontram a configuração pelo próprio diretório, mesmo chamados de outra pasta.
Também é possível iniciar diretamente com `docker compose --env-file .env up -d`.
Imagens ausentes são baixadas; as existentes não são atualizadas a cada início.

## Arquivos

- `.env`: configuração local, ignorada pelo Git; inclua na entrega à faculdade.
- `.env.exemplo`: valores didáticos versionados para copiar.
- `docker-compose.yml`: definição fixa dos serviços e volumes.
- `mqtt-broker/mosquitto.conf`: configuração pronta de MQTT e WebSocket.
- `nodered/settings.js`: autenticação que lê variáveis recebidas do Compose.
- `start-linux.sh` e `stop-linux.sh`: início e parada no Linux/WSL2.
- `start-windows.ps1` e `stop-windows.ps1`: início e parada no Windows.

n8n, PostgreSQL, InfluxDB e Grafana recebem configurações pelo Compose a partir do `.env`.
Não precisam de pastas de configuração vazias. Os dados ficam em volumes persistentes
administrados pelo Docker, evitando ajustes de permissões no servidor.

## Configuração

O `.env` concentra imagens, portas publicadas, fuso horário, usuários, senhas, token do
InfluxDB e chave do n8n. Revise os valores didáticos para a instalação da faculdade.

Para o n8n na rede, ajuste `N8N_HOST` para o IP ou domínio do servidor e
`N8N_EDITOR_BASE_URL` e `WEBHOOK_URL` para a URL completa com protocolo e porta pública.
Ao mudar `N8N_PORT`, atualize essas URLs; a porta interna continua sendo 5678.
Com HTTPS, ajuste o protocolo e `N8N_SECURE_COOKIE=true` conforme o proxy.

O Node-RED usa `NODERED_USER` e `NODERED_PASSWORD_HASH`. O hash fornecido corresponde
a `FIAPIoT`. Para trocar a senha, gere um hash uma vez, com a plataforma iniciada:

```bash
docker compose exec node-red node-red admin hash-pw
```

Cole o resultado em `NODERED_PASSWORD_HASH`, mantendo as aspas simples, e execute o start.
Referências: [autenticação do Node-RED](https://nodered.org/docs/user-guide/runtime/securing-node-red)
e [variáveis do Compose](https://docs.docker.com/compose/how-tos/environment-variables/variable-interpolation/).

## Acessos iniciais

Substitua localhost pelo endereço do servidor ao acessar de outra máquina.

| Serviço | Endereço padrão | Acesso |
| --- | --- | --- |
| MQTT | `localhost:1883` | Sem autenticação, como no laboratório original |
| MQTT WebSocket | `localhost:9001` | Sem autenticação |
| Node-RED | http://localhost:1880 | admin / FIAPIoT |
| n8n | http://localhost:5678 | Crie a conta no primeiro acesso |
| InfluxDB | http://localhost:8086 | admin / FIAP@123 |
| Grafana | http://localhost:3000 | admin / admin |

InfluxDB: organização `fiapiot`, bucket `sensores`, token no `.env`.
A porta adicional 3456 da imagem personalizada do Node-RED foi preservada.
Entre contêineres, use `mosquitto:1883` (ou `mqtt-broker:1883`), `influxdb:8086`,
`node-red:1880` e `n8n:5678`. No Wokwi com gateway local, use `host.wokwi.internal:1883`.
Libere no servidor as portas necessárias ao laboratório.

## Operação e persistência

Dentro desta pasta, consulte `docker compose ps` e `docker compose logs -f --tail 100`.
Pare com `bash stop-linux.sh` ou `./stop-windows.ps1`.
Parar e iniciar preserva fluxos, workflows, credenciais, bancos e dashboards.
Não acrescente a opção de remoção de volumes ao comando de parada.

Após editar `.env`, execute o start novamente. As contas iniciais de PostgreSQL,
InfluxDB e Grafana só são criadas em volumes vazios; editar a senha no arquivo não
altera uma conta já gravada no banco. Preserve `N8N_ENCRYPTION_KEY` após salvar credenciais.
Para atualizar imagens, faça backup e execute `docker compose pull`, seguido do start.

## Instalações anteriores

A pasta antiga `IoTStack/` não é removida nem migrada. Antes de iniciar no mesmo servidor,
pare o Compose antigo dentro dessa pasta, preservando os volumes, para evitar conflitos
de nomes e portas. Faça backup e migre/restaure os dados antes de substituir uma instalação em uso.

Os dados antigos do n8n e PostgreSQL ficavam em pastas locais; agora ficam em volumes Docker.
Volumes antigos de InfluxDB e Grafana podem ser reutilizados se o projeto anterior se
chamava `iotstack`, padrão de `COMPOSE_PROJECT_NAME`. Confira com `docker volume ls`.
Copiar esta pasta leva a configuração, não os dados dos volumes. Para transportar uma
instalação com dados, leve e restaure os backups também.
