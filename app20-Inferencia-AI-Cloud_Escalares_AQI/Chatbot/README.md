# Chatbot — perguntar ao ar em português

A classificação do `CloudAI` acendia um alerta no Telegram e acabava ali. Aqui
ela passa a ser **guardada** no PostgreSQL, e um modelo de linguagem lê essas
anotações para responder perguntas como *"como estava o ar ontem à noite?"*.

```text
ESP32 ─► MQTT ─► n8n ─► API ─┬─► Telegram
                             └─► PostgreSQL

você ─► chat do n8n ─► agente LLM ─► PostgreSQL ─► resposta em português
```

São **dois fluxos**: um que ingere sem parar, outro que responde quando você
pergunta. Eles não se falam — conversam pelo banco.

## 1) A plataforma muda

O `CloudAI` roda na plataforma comum. Este precisa da **plataforma LLM + IoT**,
que acrescenta PostgreSQL e Ollama. Pare a anterior antes:

```bash
wsl -d ubuntu
cd ~/FIAP-IoT/IoT-platform/ && docker compose down

cd ~/FIAP-IoT/LLM-IoT-platform/
sudo ./start-llm-iot-platform.sh
docker ps
```

| Serviço | Onde |
|---|---|
| MQTT Broker | `localhost:1883` |
| n8n | `http://localhost:5678` |
| PostgreSQL | `localhost:5432` |
| Ollama | `http://localhost:11434` |

A API continua sendo a mesma, rodando no seu computador a partir de `api/`.

## 2) O fluxo de ingestão

Importe `n8n/Fluxo-1-ingestao-postgres.json` e configure **MQTT**, **Telegram**
e **PostgreSQL**.

Credenciais do banco, como estão no compose da plataforma:

| Campo | Valor |
|---|---|
| Host | `n8n-postgres` |
| Database | `n8n` |
| User | `n8nuser` |
| Password | `minha_senha_super_secreta` |

Comparado ao fluxo do `CloudAI`, são **dois nós a mais**: um `Code` que monta o
carimbo de tempo e o **período**, e um `Postgres` que cria a tabela se não
existir e insere. Eles saem em paralelo com o ramo do Telegram — o alerta não
espera a gravação.

### O período é o truque da aplicação

O nó `Code` grava, além da classe, uma palavra: `madrugada`, `manhã`, `tarde` ou
`noite`. Parece detalhe, e é o que faz o chat funcionar.

Sem ela, *"como estava ontem à noite?"* obrigaria o modelo de linguagem a
converter a frase em um intervalo de horas e montar um `BETWEEN`. Com ela, a
consulta vira um `WHERE periodo = 'noite'` — e o LLM só precisa escolher uma
palavra de uma lista de quatro.

**Trabalho feito na ingestão é trabalho que o LLM não precisa acertar.**

### A tabela

```sql
CREATE TABLE aqi_medicoes (
  id                SERIAL PRIMARY KEY,
  class             TEXT,
  timestamp_medicao TEXT,
  periodo           TEXT,
  created_at        TIMESTAMPTZ DEFAULT NOW()
);
```

Os doze poluentes não entram: o chat responde sobre o **estado** do ar, e cada
coluna a mais é uma coluna em que o LLM pode se perder. O `CREATE TABLE IF NOT
EXISTS` está dentro do próprio nó, então não há passo de preparação do banco —
a primeira medição cria a tabela.

## 3) O fluxo do chat

Importe `n8n/Fluxo-2-chat-llm.json` e configure **Ollama** e **PostgreSQL**.

| Nó | Papel |
|---|---|
| `When chat message received` | abre a janela de chat |
| `FIoT Agent` | escolhe a ferramenta e redige a resposta |
| `Ollama Chat Model` | o modelo de linguagem (`llama3.2:1b`) |
| `Simple Memory` | lembra as mensagens anteriores |
| `aqi_atual` | `SELECT ... ORDER BY created_at DESC LIMIT 1` |
| `aqi_historico` | `SELECT ... WHERE data = $1 AND periodo = $2` |

Ative o fluxo e abra a URL que o nó de chat mostra.

### Como o agente escolhe a ferramenta

Não é adivinhação: a escolha está escrita em dois lugares.

- Na **descrição de cada ferramenta**. A do `aqi_historico` termina com um aviso
  em maiúsculas para não ser usada em perguntas sobre "agora" — sem isso,
  modelos pequenos erram a escolha com frequência.
- Na ***system message*** do agente, que recebe a data e a hora de hoje já
  resolvidas pelo n8n e traz dois exemplos prontos de conversão.

O modelo de linguagem não sabe que dia é hoje. Quem sabe é o n8n, e por isso a
data entra na *system message* em vez de o LLM ter de deduzir.

## 4) Testar

Com o ESP32 rodando e o fluxo de ingestão **ativo**, espere alguns segundos e
pergunte no chat:

- *"Como está a qualidade do ar agora?"* → usa `aqi_atual`
- *"E como estava hoje de manhã?"* → usa `aqi_historico`
- *"Posso entrar na fábrica?"* → usa a orientação da *system message*

Conferindo direto no banco, se quiser ver o que o agente viu:

```sql
SELECT class, periodo, timestamp_medicao
FROM aqi_medicoes
ORDER BY created_at DESC
LIMIT 10;
```

## Se der errado

**O chat responde que não há medições.** A tabela está vazia: confira se o fluxo
de ingestão está **ativo** (não basta importar) e se o Serial Monitor mostra
publicações.

**O agente sempre usa a ferramenta errada.** É o sintoma clássico de modelo
pequeno. Confira se as descrições das duas ferramentas vieram completas na
importação — elas são a única instrução que o agente tem para escolher.

**O alerta de `Perigoso` nunca dispara.** Confira as classes do modelo que está
em `api/`: têm de ser exatamente `Aceitável`, `Ruim` e `Perigoso`. Um modelo
retreinado com outros nomes mata esse ramo em silêncio.

**O horário sai errado.** O nó `Code` fixa `America/Sao_Paulo`. Se o seu fuso
for outro, é lá que se muda — em um lugar só.
