# Construir o fluxo 1 — a predição da borda no banco

Três nós: receber a mensagem do ESP32, montar a linha da tabela e gravar.

```text
MQTT Trigger → Code: Formata para o banco → Postgres: Armazena a predicao
```

A cada etapa o fluxo roda e você confere a saída do nó antes de ir para o
próximo.

É o fluxo de ingestão da `Chatbot-CloudAI/` com dois nós a menos. Lá a classe
vinha da API; aqui ela **já chega decidida** pelo ESP32. Por isso não há
`Predict Motor` e não há tópico de comando: não há o que perguntar à nuvem nem
o que devolver ao dispositivo.

| Nó da `Chatbot-CloudAI/` | Aqui |
|---|---|
| `FIAPIoT/motor/multiclasse` | **muda o tópico**: `FIAPIoT/motor/validacao` |
| `Predict Motor` | sai |
| `FIAPIoT/motor/multiclasse/cmd` | sai |
| `Formata para o banco` | **⇄ igual**, menos uma linha: de onde vem a classe |
| `Armazena a predicao` | **⇄ igual**, mesma tabela, mesmo SQL |

O **⇄** marca o que precisa ficar idêntico ao da `Chatbot-CloudAI/`. É essa
igualdade que deixa o chat de lá funcionar aqui sem mudar nada.

## 0 — Antes de começar

- Inicie a IoT-platform e abra `http://localhost:5678`.
- Deixe o ESP32 publicando (ou use o `mosquitto_pub` da etapa 4).
- Crie um workflow vazio e dê a ele o nome
  `app19 - Ingestao da predicao da borda no PostgreSQL`.

Para usar a versão pronta, importe
[Fluxo-1-ingestao-borda.json](Fluxo-1-ingestao-borda.json) e selecione suas
credenciais de MQTT e PostgreSQL.

## 1 — MQTT Trigger: receber a mensagem

Adicione **MQTT Trigger** e configure:

| Campo | Valor |
|---|---|
| Nome do nó | `FIAPIoT/motor/validacao` |
| Credential | Sua credencial MQTT da IoT-platform |
| Topics | `FIAPIoT/motor/validacao` |
| Options | **JSON Parse Body** |

Se precisar criar a credencial MQTT, use Host `mqtt-broker`, Port `1883` e SSL
desligado.

> **Não ligue `Only Message`.** Ela embrulha o conteúdo num array, e o
> `$json.message` da etapa 2 deixa de existir.

**O tópico é outro de propósito.** Se fosse `FIAPIoT/motor/multiclasse`, o
fluxo da `Chatbot-CloudAI/` também receberia estas mensagens e responderia no
tópico de comando dele.

**Confira.** Clique em **Execute step** e espere o ESP32 publicar. Na saída,
`message` deve abrir em campos, não aparecer como um texto entre aspas:

```json
{
  "topic": "FIAPIoT/motor/validacao",
  "message": {
    "device": "IoTDevValidacaoMotor001",
    "predicao_borda": "operando",
    "mean_ax": -0.124, "mean_ay": -0.014, "mean_az": 0.879,
    "std_ax": 0.248, "std_ay": 0.133, "std_az": 0.225,
    "std_mag": 0.203, "p2p_mag": 0.781
  }
}
```

Este fluxo só vai usar a `predicao_borda`. As oito features ficam para o fluxo
2, que confere a borda contra a nuvem. É a mesma mensagem servindo aos dois.

## 2 — Code: montar a linha da tabela

Conecte um nó **Code** ao MQTT Trigger:

| Campo | Valor |
|---|---|
| Nome do nó | `Formata para o banco` |
| Mode | `Run Once for All Items` |
| Language | `JavaScript` |

Apague o código de exemplo e cole:

```js
// Timestamp e periodo no fuso de Sao Paulo. O PERIODO e o truque desta
// aplicacao: gravado agora, ele faz "ontem a noite" virar um WHERE comum
// na hora da consulta, sem ninguem precisar fazer conta de data.
const agora = new Date().toLocaleString("sv-SE", {
  timeZone: "America/Sao_Paulo",
  hour12: false
});

const hora = Number(new Date().toLocaleString("en-US", {
  timeZone: "America/Sao_Paulo",
  hour: "numeric",
  hour12: false
}));

let periodo = "madrugada";
if (hora >= 6 && hora < 12) periodo = "manhã";
else if (hora >= 12 && hora < 18) periodo = "tarde";
else if (hora >= 18) periodo = "noite";

// A classe ja vem pronta: quem decidiu foi o ESP32. Nao ha API no caminho.
const classe = $input.first().json.message.predicao_borda;

return [{ json: { classe, timestamp_medicao: agora, periodo } }];
```

**⇄ Igual ao da `Chatbot-CloudAI/`, menos a linha da `classe`.** Lá era
`$input.first().json.class`, porque o item que chegava era a resposta da API.
Aqui o item é a mensagem do ESP32, e a classe está em `message.predicao_borda`.

Três detalhes que precisam ficar como estão:

- **`sv-SE`** não é capricho: é o locale que escreve a data como
  `2026-09-30 14:00:01`, o formato que o `DATE(timestamp_medicao::timestamp)`
  do chat entende.
- **`manhã` leva til.** O chat filtra `periodo = 'manhã'`; sem o til, as manhãs
  somem das consultas.
- **O fuso é `America/Sao_Paulo`.** O contêiner do n8n roda em UTC; sem o fuso,
  as 22h viram 1h do dia seguinte, na madrugada.

**Confira.** Clique em **Execute step**. A saída é um item com três campos:

```json
{ "classe": "operando", "timestamp_medicao": "2026-09-30 14:00:01", "periodo": "tarde" }
```

Compare a hora com o relógio do computador. Se estiver 3 h adiantada, o fuso
não foi aplicado.

## 3 — Postgres: gravar

Conecte um nó **Postgres** ao `Formata para o banco`:

| Campo | Valor |
|---|---|
| Nome do nó | `Armazena a predicao` |
| Credential | Sua credencial PostgreSQL (tabela abaixo) |
| Operation | `Execute a SQL query` |
| Query | o SQL abaixo |

Credencial do banco, como está no compose da IoT-platform:

| Campo | Valor |
|---|---|
| Host | `n8n-postgres` |
| Database | `n8n` |
| User | `n8nuser` |
| Password | `minha_senha_super_secreta` |
| Port | `5432` |

```sql
CREATE TABLE IF NOT EXISTS motor_medicoes (
  id SERIAL PRIMARY KEY,
  classe TEXT,
  timestamp_medicao TEXT,
  periodo TEXT,
  created_at TIMESTAMPTZ DEFAULT NOW()
);

INSERT INTO motor_medicoes (classe, timestamp_medicao, periodo)
VALUES ('{{ $json.classe }}', '{{ $json.timestamp_medicao }}', '{{ $json.periodo }}');
```

**⇄ Igual ao da `Chatbot-CloudAI/`: mesma tabela, mesmas colunas.** O chat de
lá (`Chatbot-CloudAI/n8n/Fluxo-2-chat-llm.json`) lê `motor_medicoes` e não
sabe, nem precisa saber, quem classificou.

O `CREATE TABLE IF NOT EXISTS` roda a cada mensagem e só faz algo na primeira:
a tabela nasce sozinha, sem passo manual no banco.

**As aspas simples em volta de cada `{{ }}` são obrigatórias.** O n8n troca a
expressão pelo texto, e o SQL precisa ver `'operando'`, não `operando`.

**Confira.** Clique em **Execute step**. Depois, no terminal:

```bash
docker exec n8n-postgres psql -U n8nuser -d n8n \
  -c "SELECT * FROM motor_medicoes ORDER BY id DESC LIMIT 5;"
```

A linha que você acabou de gravar aparece no topo.

## 4 — Testar sem o ESP32

No Wokwi não dá para gerar uma `anomalia`, e na bancada desbalancear o motor
no meio da aula não é prático. Como este fluxo só lê a `predicao_borda`, dá
para publicar a mensagem fingindo ser o ESP32:

```bash
mosquitto_pub -h localhost -t "FIAPIoT/motor/validacao" -m '{"device":"TesteManual","predicao_borda":"anomalia","mean_ax":-0.02,"mean_ay":0.00,"mean_az":1.00,"std_ax":0.450,"std_ay":0.300,"std_az":0.400,"std_mag":0.420,"p2p_mag":1.60}'
```

O fluxo não sabe a diferença: grava `anomalia` como se o dispositivo tivesse
decidido.

Repare que o fluxo **não confere** se a classe faz sentido para as features:
ele confia na borda. Publique `"predicao_borda":"operando"` com as mesmas
features de vibração alta e ele grava `operando` do mesmo jeito. Conferir é o
trabalho do fluxo 2.

> No PowerShell, troque as aspas simples por duplas e escape as internas, ou
> rode a linha no Git Bash / WSL, onde ela funciona como está.

## 5 — Ativar

1. Salve e ative/publique o workflow para gravar continuamente.
2. Mantenha **uma** cópia ativa. Duas cópias gravam cada predição duas vezes,
   e o chat passa a contar o dobro.
3. O fluxo 2 pode ficar ativo junto: os dois escutam o mesmo tópico, e cada um
   recebe a sua cópia da mensagem.
4. **Não deixe o fluxo de ingestão da `Chatbot-CloudAI/` ativo ao mesmo tempo
   que este.** Os dois gravam em `motor_medicoes`, e o chat misturaria as
   predições da borda com as da nuvem sem ter como separar.

| Problema | Confira |
|---|---|
| MQTT não recebe | Broker, credencial e tópico `FIAPIoT/motor/validacao` |
| `classe` sai vazia no Code | **JSON Parse Body** ligado e **Only Message** desligado |
| `classe` sai `desconhecida` | o ESP32 previu um índice fora de 0–3; confira os `.hpp` do modelo |
| Hora 3 h adiantada | `timeZone: "America/Sao_Paulo"` nas duas chamadas |
| Erro de sintaxe no Postgres | as aspas simples em volta de cada `{{ }}` |
| Postgres não conecta | Host `n8n-postgres`, não `localhost`: o n8n roda num contêiner |

Referência dos campos: [Code](https://docs.n8n.io/integrations/builtin/core-nodes/n8n-nodes-base.code/) e [Postgres](https://docs.n8n.io/integrations/builtin/app-nodes/n8n-nodes-base.postgres/).
