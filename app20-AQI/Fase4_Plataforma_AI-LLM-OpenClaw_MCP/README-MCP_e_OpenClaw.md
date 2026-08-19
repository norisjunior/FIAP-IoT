# Fase 4 - OpenClaw: Agente Autônomo para Monitoramento AQI

Nesta fase, substituímos o chatbot do n8n (Fase 3) por um **agente autônomo** OpenClaw conectado ao Telegram. O pipeline de dados (ESP32 → MQTT → n8n → ML → PostgreSQL) permanece intacto.

## Arquitetura

```
ESP32 (Wokwi) ──► MQTT Broker ──► n8n (fluxo ingestão) ──► ML Service :8000 ──► PostgreSQL
                                                                                    + Telegram (alertas)

Usuário ──► Telegram ──► OpenClaw ──► MCP Server :8088 ──► PostgreSQL / ML Service
                            │
                            ├── Memória persistente (USER.md, MEMORY.md)
                            └── Heartbeat (monitoramento proativo a cada 30min)
```

**O que muda em relação à Fase 3:**

| Componente | Fase 3 | Fase 4 |
|---|---|---|
| Chatbot | n8n LLM Agent + Ollama | **OpenClaw + Telegram** |
| Interface do aluno | URL do chat n8n | **Telegram direto** |
| Comportamento | Reativo (só responde) | **Proativo + Reativo** |
| Memória | Sem memória entre sessões | **Persistente** |

---

## Parte 1 — Instalação do OpenClaw no Ubuntu (WSL)

### Pré-requisitos

- WSL2 com Ubuntu instalado
- Node.js 22+
- Plataforma IoT rodando (MQTT, n8n, PostgreSQL, Ollama)
- Bot do Telegram criado via @BotFather (token em mãos)

### Passo 1 — Instalar dependências

```bash
wsl -d ubuntu

# Node.js 22 (pule se já tiver)
curl -fsSL https://deb.nodesource.com/setup_22.x | sudo -E bash -
sudo apt-get install -y nodejs

# jq (para as skills parsearem JSON)
sudo apt-get install -y jq

# Adicionar npm global ao PATH (necessário no WSL)
echo 'export PATH="$HOME/.npm-global/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

### Passo 2 — Instalar e configurar o OpenClaw

```bash
curl -fsSL https://openclaw.ai/install.sh | bash
```

Esse comando instala o CLI e já inicia o setup interativo. Siga nesta ordem:

1. **Modo de setup:** Selecione `QuickStart`
2. **LLM Provider:** Escolha conforme sua preferência:

**Opção A — Ollama (local, gratuito):**
```json
{
  "llm": {
    "provider": "ollama",
    "model": "llama3.2:latest",
    "baseUrl": "http://localhost:11434"
  }
}
```

**Opção B — Claude via API (mais inteligente, ideal para demo):**
```json
{
  "llm": {
    "provider": "anthropic",
    "apiKey": "SUA_CHAVE_API_ANTHROPIC",
    "model": "claude-sonnet-4-5-20250929"
  }
}
```

3. **Channel:** Selecione `Telegram (Bot API)` e cole o token do @BotFather
4. **Enable hooks:** Habilite `session-memory` (memória persistente entre sessões) e pule os demais
5. **Skills adicionais:** Pule por enquanto (vamos criar uma custom)

### Passo 3 — Executar o Doctor

Antes de iniciar o gateway, rode o diagnóstico:

```bash
openclaw doctor
```

Quando perguntar sobre OAuth dir, selecione **Create OAuth dir → Yes**. Isso garante que as credenciais sejam armazenadas corretamente.

### Passo 4 — Iniciar o Gateway

Em uma **janela de terminal separada**, rode:

```bash
# Necessário para o Ollama como provider (apenas na primeira vez)
echo 'export OLLAMA_API_KEY="ollama-local"' >> ~/.bashrc
source ~/.bashrc

openclaw gateway
```

O gateway roda em foreground e ficará mostrando os logs. Deixe esse terminal aberto.

> **Nota WSL2:** O comando `openclaw gateway start` (via systemd) pode não funcionar no WSL2. Use `openclaw gateway` direto.

> **Dashboard Web:** Ao acessar `http://localhost:18789` pela primeira vez, inclua o token na URL:
> ```
> http://localhost:18789/?token=SEU_TOKEN
> ```
> Para recuperar o token a qualquer momento:
> ```bash
> cat ~/.openclaw/openclaw.json | jq -r '.gateway.auth.token'
> ```
> Após o primeiro acesso, o browser salva o token no cookie e não será mais necessário.

### Passo 5 — Parear com o Telegram

Após o setup, o gateway inicia. No Telegram, envie qualquer mensagem para o seu bot. Ele responderá com um código de pareamento. No terminal:

```bash
openclaw pairing approve telegram <CODIGO>
```

### Passo 6 — Verificar funcionamento

Envie uma mensagem ao bot no Telegram. Se ele responder, o OpenClaw está funcionando.

---

## Parte 2 — Integração com o LLM IoT Stack

### Passo 1 — Subir o MCP Server

O MCP Server é a ponte entre o OpenClaw e sua infraestrutura (PostgreSQL + ML Service).

```bash
cd ~/FIAP-IoT/app20-AQI/Fase4_Plataforma_AI-LLM-OpenClaw_MCP/MCP_ML_AQI_Predict/

# Instalar dependências (apenas na primeira vez)
pip install -r requirements.txt --break-system-packages

# Subir o MCP Server
uvicorn aqi_mcp_server:app --host 0.0.0.0 --port 8088
```

Teste rápido:
```bash
curl http://localhost:8088/health
# Esperado: {"ok": true}
```

### Passo 2 — Subir o ML Service (se ainda não estiver rodando)

```bash
cd ~/FIAP-IoT/app20-AQI/App-Service-AI_Predict/
source venv/bin/activate
uvicorn service_app:app --reload --port 8000
```

### Passo 3 — Criar a Skill AQI Monitor

Crie o diretório da skill e o arquivo SKILL.md:

```bash
mkdir -p ~/.openclaw/skills/aqi-monitor
```

Crie o arquivo `~/.openclaw/skills/aqi-monitor/SKILL.md` com o conteúdo:

```markdown
---
name: aqi-monitor
description: Consulta qualidade do ar atual e histórica via sensores IoT ESP32. Use quando o usuário perguntar sobre qualidade do ar, AQI, poluição, ou segurança do ambiente.
metadata:
  openclaw:
    requires:
      bins: ["curl", "jq"]
---

# AQI Monitor - Qualidade do Ar IoT

## Contexto
Sensores ESP32 coletam dados de 12 poluentes (PM2.5, PM10, NO, NO2, NOx, NH3, CO, SO2, O3, Benzene, Toluene, Xylene) a cada 10 segundos. Um modelo de rede neural classifica a qualidade em: Aceitável, Ruim ou Perigoso. Os dados ficam no PostgreSQL.

## Qualidade do ar AGORA
Quando o usuário perguntar como está a qualidade do ar agora:

    curl -s http://localhost:8088/tools/aqi_atual | jq .

Interprete o campo "class" e informe o timestamp da medição.

## Qualidade do ar HISTÓRICA
Quando o usuário perguntar sobre um período específico (ex: "ontem à noite", "hoje de manhã"):
- madrugada: 00:00 às 05:59
- manhã: 06:00 às 11:59
- tarde: 12:00 às 17:59
- noite: 18:00 às 23:59

    curl -s -X POST http://localhost:8088/tools/aqi_historico \
      -H "Content-Type: application/json" \
      -d '{"date":"AAAA-MM-DD","periodo":"PERIODO"}'

Substitua AAAA-MM-DD pela data e PERIODO pelo período correto.

## Predição manual
Se o usuário fornecer valores de poluentes para classificar:

    curl -s -X POST http://localhost:8088/tools/aqi_predict \
      -H "Content-Type: application/json" \
      -d '{"PM2_5":VAL,"PM10":VAL,"NO":VAL,"NO2":VAL,"NOx":VAL,"NH3":VAL,"CO":VAL,"SO2":VAL,"O3":VAL,"Benzene":VAL,"Toluene":VAL,"Xylene":VAL}'

## Publicar alerta MQTT
Para enviar um alerta manual ao broker MQTT:

    curl -s -X POST http://localhost:8088/tools/mqtt_publish \
      -H "Content-Type: application/json" \
      -d '{"topic":"FIAPIoT/aqi/alerta","payload":"MENSAGEM"}'

## Regras
- NUNCA invente valores. Use apenas dados retornados pelas ferramentas.
- Se a classe for "Perigoso", alerte que NÃO se deve permanecer na fábrica sem EPI.
- Se "found" for false, informe que não há dados para o período solicitado.
- Contexto: as medições são de uma fábrica industrial.
```

### Passo 4 — Configurar o Heartbeat (monitoramento proativo)

Edite o arquivo `~/.openclaw/HEARTBEAT.md` e adicione:

```markdown
## Verificação de Qualidade do Ar
A cada 30 minutos, execute:

    curl -s http://localhost:8088/tools/aqi_atual | jq .

Se o campo "class" for "Perigoso", envie imediatamente uma mensagem de alerta ao usuário informando que a qualidade do ar está crítica e que ninguém deve permanecer na fábrica sem EPI.

Se for "Ruim", envie um aviso suave recomendando atenção.

Se for "Aceitável", não é necessário enviar mensagem.
```

### Passo 5 — Reiniciar o Gateway

Após criar a skill e o heartbeat, reinicie o OpenClaw:

```bash
openclaw gateway restart
```

---

## Parte 3 — Validação do Fluxo Completo

### Checklist de serviços rodando

| Serviço | Porta | Como verificar |
|---|---|---|
| MQTT Broker | 1883 | `docker ps \| grep mqtt` |
| n8n (fluxo ingestão) | 5678 | http://localhost:5678 |
| PostgreSQL | 5432 | `docker ps \| grep postgres` |
| Ollama | 11434 | `curl http://localhost:11434` |
| ML Service | 8000 | `curl -X POST http://localhost:8000/predict -H "Content-Type: application/json" -d '{"PM2_5":50,"PM10":80,"NO":5,"NO2":20,"NOx":15,"NH3":10,"CO":0.5,"SO2":5,"O3":30,"Benzene":1,"Toluene":5,"Xylene":0.1}'` |
| MCP Server | 8088 | `curl http://localhost:8088/health` |
| OpenClaw Gateway | — | `openclaw status` |

### Testando pelo Telegram

Envie estas mensagens ao bot:

1. **"Como está a qualidade do ar agora?"** → Deve consultar `aqi_atual` e responder com a classe
2. **"Como estava ontem à noite?"** → Deve consultar `aqi_historico` com data de ontem e período "noite"
3. **"Classifique: PM2.5=342, PM10=477, NO=10, NO2=51, NOx=40, NH3=42, CO=1.7, SO2=17, O3=91, Benzene=5.2, Toluene=29, Xylene=0.5"** → Deve retornar "Perigoso"

### Testando o Heartbeat

Aguarde 30 minutos (ou ajuste o intervalo) e verifique se o bot envia mensagens proativas no Telegram sobre a qualidade do ar.

---

## Troubleshooting

**OpenClaw não responde no Telegram:**
- Verifique se o gateway está rodando: `openclaw status`
- Verifique o token do bot: `cat ~/.openclaw/openclaw.json | jq .channels.telegram`
- Verifique logs: `openclaw gateway logs`

**Skill não é acionada:**
- Confirme que o arquivo está em `~/.openclaw/skills/aqi-monitor/SKILL.md`
- Reinicie o gateway: `openclaw gateway restart`
- Verifique skills carregadas: `openclaw skills list`

**MCP Server retorna erro 500:**
- Verifique se PostgreSQL está acessível: `psql -h localhost -U n8nuser -d n8n -c "SELECT 1;"`
- Verifique se a tabela existe: `psql -h localhost -U n8nuser -d n8n -c "SELECT * FROM aqi_medicoes LIMIT 1;"`
- Verifique se o ML Service está rodando: `curl http://localhost:8000/docs`

**Heartbeat não funciona:**
- Confirme que `~/.openclaw/HEARTBEAT.md` existe e está formatado corretamente
- Verifique o intervalo configurado: `cat ~/.openclaw/openclaw.json | jq .heartbeat`

---

## Comandos Úteis

```bash
# Status do OpenClaw
openclaw status

# Logs em tempo real
openclaw gateway logs

# Reiniciar gateway
openclaw gateway restart

# Listar skills instaladas
openclaw skills list

# Parar o gateway
openclaw gateway stop
```