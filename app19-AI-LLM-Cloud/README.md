# Perguntar ao motor em português

Até aqui, cada janela do motor era classificada e esquecida. A predição acendia
um LED e acabava ali: não havia como perguntar *"como o motor estava ontem à
noite?"*, porque ninguém guardou.

O dispositivo continua fazendo exatamente o que fazia: mede a janela, publica,
recebe a classe de volta e acende a saída. O que muda é que agora **alguém
anota** cada resposta, num **PostgreSQL**, e um modelo de linguagem lê essas
anotações.

```text
ESP32 ─► MQTT ─► n8n ─► API ─┬─► MQTT ─► ESP32 acende a saída
                             └─► PostgreSQL

você ─► chat do n8n ─► agente LLM ─► PostgreSQL ─► resposta em português
```

São **dois fluxos**: um que ingere sem parar, outro que responde quando você
pergunta. Eles não se falam — conversam pelo banco.

## Antes de começar

Este app roda na **mesma plataforma das aulas anteriores**, a `IoT-platform`.
Ela já sobe o PostgreSQL, que até aqui só guardava os dados do próprio n8n, e
a tabela deste app fica nele. O modelo de linguagem não roda no seu
computador: é o **Gemini**, do Google, chamado pela internet com a chave do
seu grupo.

Serviços que este app usa:

| Serviço | Onde | Para quê |
|---|---|---|
| MQTT Broker | `localhost:1883` | o ESP32 publica a janela |
| API | `localhost:8000`, no seu computador | a rede neural do app18 |
| n8n | `http://localhost:5678` | os fluxos de ingestão e de chat |
| PostgreSQL | `n8n-postgres:5432`, dentro do Docker | a tabela `motor_medicoes` |
| Gemini | na internet, com a chave do grupo | o modelo de linguagem do chat |

### A chave do Gemini

1. Entre em [aistudio.google.com](https://aistudio.google.com) com uma conta
   Google e clique em **Get API key** → **Create API key**. Não pede cartão.
2. No n8n, crie a credencial **Google Gemini(PaLM) Api**: cole a chave em
   **API Key** e deixe o **Host** como está.

A chave é uma senha: não vai para o GitHub nem para o grupo da turma.

O plano gratuito tem dois limites que aparecem na prática:

- **Pedidos por dia.** O agente gasta dois ou três pedidos a cada pergunta: um
  para escolher a ferramenta, outro para escrever a resposta. Acabou o limite,
  o chat devolve erro até o dia virar; use a chave de outro integrante do grupo.
- **O Google pode usar o que você manda** para melhorar os produtos dele. Dados
  do motor, tudo bem; dado pessoal, nunca.

Os fluxos já vêm com o modelo `gemini-3.5-flash-lite`. Se ele não aparecer na
lista do nó, escolha outro **Flash-Lite** ou **Flash** da lista.

## A API

É a do app18, na pasta [`api/`](../app18-Inferencia_AI_Sinais-MultiClass_IMU/api/),
servindo a mesma rede neural. Se ela ficou rodando desde a aula passada, não há
nada a fazer. Se não:

```bash
cd ../app18-Inferencia_AI_Sinais-MultiClass_IMU/api
python -m venv venv

venv\Scripts\activate        # Windows
source venv/bin/activate     # Linux / macOS

pip install -r requirements.txt
uvicorn service_app:app --host 0.0.0.0 --port 8000
```

Os fluxos apontam para `http://host.docker.internal:8000/predict` — o n8n roda em
contêiner e a API roda no seu computador, então `localhost` de dentro do n8n não
acharia ninguém.

Confira que subiu:

```bash
curl -X POST http://localhost:8000/predict \
  -H "Content-Type: application/json" \
  -d '{"mean_ax":0.049,"mean_ay":0.696,"mean_az":0.687,"std_ax":0.009,"std_ay":0.011,"std_az":0.011,"std_mag":0.011,"p2p_mag":0.088}'
```

Com o modelo de referência, a resposta é `inclinado_frente`. Se o seu grupo
treinou o próprio `.pkl`, ele vai nessa mesma pasta, por cima do de referência.

## 1) A placa

Não há firmware nesta pasta: a placa continua com o do app anterior
(`device-devkitv1/` ou `device-esp32c3/`, no app18), **sem regravar**. Ela
publica em `FIAPIoT/motor/multiclasse/<nome>` e escuta a classe em
`FIAPIoT/motor/multiclasse/<nome>/cmd`, onde `<nome>` é o `MQTT_CLIENT_ID` da
sua placa.

| GPIO | Componente | Classe |
|---|---|---|
| 4 | LED azul | `operando` |
| 21 | LED amarelo | `inclinado_frente` |
| 18 | LED vermelho | `inclinado_tras` |
| 19 | buzzer | `anomalia` |

## 2) O fluxo de ingestão

É o fluxo do app anterior com **dois nós a mais**. Há dois jeitos de chegar
nele:

- **No seu fluxo, à mão (o melhor):** abra o fluxo que você já tem rodando e
  acrescente os nós 7 e 8 da tabela, copiando o código e a consulta do
  `n8n/Fluxo-1-ingestao-postgres.json`.
- **Importando pronto:** importe o `n8n/Fluxo-1-ingestao-postgres.json`, troque
  `DEFINA_UM_NOME` nos dois nós de MQTT e `SEU_CHAT_ID` nos dois do Telegram, e
  **desative o fluxo do app anterior**: com os dois ativos, a placa recebe cada
  resposta duas vezes.

Nos dois casos, crie a credencial do **PostgreSQL**: Host `n8n-postgres`, e
Database, User e Password iguais aos do `.env` da plataforma (`POSTGRES_DB`,
`POSTGRES_USER`, `POSTGRES_PASSWORD`).

| # | Nó | O que faz |
|---|---|---|
| 1 | `Janela da placa` | recebe a janela, já como objeto (**JSON Parse Body**) |
| 2 | `Predict Motor` | `POST` do `message` para a API |
| 3 | `Classe para a placa` | devolve a classe ao ESP32 |
| 4 | `E anomalia?` | separa a classe `anomalia` |
| 5 | `ANOMALIA: alerta no Telegram` | avisa a pessoa |
| 6 | `FALHA: a API nao respondeu` | avisa quando a predição falha |
| 7 | `Formata para o banco` | calcula o carimbo de tempo e o **período** |
| 8 | `Armazena a predicao` | cria a tabela se não existir e insere |

Os nós **3, 4 e 7 saem os três do nó 2**, em paralelo: o ESP32 não espera a
gravação no banco para acender o LED, e o alerta não espera o banco.

**O alarme não passa pelo modelo de linguagem.** O buzzer e o Telegram
continuam neste fluxo, com uma regra fixa. O chat só consulta o que foi
guardado: se o limite da chave do Gemini acabar, o chat para e o alarme não.

### O período é o truque da aplicação

O nó 4 grava, além da classe, uma palavra: `madrugada`, `manhã`, `tarde` ou
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

Importe `n8n/Fluxo-2-chat-llm.json` e configure as credenciais do **Gemini**
(veja [A chave do Gemini](#a-chave-do-gemini)) e do **PostgreSQL**.

| Nó | Papel |
|---|---|
| `When chat message received` | abre a janela de chat |
| `FIoT Agent` | decide qual ferramenta usar e redige a resposta |
| `Google Gemini Chat Model` | o modelo de linguagem (`gemini-3.5-flash-lite`) |
| `Simple Memory` | lembra as mensagens anteriores da conversa |
| `motor_agora` | `SELECT ... ORDER BY created_at DESC LIMIT 1` |
| `motor_historico` | `SELECT classe, periodo, COUNT(*) ... WHERE data = $1 AND periodo = $2 GROUP BY classe, periodo` |

Ative o fluxo e abra a URL que o nó de chat mostra.

### Como o agente escolhe a ferramenta

Ele não escolhe por adivinhação: a escolha está escrita em dois lugares.

- Na **descrição de cada ferramenta**. A do `motor_historico` termina com um
  aviso em maiúsculas para não ser usada em perguntas sobre "agora" — sem isso,
  o agente às vezes usa a ferramenta errada.
- Na ***system message*** do agente, que recebe a data e a hora de hoje já
  resolvidas pelo n8n e traz dois exemplos prontos de conversão.

### O histórico chega contado

Um período tem milhares de janelas, uma por segundo. Mandar todas para o
modelo de linguagem e perguntar *"quantas falhas?"* seria pedir que um modelo de
linguagem contasse, e ele não conta. Por isso o `motor_historico` agrupa e
conta no banco, e o agente recebe no máximo quatro linhas:

| classe | periodo | ocorrencias |
|---|---|---|
| operando | madrugada | 3412 |
| anomalia | madrugada | 27 |

A *system message* completa a leitura com duas regras: **falha é a classe
`anomalia`**, e uma classe que não aparece no resultado teve **zero**
ocorrências. Sem a segunda, uma madrugada sem anomalia simplesmente não traz
a linha `anomalia`, e o modelo tende a inventar um número.

O modelo de linguagem não sabe que dia é hoje. Quem sabe é o n8n, e por isso a
data entra na *system message* em vez de o LLM ter de deduzir.

## 4) Testar

Com o ESP32 rodando e o fluxo de ingestão ativo, espere alguns segundos e
pergunte no chat:

- *"Como está o motor agora?"* → usa `motor_agora`
- *"Quantas falhas o motor teve nesta madrugada?"* → usa `motor_historico`
- *"Preciso fazer alguma coisa?"* → usa a memória da conversa e a orientação
  da *system message*

Na aula, as três respostas possíveis do histórico aparecem com o seu próprio
dado, trocando *noite* pelo período em que você está:

- *"Quantas falhas hoje à noite?"*, antes de chacoalhar → zero (a linha
  `anomalia` nem vem do banco);
- chacoalhe a placa e pergunte de novo → a contagem;
- *"E nesta madrugada?"* → não há medição.

Conferindo direto no banco, se quiser ver o que o agente viu (troque o usuário
e o banco se o seu `.env` for diferente):

```bash
docker exec -it n8n-postgres psql -U n8nuser -d n8n -c "SELECT classe, periodo, timestamp_medicao FROM motor_medicoes ORDER BY created_at DESC LIMIT 10;"
```

## 5) Quando a placa para

Desligue a placa, espere um minuto e pergunte *"como está o motor agora?"*.

O `motor_agora` devolve a **última linha do banco, seja de quando for**. O
agente recebe uma classe e um horário, e nada diz a ele que aquilo já é velho:
ele pode responder o estado de uma hora atrás como se fosse o de agora.

A correção segue a regra desta aplicação: o trabalho fica na consulta, não no
modelo de linguagem. Troque a consulta do `motor_agora` por esta, que já
entrega a conta feita:

```sql
SELECT classe, timestamp_medicao, periodo,
       EXTRACT(EPOCH FROM NOW() - created_at)::int AS segundos_atras
FROM motor_medicoes
ORDER BY created_at DESC
LIMIT 1;
```

E acrescente uma linha às LIMITAÇÕES da *system message*:

```text
- motor_agora devolve segundos_atras. Se for maior que 60, o motor está SEM
  MEDIÇÃO: diga desde que horas, e não diga que ele está naquele estado.
```

## 6) Exercício: uma ferramenta nova

Hoje o agente não sabe responder *"como o motor estava às 20h?"*: ele só
conhece "agora" e os quatro períodos. Crie a ferramenta `motor_horario`.

Dica: duplique o `motor_historico` e troque três coisas — o nome, a descrição
e o filtro da consulta. Está pronto quando o agente escolhe a ferramenta nova
sozinho, sem você citar o nome dela na pergunta.

<details>
<summary>Uma resposta possível</summary>

Descrição:

```text
Esta ferramenta conta quantas vezes cada classe ocorreu em uma HORA específica
de uma data. Use quando a pergunta citar um horário, como "às 20h" ou "por
volta das 8 da manhã". Sempre envie Query_Parameters como uma string no
formato: "AAAA-MM-DD, HH", com a hora de 0 a 23, por exemplo: "2026-09-21, 20".

**NÃO USE ESTA FERRAMENTA PARA RESPONDER COMO O MOTOR ESTÁ AGORA.**
```

Consulta (com o mesmo `Query Parameters` do `motor_historico`):

```sql
SELECT classe,
       COUNT(*) AS ocorrencias
FROM motor_medicoes
WHERE DATE(timestamp_medicao::timestamp) = $1
  AND EXTRACT(HOUR FROM timestamp_medicao::timestamp) = $2
GROUP BY classe
ORDER BY ocorrencias DESC;
```

</details>

## 7) As mesmas ferramentas, publicadas (MCP)

Até aqui as duas ferramentas moram **dentro** do fluxo do chat: só aquele
agente as enxerga. O MCP (*Model Context Protocol*) publica as ferramentas em
um endereço, e qualquer agente que fale MCP as lista e as chama. É o que a API
fez com o modelo: um contrato publicado.

1. Importe `n8n/Fluxo-3-mcp-publica.json`, configure a credencial do
   PostgreSQL nas duas ferramentas e **ative**. Ele publica o `motor_agora` e o
   `motor_historico` em `http://localhost:5678/mcp/motor`.
2. Importe `n8n/Fluxo-4-chat-mcp.json`, configure a credencial do Gemini e
   ative. É o mesmo agente, com a mesma *system message*, e **nenhuma consulta
   dentro dele**: no lugar das duas ferramentas há um nó **MCP Client**, que
   aponta para o endereço acima.
3. Faça as mesmas perguntas. As respostas são as mesmas.
4. No nó **MCP Client**, troque o **Endpoint** pelo endereço do n8n de outra
   pessoa (na aula, o do professor: `http://172.16.10.101:5678/mcp/motor`) e
   pergunte de novo: o seu agente passa a responder sobre **o motor dela**.
   Mudou o banco e mudou a máquina; o contrato é o mesmo.

O cliente recebe de cada ferramenta o nome, a descrição que você escreveu e o
que ela espera de entrada. A descrição continua sendo a única instrução para o
agente escolher.

O endereço está **sem senha**: quem alcança a porta 5678 do seu computador
pergunta ao seu banco. Serve para a aula; fora dela, o nó *MCP Server Trigger*
tem o campo **Authentication**.

## Estrutura

```text
n8n/   Fluxo-1 (ingestão) · Fluxo-2 (chat) · Fluxo-3 (as ferramentas por MCP) · Fluxo-4 (chat por MCP)
```

A placa e a API são as do app18. Depois daqui, o app20 coloca a decisão dentro
da placa e continua gravando nesta mesma tabela.

## Se der errado

**O chat responde que não há medições.** A tabela está vazia: confira se o
fluxo de ingestão está **ativo** (não basta importar) e se o Serial Monitor do
ESP32 mostra publicações.

**O agente sempre usa a ferramenta errada.** Confira se as descrições das duas
ferramentas vieram completas na importação — elas são a única instrução que o
agente tem para escolher.

**O chat devolve erro de cota (429) ou de chave.** Erro de cota é o limite
diário do plano gratuito do Gemini: espere o dia virar ou use a chave de outro
integrante do grupo. Erro de chave é a credencial do n8n: cole a chave de novo,
sem espaço no começo nem no fim.

**A API não responde.** Do n8n em contêiner, `localhost` é o próprio contêiner.
A URL precisa ser `http://host.docker.internal:8000/predict`.

**O horário sai errado.** O nó `Formata para o banco` fixa `America/Sao_Paulo`. Se o seu fuso for
outro, é lá que se muda — em um lugar só.
