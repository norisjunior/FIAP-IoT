# Plataforma IoT Segura com Machine Learning e LLM

Aplicação IoT + ML + LLM local + Banco + Notificações + MQTT seguro (autenticação e ACL).

## Fluxo do Sistema

```
ESP32 → MQTT → n8n → Flask ML API → PostgreSQL → n8n → Ollama
```

## O que faz

1. ESP32 lê 12 sensores de qualidade do ar e publica via MQTT
2. n8n recebe dados, envia para API de ML (Flask/TensorFlow)
3. Modelo classifica como: Aceitável, Ruim ou Perigoso
4. Dados são gravados no PostgreSQL com timestamp e período do dia
5. Se perigoso, envia alerta no Telegram
6. Chatbot com Ollama responde perguntas sobre qualidade do ar atual e histórica

## Pré-requisitos

- WSL2 Ubuntu
- Docker e Docker Compose
- GPU NVIDIA (opcional, mas recomendado se disponível)
- 16GB RAM mínimo


## Como usar

### 0. Acessar a máquina ubuntu no WSL2 (exemplo com máquina ubuntu já instalada):
```
wsl -d ubuntu
```

- Caso precise/queira instalar a máquina ubuntu no WSL2:
```
wsl --install ubuntu
```


### 1. Clonar o git da disciplina
```bash
cd ~

git clone https://github.com/norisjunior/FIAP-IoT
```

### 2. Acessar o diretório que contém o script automático para inicialização dos serviços
```bash
cd FIAP-IoT/secure-LLM-IoT-platform/
```

### 3. Iniciar todos os serviços como uma plataforma integrada
```bash
sudo ./start-secure-llm-iot-platform.sh
```

### 4. Parar (sem excluir) todos os serviços
```bash
sudo ./stop-secure-llm-iot-platform.sh
```


O script vai:
- Criar o diretório `LLMIoTStack` com subdiretórios para cada serviço
- Criar todos os arquivos de configuração (docker-compose.yml, settings.js, .env, etc)
- Configurar o broker MQTT com autenticação (`allow_anonymous false`) e ACL
- Fazer build e subir todos os containers em uma única rede Docker

A plataforma IoT cria e inicia: Ollama, MQTT (com autenticação e ACL), Node-RED, n8n, PostgreSQL, InfluxDB, Grafana.

### 4. Acessos

| Serviço | URL | Credenciais |
|---------|-----|-------------|
| n8n | http://localhost:5678 | Criar no primeiro acesso |
| Ollama | http://localhost:11434 | - |
| PostgreSQL | localhost:5432 | n8nuser / minha_senha_super_secreta |

### 3. Configurar Credenciais no n8n

**MQTT**:
- Host: `mqtt-broker`, Port: `1883`
- Requer autenticação (usuário e senha configurados no broker - ver seção abaixo)

### Configurar Segurança MQTT (Autenticação e ACL)

O broker MQTT está configurado com `allow_anonymous false`. É necessário criar usuários e definir permissões (ACL) antes de publicar/assinar tópicos.

#### Passo 1 - Criar usuários e senhas no broker

Execute:
```bash
sudo docker exec -it mqtt-broker \
  mosquitto_passwd /mosquitto/config/passwd device
```

#### Passo 2 - Configurar ACL (quem publica/assina onde)

Edite o arquivo `LLMIoTStack/mqtt-broker/acl` e adicione as regras:
```
user device1
topic write FIAPIoT/smartagro/#

user device2
topic read FIAPIoT/smartagro/cmd/#

user admin
topic readwrite #
```

#### Passo 3 - Reiniciar o broker para aplicar as mudanças
```bash
cd LLMIoTStack && sudo docker compose restart mosquitto
```

**PostgreSQL**:
- Host: `n8n-postgres`, Port: `5432`
- Database: `n8n`, User: `n8nuser`
- Password: `minha_senha_super_secreta`

**Telegram** (opcional):
- Token do BotFather
- Seu Chat ID

## Configurar ESP32

Siga: [README do App20](https://github.com/norisjunior/FIAP-IoT/blob/main/app20-AQI/README-AQIManual.md)

## Iniciar Serviço de ML

Considera-se que o README-Manual do app20-AQI foi seguido e executado integralmente

```bash
cd ~/FIAP-IoT/app20-AQI
python3 -m venv venv
source venv/bin/activate
python3 app_ML.py
```

Testar:
```bash
curl -X POST http://localhost:8000/predict -H "Content-Type: application/json" -d '{
  "PM2_5": 342, "PM10": 477, "NO": 10, "NO2": 51, "NOx": 40,
  "NH3": 42, "CO": 1.7, "SO2": 17, "O3": 91,
  "Benzene": 5.2, "Toluene": 29, "Xylene": 0.5
}'
```

## Configurar Workflows n8n

### 1. Workflow de Ingestão

Importar `Fluxo-n8n-manual.json` no n8n.

Criar fluxo de ingestão a partir dele.

#### Banco de dados

Execute query para criar a tabela `aqi_medicoes`:
```sql
CREATE TABLE IF NOT EXISTS aqi_medicoes (
  id SERIAL PRIMARY KEY,
  class TEXT,
  timestamp_medicao TEXT,
  periodo TEXT,
  created_at TIMESTAMPTZ DEFAULT NOW()
);

INSERT INTO aqi_medicoes (class, timestamp_medicao, periodo)
VALUES ('{{$json.value.class}}', '{{$json.value.timestamp}}', '{{$json.value.periodo}}');
```

### 2. Workflow Chatbot

Criar novo fluxo para o agente conversacional.

**Tools**:
- `aqi_atual`: Consulta última medição
- `aqi_historico`: Consulta por data e período (madrugada/manhã/tarde/noite)

**Exemplos de perguntas**:
- "Como está o ar agora?"
- "Como estava nesta manhã?"
- "Ontem à noite estava bom?"



#### System prompt:
```
Você é um assistente de qualidade do ar. 

CONTEXTO TEMPORAL IMPORTANTE:
- Data atual: {{ $now.format('yyyy-LL-dd') }}
- Hora atual: {{ $now.format('HH:mm') }}
- Dia da semana: {{ $now.format('EEEE') }}

PERÍODOS DO DIA:
- madrugada: 00:00 às 05:59
- manhã: 06:00 às 11:59
- tarde: 12:00 às 17:59
- noite: 18:00 às 23:59

Quando o usuário perguntar sobre qualidade do ar em um período específico (ex: "nesta madrugada", "hoje de manhã"), use a ferramenta aqi_historico com os parâmetros no formato: "AAAA-MM-DD, periodo"

Exemplos:
- "Como estava nesta madrugada?" → "{{ $now.format('yyyy-LL-dd') }}, madrugada"
- "Como estava ontem à noite?" → "{{ $now.minus({days: 1}).format('yyyy-LL-dd') }}, noite"

Para perguntas sobre "agora" ou "atualmente", use a ferramenta aqi_atual.

LIMITAÇÕES
- Não invente valores de AQI. Sempre baseie sua resposta somente nos dados retornados pelas ferramentas.

CONTEXTO DO LOCAL
- A medição de qualidade do ar é em uma fábrica. Portanto, em medições perigosas, informe que não se deve permanecer dentro da fábrica.
```

#### Tool `aqi_atual`:

##### Description:
  ```
  Essa ferramenta se chama: "aqi_atual" e responde como está a qualidade do ar agora, nesse instante. Tool ou ferramenta para informar a qualidade do ar mais recentemente observada.
  ```

##### Query:
  ```sql
  SELECT class, timestamp_medicao, periodo
  FROM aqi_medicoes
  ORDER BY created_at DESC
  LIMIT 1;
  ```


#### Tool `aqi_historico`:

##### Description:
  ```
  Esta ferramenta consulta dados históricos de qualidade do ar. Sempre que você usá-la, envie Query_Parameters como uma string no formato: "AAAA-MM-DD, periodo", por exemplo: "2025-11-19, madrugada".
  O campo periodo deve ser uma destas palavras exatas: madrugada, manhã, tarde, noite.

  **NÃO USE ESSA FERRAMENTA OU TOOL PARA RESPONDER À PERGUNTAS DE COMO A QUALIDADE DO AR ESTÁ AGORA, OU QUAL A QUALIDADE DO AR MAIS RECENTE**"
  ```

##### Query:
  ```sql
  SELECT class,
        timestamp_medicao,
        periodo
  FROM aqi_medicoes
  WHERE DATE(timestamp_medicao::timestamp) = $1
    AND periodo = $2
  ORDER BY created_at DESC
  LIMIT 1;
  ```

