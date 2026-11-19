# Inferência na borda — a placa decide, a nuvem guarda e confere

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

São **quatro fluxos**, um para guardar e três para conferir:

| Fluxo | Etapa | O que faz |
|---|---|---|
| `Fluxo-1-ingestao-borda` | guardar | grava cada predição da borda em `motor_medicoes` |
| `Fluxo-2-ingestao-validacao` | conferir | pergunta à nuvem e grava as duas respostas em `motor_validacao` |
| `Fluxo-3-mcp-concordancia` | conferir | publica por MCP as três consultas da concordância |
| `Fluxo-4-chat-concordancia` | conferir | chat com um agente LLM, que usa essas consultas |

O 1 e o 2 não se falam — conversam pelo banco. Os dois escutam o **mesmo
tópico** e podem ficar ativos juntos: cada um recebe a sua cópia da mensagem.

## O dispositivo não obedece a ninguém

Repare no que sumiu do firmware em relação ao app da nuvem: `subscribe`,
`callback`, tópico de comando. O device **já respondeu** quando publica — ele
não espera nada de volta.

A consequência prática aparece quando a rede cai: o motor continua sendo
monitorado e os LEDs continuam certos. **O que para é a conferência, não o
monitoramento.** Na versão em que a nuvem decidia, cair a rede era ficar cego.

Para isso valer, o firmware não espera a rede: o Wi-Fi tem até 10 s no
`setup()`, e o broker recebe uma tentativa a cada 5 s, sem parar a leitura do
sensor. O porquê, linha a linha, está na etapa 6 do
[device-MQTT-devkit/CONSTRUIR-O-FIRMWARE.md](device-MQTT-devkit/CONSTRUIR-O-FIRMWARE.md).

## Antes de começar

A bancada do app19 no ar: a `IoT-platform` (broker, n8n e PostgreSQL) e a API do
app18, em [`api/`](../app18-Inferencia_AI_Sinais-MultiClass_IMU/api/), servindo a
rede neural. É ela que confere a placa. A chave do Gemini só entra no fluxo 4.

## 1) Treinar a floresta

[`colab/treinamento_rf_edge.ipynb`](colab/treinamento_rf_edge.ipynb) lê **o mesmo
dataset** da coleta no InfluxDB, com o mesmo `SELECT` e o mesmo corte por
rodada. Em `DEVICE`, o nome da sua placa. Sem dataset próprio, o **Plano B** do
notebook (`USAR_REFERENCIA = True`) treina com o dataset de referência.

Sai o mesmo modelo em três formatos, os três no mesmo `.zip`:

| Arquivo | Serve para |
|---|---|
| `ModeloMotorRF.hpp` | as 15 árvores, em `if`/`else`, para o ESP32 |
| `ModeloMotorScaler.hpp` | a média e o desvio de cada feature, para o ESP32 |
| `modelo_motor_rf.pkl` | o Pipeline inteiro, para Python |

Descompacte e cole os dois `.hpp` direto em `device-MQTT-devkit/src/`, por cima
dos que vieram no projeto (os do dataset de referência):

```text
device-MQTT-devkit/src/
├── validacao-device.cpp
├── ModeloMotorRF.hpp        ← gerado no Colab
└── ModeloMotorScaler.hpp    ← gerado no Colab
```

> Os dois **sempre do mesmo treino**. O scaler guarda a escala em que os limiares
> da floresta foram escritos; misturar o scaler de uma execução com a floresta de
> outra não dá erro de compilação — dá predição errada, em silêncio.

A seção 9 do notebook imprime a linha do vetor `NOMES_CLASSES[4]`. Confira contra
a do `.cpp`: é ela que transforma o número que sai do `predict()` de volta em
nome de classe.

O `.pkl` fica para Python: a API continua servindo a rede neural, que é a segunda
opinião da conferência.

## 2) O firmware

```bash
cd device-MQTT-devkit
pio run
```

Ajuste no `.cpp` o Wi-Fi, o `MQTT_SERVER` e o nome da placa: `MQTT_CLIENT_ID`,
no lugar de `DEFINA_UM_NOME`. Sem os `.hpp` da sua placa, o projeto compila com
os do dataset de referência, os mesmos que o notebook gera no Plano B.

Para montar o firmware do zero, em etapas que compilam:
[device-MQTT-devkit/CONSTRUIR-O-FIRMWARE.md](device-MQTT-devkit/CONSTRUIR-O-FIRMWARE.md).

| GPIO | Componente | Classe | Índice |
|---|---|---|---|
| 19 | buzzer | `anomalia` | 0 |
| 21 | LED amarelo | `inclinado_frente` | 1 |
| 18 | LED vermelho | `inclinado_tras` | 2 |
| 4 | LED azul | `operando` | 3 |
| 2 | LED onboard | — | aceso = conectado ao broker |

### O tópico leva o nome da placa

`FIAPIoT/motor/borda/<nome>`, com o `<nome>` vindo do `MQTT_CLIENT_ID`. Num
broker da turma inteira, cada placa publica no seu tópico, e o mesmo nome vai
no gatilho dos fluxos 1 e 2.

E é `borda`, não o `FIAPIoT/motor/multiclasse` do app18 e do app19. Se fosse o
mesmo, o fluxo de ingestão do app19 reagiria a estas mensagens também — ele
responderia no tópico de comando, e os dois fluxos se embolariam no mesmo
dispositivo.

### O que sai no JSON

```json
{
  "device": "grupo03",
  "predicao_borda": "operando",
  "mean_ax": 0.003, "mean_ay": -0.001, "mean_az": 0.999,
  "std_ax": 0.007, "std_ay": 0.007, "std_az": 0.009,
  "std_mag": 0.009, "p2p_mag": 0.063
}
```

As oito features vão para a nuvem rodar o modelo dela **nos mesmos números**, e
a `predicao_borda` vai junto para haver o que comparar. Sem ela chegaria uma
predição só, e não existiria conferência nenhuma.

A predição vai como **nome**, não como índice: a API também devolve nome, e
comparar texto com texto dispensa qualquer tabela de tradução no meio.

Para **guardar**, só a `predicao_borda` interessa; as oito features esperam a
etapa de **conferir**. É a mesma mensagem servindo às duas.

### O que sai no Serial

Uma janela por segundo:

```text
--- Janela fechada ---
  mean_ax=0.042  mean_ay=0.714  mean_az=0.671
  std_ax=0.008   std_ay=0.011   std_az=0.009
  std_mag=0.008  p2p_mag=0.076
  PREDIÇÃO:  1 -> inclinado_frente
  Inferencia: N us
----------------------
```

Uma janela real do dataset de referência, com o motor inclinado uns 45° para a
frente. O `N` da última linha é o tempo do scaler mais a floresta, em
microssegundos: anote o da sua placa.

No Wokwi a **inclinação** sai: o MPU6050 tem controle de aceleração em X, Y e Z.
A `anomalia` não, porque ela é vibração e o controle parado deixa as 100
amostras da janela idênticas. Use o bloco `(A) Wokwi` da rede, troque o
`#define MPU_TYPE` para `MPU6050` e comente a linha
`mpu.calibrateAccelGyro(&calib);` — sem isso o ESP32 aborta no simulador, com
"Guru Meditation: IntegerDivideByZero".

### Só decidir, sem rede

[`device-local-devkit/`](device-local-devkit/README.md) é o mesmo firmware sem a
metade da rede: sem Wi-Fi, sem broker, sem n8n. Ocupa 23% da flash; este, que
decide e publica, volta para os 60% do firmware da nuvem. Quem pesa é a rede,
não o modelo.

### Na ESP32-C3 do professor

`device-MQTT-esp32c3/` e `device-local-esp32c3/` são os mesmos dois firmwares na
montagem da ESP32-C3 Super Mini Plus com motor de vibração, a do app18. Os mesmos
`.hpp`, o mesmo JSON e o mesmo tópico: os fluxos não mudam, e o nome da placa é
`IoTDevMultiClasse001`. Pinos e cores:
[device-MQTT-esp32c3/README.md](device-MQTT-esp32c3/README.md).

## 3) A predição da borda no banco

Importe `n8n/Fluxo-1-ingestao-borda.json` e configure as credenciais de **MQTT**
e **PostgreSQL**. São dois nós depois do gatilho:

| # | Nó | O que faz |
|---|---|---|
| 1 | `Janela da placa` | escuta `FIAPIoT/motor/borda/<nome>` e recebe a mensagem, já como objeto (**JSON Parse Body**) |
| 2 | `Formata para o banco` | lê a `predicao_borda` e calcula a data, a hora e o período |
| 3 | `Armazena a predicao` | cria a tabela se não existir e insere |

Troque `DEFINA_UM_NOME` pelo nome da placa em **Topics**, no nó 1.

É o fluxo de ingestão do app19 **sem** o `Predict Motor` e sem o
tópico de comando: a classe já chega decidida, e não há nada a devolver ao
dispositivo. Muda uma linha no nó 2 — de onde vem a classe:

```js
const classe = $input.first().json.message.predicao_borda;
```

Para montar o fluxo do zero, nó a nó:
[n8n/CONSTRUIR-O-FLUXO-1.md](n8n/CONSTRUIR-O-FLUXO-1.md).

A tabela é a mesma `motor_medicoes` do app19, com as mesmas colunas. Por isso
o chat de lá (`n8n/Fluxo-2-chat-llm.json` do app19) funciona aqui sem mudar
nada: ele não sabe, nem precisa saber, quem classificou.

**Desative o fluxo de ingestão do app19** antes de ativar este. Os dois gravam em
`motor_medicoes`, e o chat misturaria as predições da borda com as da nuvem sem
ter como separar.

```sql
CREATE TABLE motor_medicoes (
  id                SERIAL PRIMARY KEY,
  classe            TEXT,
  timestamp_medicao TEXT,
  periodo           TEXT,
  created_at        TIMESTAMPTZ DEFAULT NOW()
);
```

## 4) Levar os dados para fora: `.csv`

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

## 5) A ingestão da comparação

Importe `n8n/Fluxo-2-ingestao-validacao.json` e configure as credenciais de
**MQTT** e **PostgreSQL**. São três nós depois do gatilho:

| # | Nó | O que faz |
|---|---|---|
| 1 | `Janela da placa` | o mesmo tópico do fluxo 1 (troque `DEFINA_UM_NOME` aqui também); recebe a janela e a predição da borda, já como objeto (**JSON Parse Body**) |
| 2 | `Predict Motor (nuvem)` | `POST` do `message` para a API, que ignora os campos a mais |
| 3 | `Compara borda e nuvem` | monta as duas predições e o `concordam` |
| 4 | `Armazena a comparacao` | cria a tabela se não existir e insere |

No nó 3 deste fluxo há um detalhe que vale explicar em voz alta: depois do
`POST`, o item que circula é a **resposta da API** — a predição da borda ficou
para trás. Por isso ela é buscada pelo nome do nó:

```js
const nuvem = $input.first().json.class;
const borda = $('Janela da placa').first().json.message.predicao_borda;
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

## 6) A medida que importa

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

Medido com o dataset de referência, as 360 janelas passando pelos dois
fluxos: concordaram em 359 (**99,7%**). Na única discordância, a borda disse
`anomalia` e a nuvem `inclinado_tras` — e a janela era de anomalia: quem errou
foi a nuvem.

O número que interessa não é "errou/acertou", é a **taxa de concordância** e,
principalmente, **se ela muda com o tempo**. Uma taxa estável em 95% é um
sistema saudável. A mesma taxa caindo para 60% ao longo de uma semana é o
sinal de que alguma coisa mudou — o motor, a montagem do sensor, ou o mundo.

Se as duas discordarem sempre da mesma forma — por exemplo, a borda dizendo
`operando` onde a nuvem diz `anomalia` —, vale conferir a janela que causou
isso: ela está no Serial Monitor do dispositivo, que imprime as oito features a
cada segundo.

## 7) O chat, com as consultas por MCP

A taxa acima é uma consulta que você roda. Mas ela também pode ser uma
**pergunta em português**, do mesmo jeito que o chat do app19 pergunta pelo
estado do motor. E as consultas chegam ao agente **por MCP**, como no fim do
app19 ([As mesmas ferramentas, publicadas](../app19-AI-LLM-Cloud/README.md#7-as-mesmas-ferramentas-publicadas-mcp)):
um fluxo publica as três consultas num endereço, e o chat só aponta para ele.

1. Importe `n8n/Fluxo-3-mcp-concordancia.json`, configure a credencial do
   **PostgreSQL** nas três ferramentas e **ative**. Ele publica as consultas em
   `http://localhost:5678/mcp/concordancia`.
2. Importe `n8n/Fluxo-4-chat-concordancia.json`, configure a credencial do
   **Gemini** (veja [A chave do Gemini](../app19-AI-LLM-Cloud/README.md#a-chave-do-gemini)),
   ative e abra a URL que o nó de chat mostra.

O fluxo 3 publica:

| Nó | Papel |
|---|---|
| `MCP Server Trigger` | publica as três ferramentas em `/mcp/concordancia` |
| `concordancia_geral` | a taxa sobre todas as janelas gravadas |
| `concordancia_periodo` | a taxa em `WHERE data = $1 AND periodo = $2` |
| `onde_discordam` | quais classes se confundiram, da mais frequente para a menos |

O fluxo 4 conversa:

| Nó | Papel |
|---|---|
| `When chat message received` | abre a janela de chat |
| `FIoT Agent` | decide qual ferramenta usar e redige a resposta |
| `Google Gemini Chat Model` | o modelo de linguagem (`gemini-3.5-flash-lite`) |
| `Simple Memory` | lembra as mensagens anteriores da conversa |
| `MCP Client` | busca as três ferramentas no endereço do fluxo 3 |

Nenhuma consulta mora no chat. O agente recebe de cada ferramenta o nome, com o
prefixo do nó (`MCP_Client_concordancia_geral`), a descrição que você escreveu no
fluxo 3 e o que ela espera de entrada; a descrição continua sendo a única
instrução para ele escolher. O endereço está sem senha, como no app19: serve
para a aula.

### Quem faz a conta é o Postgres, não o LLM

Repare no SQL das três ferramentas, no fluxo 3: a agregação está toda lá dentro,
e o agente recebe `taxa_pct` já pronto. Isso é deliberado.

Se o modelo de linguagem recebesse as 300 linhas cruas e a pergunta *"qual a taxa de
concordância?"*, ele devolveria um número plausível e errado — **modelo de
linguagem não conta**. O `COUNT(*) FILTER` roda no banco, e o LLM fica só com o
trabalho que é dele: escolher a ferramenta e escrever a frase.

É a mesma decisão do `concordam`, calculado no nó 3 do fluxo 2 e gravado
pronto: **trabalho feito antes é trabalho que o LLM não precisa acertar.**

E a *system message* vai um passo além da tradução: ela traz as faixas de
leitura — acima de 95% está saudável, abaixo de 85% é hora de retreinar. O
número sai do banco; o que fazer com ele está escrito no prompt.

## 8) Testar

Com o ESP32 rodando e os fluxos 2, 3 e 4 **ativos**, espere alguns segundos e
pergunte no chat:

- *"O modelo da borda está confiável?"* → usa `concordancia_geral`
- *"E ontem à noite?"* → usa `concordancia_periodo`
- *"Em quais classes eles discordam?"* → usa `onde_discordam`
- *"Preciso retreinar?"* → usa a taxa que já está na conversa e as faixas da
  *system message*

## Estrutura

```text
colab/                  treinamento_rf_edge.ipynb · modelo_motor_rf.pkl (o do dataset de referência)
device-MQTT-devkit/     a placa decide e publica: o firmware da aula + os dois .hpp
device-local-devkit/    a mesma decisão, sem rede
device-MQTT-esp32c3/    o firmware da aula, na ESP32-C3 do professor (com motor de vibração)
device-local-esp32c3/   a mesma decisão, sem rede, na ESP32-C3
n8n/                    Fluxo-1 (guardar) · Fluxo-2 (conferir) · Fluxo-3 (as consultas, por MCP) · Fluxo-4 (o chat)
```

A API é a do app18; a plataforma, a mesma `IoT-platform`.

## Se der errado

**Sempre a mesma classe, em qualquer posição.** Os dois `.hpp` vieram de treinos
diferentes: um scaler antigo com uma floresta nova joga todas as janelas para o
mesmo lado da primeira comparação. Copie os dois do mesmo `.zip`.

**`ModeloMotorRF.hpp: No such file or directory`.** O arquivo não está direto em
`src/`. Ao descompactar, o Windows cria uma pasta com o nome do `.zip`:
copie os dois `.hpp`, não a pasta.

**`'Eloquent' does not name a type`.** O `ModeloMotorRF.hpp` está lá, mas não é o
que o micromlgen gerou, ou foi colado pela metade: ele precisa conter
`namespace Eloquent`.

**O motor operando acende o amarelo.** A floresta é a do dataset de referência e a
montagem do seu sensor é outra (ou mudou desde a coleta): treine com os dados da
sua placa. O mesmo amarelo aparece se o `predict()` receber as features sem
passar pelo scaler.

**As inclinações saem trocadas.** Confira o `NOMES_CLASSES[4]` contra a linha que
o Colab imprimiu e, depois, a orientação física do sensor: girar o sensor 90° no
gabarito troca `mean_ax` por `mean_ay`.

**Predição diferente da que o Colab mostrou.** Confira primeiro se os dois `.hpp`
são do mesmo treino. Se forem, o problema é paridade de features: vá para o
checklist do [guia do firmware](device-MQTT-devkit/CONSTRUIR-O-FIRMWARE.md).

**`concordam` é sempre `true`, em todas as janelas.** Confira se a API está
servindo a **rede neural** (`modelo_motor_multiclasse.pkl`). Se alguém apontar
o `MODELO_ARQUIVO` para a mesma Random Forest que está na flash, a comparação
vira tautologia: mesmo modelo, mesmos números, mesma resposta, para sempre.

**Nada chega no fluxo.** Confira o nome: o `MQTT_CLIENT_ID` do firmware e o
tópico dos dois gatilhos têm que bater, letra por letra
(`FIAPIoT/motor/borda/<nome>`). Um
`mosquitto_sub -h localhost -t "FIAPIoT/motor/borda/#" -v` mostra quem está
publicando, e em qual tópico.

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
propósito. O Serial mostra `MQTT: sem conexao - a decisao ficou so na placa`;
confira o Wi-Fi e o `MQTT_SERVER`.

**O chat dá erro no `MCP Client`, ou diz que não tem como consultar.** O fluxo 3
tem que estar **ativo**: é ele que publica as consultas. E o Endpoint do `MCP
Client` é `http://localhost:5678/mcp/concordancia`: dentro do n8n, `localhost` é
o próprio n8n.

**O chat responde um número que não bate com o `SELECT`.** O agente não deveria
calcular nada. Confira se as `query` das ferramentas vieram completas na
importação do fluxo 3 — se o `COUNT(*) FILTER` se perder, o LLM passa a inventar.

**O agente sempre usa a ferramenta errada.** É o sintoma clássico de modelo
pequeno. Confira se as descrições das três ferramentas vieram completas: elas
são a única instrução que o agente tem para escolher.
