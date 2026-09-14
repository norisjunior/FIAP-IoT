# Construir o fluxo no n8n, do zero

Duas iterações. Cada uma roda.

1. O dado do ESP32 chegando no n8n.
2. O n8n decidindo e avisando no Telegram.

O n8n assina **o mesmo tópico do ESP32**, `.../dados`, junto com o Node-RED. Um
tópico MQTT entrega para todo mundo que assinar: o Node-RED mostra, o n8n decide.
Nenhum dos dois sabe do outro.

Sem nó de código. Um Switch compara os números, quatro nós montam o texto e um
único Telegram envia.

`http://localhost:5678`

---

## Iteração 1 — Ver o dado chegar

Workflow novo. Um nó só: **MQTT Trigger**.

| Campo | Valor |
|---|---|
| Credential | Create new · Host `mosquitto` · Port `1883` · sem usuário/senha |
| Topics | `FIAPIoT/nexolog/equipe01/dados` |
| Options | **JSON Parse Body** |

Clique em **Listen for test event**.

**Funcionou?** O nó verde, com um item por segundo:

```json
{ "topic": "FIAPIoT/nexolog/equipe01/dados",
  "message": { "device": "NexoLogEquipe01", "temp": 24, "umid": 40,
               "dist": 10, "movimentacao": 0.03 } }
```

- [ ] A credencial fecha com **Connection tested successfully**
- [ ] `message` abre em campos, não é um texto entre aspas
- [ ] Chega sozinho, sem você mexer em nada

Os campos ficam **dentro** de `message`. Por isso toda expressão daqui para a frente
começa com `$json.message`.

> **Não ligue `Only Message`.** Ela promete entregar só o conteúdo, mas embrulha num
> array: o item vira `[{...}]`, `{{ $json.temp }}` fica `undefined`, nenhuma regra do
> Switch casa e as mensagens saem vazias. Dá para reconhecer na aba Schema — aparece
> um nível `0` entre o nó e os campos.

| Deu errado | Onde olhar |
|---|---|
| Erro na credencial | `mosquitto`, não `localhost` — os dois containers estão na mesma rede |
| Nada chega | o ESP32 está publicando? Confira no Debug do Node-RED, que assina o mesmo tópico |
| `message` como texto | faltou `JSON Parse Body` |

---

## Iteração 2 — Decidir e avisar

**a) Nó Switch**, chamado `Passou de algum limiar?`. Mode `Rules`. Uma regra por
variável, comparando número com número:

| Saída | Left | Operação | Right | Rename output |
|---|---|---|---|---|
| 1 | `{{ $json.message.temp }}` | Number · **is greater than** | `30` | Temperatura |
| 2 | `{{ $json.message.umid }}` | Number · is greater than | `70` | Umidade |
| 3 | `{{ $json.message.dist }}` | Number · is greater than | `25` | Tampa |
| 4 | `{{ $json.message.movimentacao }}` | Number · is greater than | `3` | Movimentação |

Em **Options**, ligue **Send data to all matching outputs**. Sem isso o Switch para na
primeira regra que casar, e a caixa quente **e** aberta avisaria só a temperatura.

Não ponha `Fallback Output`: quando nada passa do limite, nada deve sair. O aviso
serve para o que está errado.

**Os quatro limiares moram aqui**, e em nenhum outro lugar. O ESP32 não conhece
nenhum deles — ele mede e publica. Mudar um limite é mudar um número nesta tela, sem
recompilar nada. Ponha ao lado um **Sticky Note** com a tabela, para quem abrir o
workflow achar os números.

**b) Quatro nós Edit Fields (Set)**, um por saída do Switch. Cada um cria um campo
`texto`, do tipo String, com a mensagem daquela variável — modo expressão:

```
NexoLog | {{ $json.message.device }}
Tampa aberta
Distância: {{ $json.message.dist.toFixed(1) }} cm
```

Trocando as duas últimas linhas em cada um:

| Nó | Título | Linha do valor |
|---|---|---|
| `Texto: temperatura alta` | Temperatura alta | `Temperatura: {{ $json.message.temp.toFixed(1) }} °C` |
| `Texto: umidade alta` | Umidade alta | `Umidade: {{ $json.message.umid.toFixed(1) }} %` |
| `Texto: tampa aberta` | Tampa aberta | `Distância: {{ $json.message.dist.toFixed(1) }} cm` |
| `Texto: movimentação brusca` | Movimentação brusca | `Movimentação: {{ $json.message.movimentacao.toFixed(2) }} m/s²` |

**c) Um nó Telegram**, action `Send a Text Message`. Os quatro Set ligam **nele**:

| Campo | Valor |
|---|---|
| Credential | token do @BotFather |
| Chat ID | o seu — mande um "oi" para o bot e pegue em `api.telegram.org/bot<TOKEN>/getUpdates` |
| Text | `{{ $json.texto }}` (com o botão de expressão ligado) |

Um nó só, uma credencial só, e ainda assim uma mensagem diferente por variável. O que
muda é o caminho até ele, não o envio.

**Save** e **Active**.

> **Antes de ativar.** O ESP32 publica a cada 1 s, e o aviso sai a cada leitura que
> passar do limite — o Telegram bloqueia com esse ritmo. Para demonstrar o **fluxo**,
> deixe a credencial do Telegram de fora e acompanhe pelas execuções. Para demonstrar
> o **aviso**, aumente `INTERVALO_COLETA` no firmware ou ponha um nó **Wait** antes do
> Telegram.

**Funcionou?** Abra a tampa no Wokwi, de 10 para 40 cm:

```
NexoLog | NexoLogEquipe01
Tampa aberta
Distância: 40.0 cm
```

- [ ] Caixa boa: o Switch recebe e não sai nada por nenhuma saída
- [ ] Tampa aberta: só a saída 3 acende
- [ ] Tampa aberta **e** 35 °C: duas saídas acendem e chegam duas mensagens
- [ ] Feche a tampa: para de avisar sozinho

| Deu errado | Onde olhar |
|---|---|
| Campos vazios, e nada casa | `Only Message` está ligada: desligue. O Schema mostra um nível `0` quando isso acontece |
| `{{ $json.temp }}` não resolve | faltou o `message` no meio: é `$json.message.temp` |
| Caixa quente e aberta avisa só uma coisa | faltou **Send data to all matching outputs** |
| Nunca casa nada | a regra está comparando texto com número: o operador tem que ser **Number** |
| `toFixed is not a function` | o campo veio `null` — o sensor falhou. Veja a leitura no Debug do Node-RED |
| `Bad Request: chat not found` | Chat ID errado, ou você nunca falou com o bot primeiro |
| Telegram parou de responder | a taxa: veja o aviso acima |

O fluxo completo está em [fluxo_mqtt.json](fluxo_mqtt.json) — Import from File, para
comparar com o seu.

Uma equipe, um workflow ativo. Dois workflows no mesmo tópico mandam a mensagem duas
vezes.
