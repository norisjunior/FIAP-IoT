# Chatbot-EdgeAI — a borda decide, a nuvem guarda e confere

O dispositivo decide sozinho, com a Random Forest que mora na flash dele. Isso é
ótimo: responde em microssegundos e funciona sem rede. O que ele decide, ele
publica — e este app faz duas coisas com isso, nesta ordem.

**Primeiro, guardar.** Cada predição da borda vai para o PostgreSQL com a data, a
hora e o período do dia — e dali pode sair num `.csv` para qualquer outra
ferramenta. Nenhum modelo roda na nuvem: a classe já chega pronta.

```text
ESP32 ─► decide na flash ─► acende a saída
   │
   └─► MQTT (predição da borda) ─► n8n ─► PostgreSQL ─► .csv
```

**Depois, conferir.** Com o dado guardado, aparece uma pergunta que não existia
enquanto a nuvem decidia tudo:

> **Como eu sei que o modelo pequeno da ponta continua certo?**

O n8n faz a mesma pergunta ao modelo maior que roda na nuvem e guarda as duas
respostas lado a lado.

```text
   └─► MQTT (janela + predição da borda) ─► n8n ─► API (a rede neural) ─► PostgreSQL
                                                              └► as duas respostas
```

Na indústria isso se chama **shadow mode**: o modelo grande roda em paralelo,
sem mandar em nada, só para medir o quanto o modelo pequeno concorda com ele.

São **três fluxos**, um para guardar e dois para conferir:

| Fluxo | Etapa | O que faz |
|---|---|---|
| `Fluxo-1-ingestao-borda` | guardar | grava cada predição da borda em `motor_medicoes` |
| `Fluxo-2-ingestao-validacao` | conferir | pergunta à nuvem e grava as duas respostas em `motor_validacao` |
| `Fluxo-3-chat-concordancia` | conferir | chat com um agente LLM sobre a concordância |

Os fluxos não se falam — conversam pelo banco. O 1 e o 2 escutam o **mesmo
tópico** e podem ficar ativos juntos: cada um recebe a sua cópia da mensagem.

## O dispositivo não obedece a ninguém

Repare no que sumiu do firmware em relação ao app da nuvem: `subscribe`,
`callback`, tópico de comando. O device **já respondeu** quando publica — ele
não espera nada de volta.

A consequência prática aparece quando a rede cai: o motor continua sendo
monitorado e os LEDs continuam certos. **O que para é a conferência, não o
monitoramento.** Na versão em que a nuvem decidia, cair a rede era ficar cego.

## 1) O firmware

```bash
cd device
pio run
```

Ajuste no `.cpp` o Wi-Fi e o `MQTT_SERVER`. Os dois `.hpp` do modelo já estão em
`device/src/` — são os mesmos que você gerou no Colab do app anterior. Se
treinou um modelo novo, troque os dois **juntos**.

Para montar o firmware do zero, em etapas que compilam:
[CONSTRUIR-O-FIRMWARE.md](CONSTRUIR-O-FIRMWARE.md).

| GPIO | Componente | Classe | Índice |
|---|---|---|---|
| 19 | buzzer | `anomalia` | 0 |
| 21 | LED amarelo | `inclinado_frente` | 1 |
| 18 | LED vermelho | `inclinado_tras` | 2 |
| 4 | LED azul | `operando` | 3 |
| 2 | LED onboard | — | aceso = conectado ao broker |

### O tópico é outro

`FIAPIoT/motor/validacao`, e não o `FIAPIoT/motor/multiclasse` das outras
aplicações. Se fosse o mesmo, o fluxo da `Chatbot-CloudAI` reagiria a estas
mensagens também — ele responderia no tópico de comando, e as duas aplicações
se embolariam no mesmo dispositivo.

### O que sai no JSON

```json
{
  "device": "IoTDevValidacaoMotor001",
  "predicao_borda": "operando",
  "mean_ax": -0.124, "mean_ay": -0.014, "mean_az": 0.879,
  "std_ax": 0.248, "std_ay": 0.133, "std_az": 0.225,
  "std_mag": 0.203, "p2p_mag": 0.781
}
```

As oito features vão para a nuvem rodar o modelo dela **nos mesmos números**, e
a `predicao_borda` vai junto para haver o que comparar. Sem ela chegaria uma
predição só, e não existiria conferência nenhuma.

A predição vai como **nome**, não como índice: a API também devolve nome, e
comparar texto com texto dispensa qualquer tabela de tradução no meio.

Para **guardar**, só a `predicao_borda` interessa; as oito features esperam a
etapa de **conferir**. É a mesma mensagem servindo às duas.

## 2) A predição da borda no banco

Importe `n8n/Fluxo-1-ingestao-borda.json` e configure as credenciais de **MQTT**
e **PostgreSQL**. São dois nós depois do gatilho:

| # | Nó | O que faz |
|---|---|---|
| 1 | `FIAPIoT/motor/validacao` | recebe a mensagem, já como objeto (**JSON Parse Body**) |
| 2 | `Formata para o banco` | lê a `predicao_borda` e calcula a data, a hora e o período |
| 3 | `Armazena a predicao` | cria a tabela se não existir e insere |

É o fluxo de ingestão da `Chatbot-CloudAI/` **sem** o `Predict Motor` e sem o
tópico de comando: a classe já chega decidida, e não há nada a devolver ao
dispositivo. Muda uma linha no nó 2 — de onde vem a classe:

```js
const classe = $input.first().json.message.predicao_borda;
```

A tabela é a mesma `motor_medicoes` da `Chatbot-CloudAI/`, com as mesmas
colunas. Por isso o chat de lá (`Chatbot-CloudAI/n8n/Fluxo-2-chat-llm.json`)
funciona aqui sem mudar nada: ele não sabe, nem precisa saber, quem classificou.

```sql
CREATE TABLE motor_medicoes (
  id                SERIAL PRIMARY KEY,
  classe            TEXT,
  timestamp_medicao TEXT,
  periodo           TEXT,
  created_at        TIMESTAMPTZ DEFAULT NOW()
);
```

## 3) Levar os dados para fora: `.csv`

O banco guarda; quem quiser usar o dado em outra ferramenta — um notebook, uma
planilha, outro automatizador, um agente LLM — leva a tabela inteira num `.csv`.
No terminal onde roda a plataforma:

```bash
docker exec n8n-postgres psql -U n8nuser -d n8n \
  -c "\copy motor_medicoes TO STDOUT WITH CSV HEADER" > motor_medicoes.csv
```

```text
id,classe,timestamp_medicao,periodo,created_at
1,operando,2026-09-30 08:00:01,manhã,2026-09-30 11:00:01.698538+00
2,anomalia,2026-09-30 14:00:01,tarde,2026-09-30 17:00:01.104211+00
```

O `\copy` roda **dentro** do contêiner e escreve na saída padrão; o `>` do
seu terminal é que grava o arquivo na sua máquina. Por isso o arquivo aparece
na pasta em que você está, e não dentro do Docker.

Use o `timestamp_medicao`, não o `created_at`. Os dois marcam o mesmo instante,
mas o `created_at` está em UTC — três horas à frente de São Paulo —, e o
`periodo` foi calculado pelo `timestamp_medicao`. Misturar os dois faz uma
medição das 22h aparecer no dia seguinte.

Para conferir, no Python:

```python
import pandas as pd
df = pd.read_csv("motor_medicoes.csv")
print(df.groupby(["periodo", "classe"]).size())
```

## 4) A ingestão da comparação

Importe `n8n/Fluxo-2-ingestao-validacao.json` e configure as credenciais de
**MQTT** e **PostgreSQL**. São três nós depois do gatilho:

| # | Nó | O que faz |
|---|---|---|
| 1 | `FIAPIoT/motor/validacao` | recebe a janela e a predição da borda, já como objeto (**JSON Parse Body**) |
| 2 | `Predict Motor (nuvem)` | `POST` do `message` para a API, que ignora os campos a mais |
| 3 | `Compara borda e nuvem` | monta as duas predições e o `concordam` |
| 4 | `Armazena a comparacao` | cria a tabela se não existir e insere |

No nó 3 deste fluxo há um detalhe que vale explicar em voz alta: depois do
`POST`, o item que circula é a **resposta da API** — a predição da borda ficou
para trás. Por isso ela é buscada pelo nome do nó:

```js
const nuvem = $input.first().json.class;
const borda = $('FIAPIoT/motor/validacao').first().json.message.predicao_borda;
```

### A tabela

```sql
CREATE TABLE motor_validacao (
  id                SERIAL PRIMARY KEY,
  predicao_borda    TEXT,
  predicao_nuvem    TEXT,
  concordam         BOOLEAN,
  timestamp_medicao TEXT,
  periodo           TEXT,
  created_at        TIMESTAMPTZ DEFAULT NOW()
);
```

O `concordam` é calculado no n8n e gravado pronto. Daria para deixar a conta
para o `SELECT`, mas assim a consulta que interessa fica trivial — e é ela que
você vai projetar na aula.

## 5) A medida que importa

```sql
SELECT
  COUNT(*)                                   AS janelas,
  COUNT(*) FILTER (WHERE concordam)          AS concordaram,
  ROUND(100.0 * COUNT(*) FILTER (WHERE concordam)
        / NULLIF(COUNT(*), 0), 1)            AS taxa_pct
FROM motor_validacao;
```

Para rodar direto no banco, sem sair do terminal:

```bash
docker exec -it n8n-postgres psql -U n8nuser -d n8n
```

E, quando discordarem, quais classes se confundiram:

```sql
SELECT predicao_borda, predicao_nuvem, COUNT(*)
FROM motor_validacao
WHERE NOT concordam
GROUP BY 1, 2
ORDER BY 3 DESC;
```

### Quando discordam, quem está errado?

Nenhum dos dois, necessariamente. São **modelos diferentes**: na flash roda uma
floresta de 15 árvores, na nuvem roda uma rede neural. Treinados com o mesmo
dataset, mas aprendendo de jeitos diferentes — é natural que discordem nas
janelas ambíguas, aquelas que caem perto da fronteira entre duas classes.

O número que interessa não é "errou/acertou", é a **taxa de concordância** e,
principalmente, **se ela muda com o tempo**. Uma taxa estável em 95% é um
sistema saudável. A mesma taxa caindo para 60% ao longo de uma semana é o
sinal de que alguma coisa mudou — o motor, a montagem do sensor, ou o mundo.

Se as duas discordarem sempre da mesma forma — por exemplo, a borda dizendo
`operando` onde a nuvem diz `anomalia` —, vale conferir a janela que causou
isso: ela está no Serial Monitor do dispositivo, que imprime as oito features a
cada segundo.

## 6) O fluxo do chat

A taxa acima é uma consulta que você roda. Mas ela também pode ser uma
**pergunta em português**, do mesmo jeito que a `Chatbot-CloudAI/` pergunta pelo
estado do motor. Importe `n8n/Fluxo-3-chat-concordancia.json` e configure as
credenciais de **Ollama** e **PostgreSQL**.

| Nó | Papel |
|---|---|
| `When chat message received` | abre a janela de chat |
| `FIoT Agent` | decide qual ferramenta usar e redige a resposta |
| `Ollama Chat Model` | o modelo de linguagem (`llama3.2:1b`) |
| `Simple Memory` | lembra as mensagens anteriores da conversa |
| `concordancia_geral` | a taxa sobre todas as janelas gravadas |
| `concordancia_periodo` | a taxa em `WHERE data = $1 AND periodo = $2` |
| `onde_discordam` | quais classes se confundiram, da mais frequente para a menos |

Ative o fluxo e abra a URL que o nó de chat mostra.

### Quem faz a conta é o Postgres, não o LLM

Repare no SQL das três ferramentas: a agregação está toda lá dentro, e o agente
recebe `taxa_pct` já pronto. Isso é deliberado.

Se o `llama3.2:1b` recebesse as 300 linhas cruas e a pergunta *"qual a taxa de
concordância?"*, ele devolveria um número plausível e errado — **modelo de
linguagem não conta**. O `COUNT(*) FILTER` roda no banco, e o LLM fica só com o
trabalho que é dele: escolher a ferramenta e escrever a frase.

É a mesma decisão do `concordam`, calculado no nó 3 do fluxo 2 e gravado
pronto: **trabalho feito antes é trabalho que o LLM não precisa acertar.**

E a *system message* vai um passo além da tradução: ela traz as faixas de
leitura — acima de 95% está saudável, abaixo de 85% é hora de retreinar. O
número sai do banco; o que fazer com ele está escrito no prompt.

## 7) Testar

Com o ESP32 rodando e o fluxo 2 **ativo**, espere alguns segundos e
pergunte no chat:

- *"O modelo da borda está confiável?"* → usa `concordancia_geral`
- *"E ontem à noite?"* → usa `concordancia_periodo`
- *"Em quais classes eles discordam?"* → usa `onde_discordam`
- *"Preciso retreinar?"* → usa a taxa que já está na conversa e as faixas da
  *system message*

## Se der errado

**`concordam` é sempre `true`, em todas as janelas.** Confira se a API está
servindo a **rede neural** (`modelo_motor_multiclasse.pkl`). Se alguém apontar
o `MODELO_ARQUIVO` para a mesma Random Forest que está na flash, a comparação
vira tautologia: mesmo modelo, mesmos números, mesma resposta, para sempre.

**Nada chega no fluxo.** Confira o tópico: este app usa
`FIAPIoT/motor/validacao`. Um `mosquitto_sub -h localhost -t "FIAPIoT/motor/validacao" -v`
mostra se o dispositivo está publicando.

**O `.csv` sai vazio ou com erro `relation "motor_medicoes" does not exist`.**
A tabela nasce no primeiro insert do fluxo 1. Confira se ele está **ativo** e se
já passou pelo menos uma mensagem.

**O `manhã` aparece como `manhÃ£` na planilha.** O arquivo é UTF-8 e o Excel
abriu como outra codificação. Importe por *Dados → De Texto/CSV* e escolha
UTF-8; o pandas já lê certo.

**A API recusa o corpo.** Ela ignora `device` e `predicao_borda` e lê só as oito
features. Se estiver recusando, é o JSON que chegou quebrado — veja se o `message` do
nó 1 abriu em campos ou ficou como texto.

**Os LEDs acendem mas nada é gravado.** É o comportamento esperado quando o
broker está fora do ar: o dispositivo decide primeiro e publica depois, de
propósito.

**O chat responde um número que não bate com o `SELECT`.** O agente não deveria
calcular nada. Confira se as `query` das ferramentas vieram completas na
importação — se o `COUNT(*) FILTER` se perder, o LLM passa a inventar.

**O agente sempre usa a ferramenta errada.** É o sintoma clássico de modelo
pequeno. Confira se as descrições das três ferramentas vieram completas: elas
são a única instrução que o agente tem para escolher.
