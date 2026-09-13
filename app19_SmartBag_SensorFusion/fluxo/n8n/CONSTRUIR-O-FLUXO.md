# Construir o fluxo no n8n, do zero

Duas iterações. Cada uma roda.

1. O evento chegando no n8n.
2. O evento virando mensagem no Telegram.

O n8n só entra depois que o Node-RED já publica em `.../eventos`:
[CONSTRUIR-O-FLUXO.md do Node-RED](../NodeRED/CONSTRUIR-O-FLUXO.md).

`http://localhost:5678`

---

## Iteração 1 — Ver o evento chegar

Workflow novo. Um nó só: **MQTT Trigger**.

| Campo | Valor |
|---|---|
| Credential | Create new · Host `mosquitto` · Port `1883` · sem usuário/senha |
| Topics | `FIAPIoT/nexolog/equipe01/eventos` |

Clique em **Listen for test event**. No Wokwi, mude a distância de 10 para 40 cm.

**Funcionou?** O nó verde, com um item:

```json
{ "topic": "FIAPIoT/nexolog/equipe01/eventos",
  "message": "{\"device\":\"NexoLogEquipe01\",\"estado\":\"Tampa aberta\",...}" }
```

- [ ] A credencial fecha com **Connection tested successfully**
- [ ] Chega um item ao mudar a distância

| Deu errado | Onde olhar |
|---|---|
| Erro na credencial | `mosquitto`, não `localhost` — os dois containers estão na mesma rede |
| Nada chega | o evento sai só na **mudança** de estado. Volte a distância para 10 e suba de novo |
| Nada chega, e o Node-RED também está mudo | o tópico `eventos` é o de saída do Node-RED, não o `dados` do ESP32 |

---

## Iteração 2 — Uma mensagem por limiar

Quatro limiares, quatro nós de Telegram. Se a entrega estoura temperatura **e**
tampa ao mesmo tempo, saem duas mensagens. Sem nó de código: três nós de ligar
e os avisos.

**a) Volte no MQTT Trigger** e ligue duas opções (Add Option):

| Opção | Por quê |
|---|---|
| **JSON Parse Body** | `message` chega como texto; assim vira objeto sozinho |
| **Only Message** | tira o envelope: o item passa a ser o próprio payload |

Execute de novo. Onde antes vinha `{topic, message}`, agora vêm `device`, `temp`,
`motivos` e o resto, no primeiro nível.

**b) Nó Split Out**, chamado `Um item por motivo`:

| Campo | Valor |
|---|---|
| Fields To Split Out | `motivos` |
| Include | `All Other Fields` |
| Destination Field Name | `motivo` |

`motivos` é a lista que o Node-RED publica. O Split Out faz um item por elemento,
carregando junto todos os outros campos — dois limiares estourados viram dois itens,
cada um com as medições completas. É o laço, sem o laço.

Cada item ganha o campo `motivo`, com as duas partes: `motivo.sensor` (a chave, para
rotear) e `motivo.texto` (a prosa, para mostrar).

**c) Nó Switch**, chamado `Qual limiar?`. Routing Rules sobre `{{ $json.motivo.sensor }}`,
`is equal to`. Compara a chave, nunca a prosa:

| Saída | Valor | Rename output |
|---|---|---|
| 1 | `temperatura` | Temperatura |
| 2 | `umidade` | Umidade |
| 3 | `tampa` | Tampa |
| 4 | `movimento` | Movimentação |

Em **Options**, acrescente `Fallback Output` = `Extra Output`. É a saída 5, por onde
saem as chaves `falha` e `normal`.

**d) Cinco nós Telegram**, um por saída, action `Send a Text Message`. Credencial do
@BotFather e o seu Chat ID em todos. Só o campo **Text** muda, e é onde mora o texto
de cada sensor — com o botão de expressão ligado:

```
NexoLog | {{ $json.device }}
{{ $json.motivo.texto }}
Temperatura: {{ $json.temp }} °C (limite 30)
{{ $json.timestamp }}
```

Trocando a terceira linha em cada um:

| Nó | Terceira linha |
|---|---|
| `Avisar: temperatura` | `Temperatura: {{ $json.temp }} °C (limite 30)` |
| `Avisar: umidade` | `Umidade: {{ $json.umid }} % (limite 70)` |
| `Avisar: tampa` | `Distância: {{ $json.dist }} cm (limite 25)` |
| `Avisar: movimentação` | `Movimentação: {{ $json.movimentacao.toFixed(2) }} m/s² (limite 3)` |
| `Avisar: outro` | sem terceira linha |

**Save** e **Active**.

**Funcionou?** Abra a tampa no Wokwi:

```
NexoLog | NexoLogEquipe01
Tampa aberta
Distância: 40.0 cm (limite 25)
2026-09-11T13:20:05.412Z
```

- [ ] Uma mensagem ao abrir, outra ao fechar
- [ ] Só o nó `Avisar: tampa` fica verde — os outros três não recebem nada
- [ ] Suba a temperatura para 35 °C com a tampa aberta: chegam **duas** mensagens

| Deu errado | Onde olhar |
|---|---|
| Campos vazios no Telegram | faltou `JSON Parse Body` ou `Only Message` no trigger |
| Tudo cai na saída 5 | a regra compara `motivo.sensor`, não o texto. Confira a chave no `motivos.push` do Node-RED |
| Nenhum item sai do Split Out | `motivos` não está no payload: veja a função "Estado da entrega" |
| Uma mensagem só, com tudo junto | o Split Out ficou de fora |
| `Bad Request: chat not found` | Chat ID errado, ou você nunca falou com o bot primeiro |
| Manda `{{ $json.device }}` literal | o campo Text está em modo fixo, não expressão |
| Uma enxurrada de mensagens | o nó "Somente mudança de estado" do Node-RED ficou de fora |

O fluxo completo está em [fluxo_mqtt.json](fluxo_mqtt.json) — Import from File, para comparar com o seu.

Uma equipe, um workflow ativo. Dois workflows no mesmo tópico mandam a mensagem duas vezes.
