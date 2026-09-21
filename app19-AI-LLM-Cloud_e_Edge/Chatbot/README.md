# app19 / Chatbot — perguntar ao motor em português

O dispositivo continua fazendo exatamente o que fazia: mede a janela, publica,
recebe a classe de volta e acende a saída. O que muda é que agora **alguém
anota** cada resposta, e um modelo de linguagem lê essas anotações.

```text
ESP32 ─► MQTT ─► n8n ─► API ─┬─► MQTT ─► ESP32 acende a saída
                             └─► PostgreSQL

você ─► chat do n8n ─► agente LLM ─► PostgreSQL ─► resposta em português
```

São **dois fluxos**: um que ingere sem parar, outro que responde quando você
pergunta. Eles não se falam — conversam pelo banco.

## 1) O firmware

```bash
cd device
pio run
```

Ajuste no `.cpp` o Wi-Fi e o `MQTT_SERVER` antes de gravar.

É o firmware do app anterior, sem uma linha de diferença. Ele publica em
`FIAPIoT/motor/multiclasse` e escuta a classe em
`FIAPIoT/motor/multiclasse/cmd`.

| GPIO | Componente | Classe |
|---|---|---|
| 4 | LED azul | `operando` |
| 21 | LED amarelo | `inclinado_frente` |
| 18 | LED vermelho | `inclinado_tras` |
| 19 | buzzer | `anomalia` |

## 2) O fluxo de ingestão

Importe `n8n/Fluxo-1-ingestao-postgres.json` e configure as credenciais de
**MQTT** e **PostgreSQL**.

| # | Nó | O que faz |
|---|---|---|
| 1 | `FIAPIoT/motor/multiclasse` | recebe a janela |
| 2 | `Code (gera JSON)` | converte a mensagem MQTT em JSON |
| 3 | `Predict Motor` | `POST` para a API |
| 4 | `FIAPIoT/motor/multiclasse/cmd` | devolve a classe ao ESP32 |
| 5 | `Formata para o banco` | calcula o carimbo de tempo e o **período** |
| 6 | `Armazena a predicao` | cria a tabela se não existir e insere |

Os nós **4 e 5 saem os dois do nó 3**, em paralelo: o ESP32 não espera a
gravação no banco para acender o LED.

Comparado ao fluxo do app anterior, são **dois nós a mais** — o 5 e o 6. Todo o
resto é idêntico.

### O período é o truque da aplicação

O nó 5 grava, além da classe, uma palavra: `madrugada`, `manhã`, `tarde` ou
`noite`. Parece detalhe, e é o que faz o chat funcionar.

Sem ela, *"como estava ontem à noite?"* obrigaria o modelo de linguagem a
converter "ontem à noite" em um intervalo de horas e montar um `BETWEEN`. Com
ela, a consulta vira um `WHERE periodo = 'noite'` — e o LLM só precisa escolher
uma palavra de uma lista de quatro.

**Trabalho feito na ingestão é trabalho que o LLM não precisa acertar.**

### A tabela

```sql
CREATE TABLE motor_medicoes (
  id                SERIAL PRIMARY KEY,
  classe            TEXT,
  timestamp_medicao TEXT,
  periodo           TEXT,
  created_at        TIMESTAMPTZ DEFAULT NOW()
);
```

Repare no que **não** está aqui: as 8 features. Elas não entram porque o chat
responde sobre o *estado* do motor, e cada coluna a mais é uma coluna em que o
LLM pode se perder. O `CREATE TABLE IF NOT EXISTS` está dentro do próprio nó,
então não há passo de preparação do banco — a primeira janela cria a tabela.

## 3) O fluxo do chat

Importe `n8n/Fluxo-2-chat-llm.json` e configure as credenciais de **Ollama** e
**PostgreSQL**.

| Nó | Papel |
|---|---|
| `When chat message received` | abre a janela de chat |
| `FIoT Agent` | decide qual ferramenta usar e redige a resposta |
| `Ollama Chat Model` | o modelo de linguagem (`llama3.2:1b`) |
| `Simple Memory` | lembra as mensagens anteriores da conversa |
| `motor_agora` | `SELECT ... ORDER BY created_at DESC LIMIT 1` |
| `motor_historico` | `SELECT ... WHERE data = $1 AND periodo = $2` |

Ative o fluxo e abra a URL que o nó de chat mostra.

### Como o agente escolhe a ferramenta

Ele não escolhe por adivinhação: a escolha está escrita em dois lugares.

- Na **descrição de cada ferramenta**. A do `motor_historico` termina com um
  aviso em maiúsculas para não ser usada em perguntas sobre "agora" — sem isso,
  modelos pequenos usam a ferramenta errada com frequência.
- Na ***system message*** do agente, que recebe a data e a hora de hoje já
  resolvidas pelo n8n e traz dois exemplos prontos de conversão.

O modelo de linguagem não sabe que dia é hoje. Quem sabe é o n8n, e por isso a
data entra na *system message* em vez de o LLM ter de deduzir.

## 4) Testar

Com o ESP32 rodando e o fluxo de ingestão ativo, espere alguns segundos e
pergunte no chat:

- *"Como está o motor agora?"* → usa `motor_agora`
- *"E como estava hoje de manhã?"* → usa `motor_historico`
- *"Preciso fazer alguma coisa?"* → usa a memória da conversa e a orientação
  da *system message*

Conferindo direto no banco, se quiser ver o que o agente viu:

```sql
SELECT classe, periodo, timestamp_medicao
FROM motor_medicoes
ORDER BY created_at DESC
LIMIT 10;
```

## Se der errado

**O chat responde que não há medições.** A tabela está vazia: confira se o
fluxo de ingestão está **ativo** (não basta importar) e se o Serial Monitor do
ESP32 mostra publicações.

**O agente sempre usa a ferramenta errada.** É o sintoma clássico de modelo
pequeno. Confira se as descrições das duas ferramentas vieram completas na
importação — elas são a única instrução que o agente tem para escolher.

**A API não responde.** Do n8n em contêiner, `localhost` é o próprio contêiner.
A URL precisa ser `http://host.docker.internal:8000/predict`.

**O horário sai errado.** O nó 5 fixa `America/Sao_Paulo`. Se o seu fuso for
outro, é lá que se muda — em um lugar só.
