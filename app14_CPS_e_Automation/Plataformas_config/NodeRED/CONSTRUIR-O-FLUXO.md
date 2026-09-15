# Construir o fluxo no Node-RED, do zero

Duas iterações. Cada uma roda.

1. O dado chegando no Debug.
2. O dado virando dashboard.

Aqui o Node-RED só **mostra**. Quem decide se algo está errado e avisa é o
[n8n](../n8n/CONSTRUIR-O-FLUXO.md), que assina o mesmo tópico.

Comece com uma aba vazia. Firmware publicando: [CONSTRUIR-O-FIRMWARE.md](../../app14_NexoLog_PUB_only/CONSTRUIR-O-FIRMWARE.md).

`http://localhost:1880`

---

## Iteração 1 — Ver o dado chegar

Três nós, nesta ordem:

| Nó | Aba | Configuração |
|---|---|---|
| **mqtt in** | network | Server: `mosquitto:1883` (lápis > Add) · Topic: `FIAPIoT/nexolog/equipe01/dados` · Output: `a String` |
| **json** | function | Action: `Always convert to JavaScript Object` |
| **debug** | common | Output: `complete msg object` |

Ligue `mqtt in → json → debug`. **Deploy.**

**Funcionou?** Na aba Debug, uma vez por segundo:

```
payload: object
  device: "NexoLogEquipe01"
  temp: 24
  umid: 40
  dist: 10
  movimentacao: 0.03
```

- [ ] O quadradinho sob o `mqtt in` diz **connected**
- [ ] Chega um objeto, não um texto entre aspas

| Deu errado | Onde olhar |
|---|---|
| `disconnected` | endereço do broker. Node-RED em Docker fala com `mosquitto`, não `localhost` |
| `connected`, nada no Debug | tópico. Um caractere diferente e não chega nada — confira `equipe01` dos dois lados |
| `payload: "{\"device\"..."` (com aspas) | faltou o nó **json** |

---

## Iteração 2 — Dashboard

Continue do `json`.

**a) Separar os valores.** Nó **function**, 4 saídas (aba Setup > Outputs). É a única
linha de código do fluxo inteiro — um `msg` por widget:

```javascript
const p = msg.payload;
let temp = {payload: p.temp, topic: "Temperatura"};
let umid = {payload: p.umid};
let dist = {payload: p.dist};
let movimentacao = {payload: p.movimentacao};
return [temp, umid, dist, movimentacao];
```

**b) Um widget por saída.** Crie o grupo uma vez, no primeiro widget (Group > Add > 12
de largura). Os quatro entram na mesma linha:

| Saída | Nó | Order | Size | Ajustes |
|---|---|---|---|---|
| 1 | ui_gauge | 1 | 3×3 | label `°C`, 0–50, seg 25 e 30 |
| 2 | ui_gauge | 2 | 3×3 | label `%`, 0–100, seg 60 e 70 |
| 3 | ui_level | 3 | 3×3 | label `Tampa`, unit `cm`, 0–50, warn 20, high 25, layout vertical |
| 4 | ui_gauge | 4 | 3×3 | label `m/s²`, 0–20, seg 1.5 e 3 |

Ligue a saída 1 também num **ui_chart** (order 5, 12×4).

> `ui_level` é o `node-red-contrib-ui-level`. Menu ≡ > Manage palette > Install.

As faixas de cor dos widgets repetem os limiares do n8n. São cópias: mudar o limite lá
não repinta o gauge aqui.

**c) Avisar quando o ESP32 sumir.** Nó **trigger** (aba function), ligado ao `json`:

| Campo | Valor |
|---|---|
| Send | `Recebendo dados` |
| then | `wait for` `10` `seconds` · marque **extend delay if new message arrives** |
| then send | `Sem dados há 10 s` |

Ligue num **ui_text** (order 6, 12×1, label `Conexão`, Value format `{{msg.payload}}`).

Cada leitura reinicia a contagem. Se o firmware parar, em 10 s o texto muda sozinho.

**Deploy.**

**Funcionou?** `http://localhost:1880/ui/`

- [ ] Quatro widgets numa linha só, com valor
- [ ] Sobe a distância no Wokwi de 10 para 40 cm → a barra da tampa fica vermelha
- [ ] Para o Wokwi → em 10 s a Conexão muda para "Sem dados há 10 s"

| Deu errado | Onde olhar |
|---|---|
| Widgets empilhados | somam mais de 12 de largura, ou estão em grupos diferentes |
| Gauge vazio | a saída da function não bate com o nó, ou o campo mudou de nome no firmware |
| Conexão nunca muda | faltou marcar **extend delay** no trigger |

O fluxo completo está em [dashboard.json](dashboard.json) — ≡ > Import, para comparar
com o seu.

---

## O segundo fluxo: guardar no InfluxDB

O gráfico da tela mostra os últimos cinco minutos e começa do zero a cada F5. Para
perguntar "como foi a entrega de ontem", o dado precisa estar num banco.

Importe [Fluxo_2_envio_InfluxDB.json](Fluxo_2_envio_InfluxDB.json) numa **aba nova**.
Ele assina o mesmo tópico `dados` — os dois fluxos rodam juntos, e nenhum atrapalha o
outro. É a mesma ideia do n8n: um tópico MQTT entrega para todo mundo que assinar.

Usamos o **InfluxDB Cloud**. Quatro campos para preencher, e nenhum vem pronto:

| Onde | Campo | O que é |
|---|---|---|
| nó de configuração (lápis) | URL | `https://<sua-regiao>.aws.cloud2.influxdata.com` |
| nó de configuração | Token | Load Data > API Tokens, no site do InfluxDB |
| `Gravar leituras` | Organization | a sua |
| `Gravar leituras` | Bucket | o seu |

O **token nunca vem no arquivo**: o Node-RED guarda token como credencial e a
exportação sempre remove. Isso é proposital — é o que permite compartilhar um fluxo
sem vazar acesso ao seu banco.

O nó `Campos e tags` existe por **um** motivo: transformar o objeto em `[campos, tags]`,
que é o formato em que o nó do InfluxDB separa uma coisa da outra.

```javascript
// device sai dos campos e vira tag. Mandar nos dois lugares e conflito de schema.
const {device, ...campos} = msg.payload;
msg.payload = [campos, {device}];
return msg;
```

A primeira linha separa o `device` do resto: ele sai do objeto e as outras sete chaves
ficam em `campos`. **Não dá para mandar o mesmo nome nos dois lugares** — o InfluxDB
guarda o tipo de cada coluna, e uma coluna não pode ser field numa linha e tag na
outra. Se acontecer, a escrita inteira é recusada com `batch schema conflict`.

Sem essa linha, dava para ligar o `json` direto no InfluxDB — e funcionaria. Só que
`device` entraria como **field**, um texto qualquer no meio dos números. Como **tag** ele
é indexado: filtrar por caixa fica barato, e `group by device` passa a existir. É a
única coisa que essa função faz.

Repare no que ela **não** faz: não filtra, não arredonda, não decide nada. Leitura que
falhou chega como `null` e segue assim. O banco guarda o que veio, e a pergunta se faz
depois — dataset de verdade tem buraco, e lidar com isso é parte do trabalho.

**Funcionou?** No InfluxDB, Data Explorer:

- [ ] O bucket aparece com o measurement `nexolog`
- [ ] Sete fields e a tag `device`
- [ ] Um ponto por segundo, acompanhando o Wokwi

| Deu errado | Onde olhar |
|---|---|
| `unauthorized` | token errado, ou colado com espaço no fim |
| `bucket not found` | o bucket é o **seu**, não o do colega; confira a organização também |
| `getaddrinfo ENOTFOUND` | a URL tem a região da sua conta, e não a do exemplo |
| `batch schema conflict ... 'device'` | `device` está indo como field e como tag: ele tem que sair de `campos` |
| Grava, mas falta um campo | aquele sensor mandou `null` naquele instante — o buraco é proposital |
| A escrita inteira falha com `null` | veja a nota abaixo: talvez precise voltar a filtrar |

> **Confira na primeira aula.** Desligue o DHT no Wokwi e veja o que acontece com
> `temp: null`. Se o ponto for gravado sem o campo `temp`, é exatamente o que
> queremos: buraco numa coluna só. Se a escrita inteira for recusada, a linha toda
> se perde — aí vale voltar a montar `campos` só com o que é número, como estava
> antes, para o resto da leitura sobreviver.

Pronto: falta [o aviso no n8n](../n8n/CONSTRUIR-O-FLUXO.md).
