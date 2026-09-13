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
tampa ao mesmo tempo, o Node-RED manda dois eventos e saem duas mensagens. Sem nó
de código e sem nó de separar: o evento já chega pronto, um motivo por mensagem.

**a) Volte no MQTT Trigger** e ligue duas opções (Add Option):

| Opção | Por quê |
|---|---|
| **JSON Parse Body** | `message` chega como texto; assim vira objeto sozinho |
| **Only Message** | tira o envelope: o item passa a ser o próprio evento |

Execute de novo. Onde antes vinha `{topic, message}`, agora vêm `device`, `sensor`,
`texto`, `valor` e `limite`, no primeiro nível.

**b) Nó Switch**, chamado `Qual limiar?`. Routing Rules sobre `{{ $json.sensor }}`,
`is equal to`. Compara a chave, nunca a prosa:

| Saída | Valor | Rename output |
|---|---|---|
| 1 | `temperatura` | Temperatura |
| 2 | `umidade` | Umidade |
| 3 | `tampa` | Tampa |
| 4 | `movimento` | Movimentação |

Em **Options**, acrescente `Fallback Output` = `Extra Output`. É a saída 5, por onde
saem as chaves `falha` e `normal`.

**c) Cinco nós Telegram**, um por saída, action `Send a Text Message`. Credencial do
@BotFather e o seu Chat ID em todos. Só o campo **Text** muda — com o botão de
expressão ligado:

```
NexoLog | {{ $json.device }}
{{ $json.texto }}
Distância: {{ $json.valor.toFixed(1) }} cm (limite {{ $json.limite }})
```

Trocando a terceira linha em cada um:

| Nó | Terceira linha |
|---|---|
| `Avisar: temperatura` | `Temperatura: {{ $json.valor.toFixed(1) }} °C (limite {{ $json.limite }})` |
| `Avisar: umidade` | `Umidade: {{ $json.valor.toFixed(1) }} % (limite {{ $json.limite }})` |
| `Avisar: tampa` | `Distância: {{ $json.valor.toFixed(1) }} cm (limite {{ $json.limite }})` |
| `Avisar: movimentação` | `Movimentação: {{ $json.valor.toFixed(2) }} m/s² (limite {{ $json.limite }})` |
| `Avisar: outro` | sem terceira linha |

**Save** e **Active**.

> **Antes de ativar o Telegram.** O Node-RED publica a cada leitura, 2,5 s — de
> propósito, para a demonstração ficar viva. O Telegram bloqueia com esse ritmo.
> Para demonstrar o **fluxo**, deixe as credenciais do Telegram de fora e acompanhe
> pelas execuções. Para demonstrar o **aviso**, aumente `INTERVALO_COLETA` no firmware
> ou ponha um nó **delay** em `Rate Limit` antes do `mqtt out` no Node-RED.

**Funcionou?** Abra a tampa no Wokwi:

```
NexoLog | NexoLogEquipe01
Tampa aberta
Distância: 40.0 cm (limite 25)
```

- [ ] Chega uma execução a cada 2,5 s, sem parar
- [ ] Com a caixa boa, só a saída 5 acende
- [ ] Abrindo a tampa, só o `Avisar: tampa` acende
- [ ] Tampa aberta **e** 35 °C: duas execuções, uma em cada saída

| Deu errado | Onde olhar |
|---|---|
| Campos vazios | faltou `JSON Parse Body` ou `Only Message` no trigger |
| Tudo cai na saída 5 | a regra compara `sensor`, não o texto. Confira a chave no `motivo(...)` do Node-RED |
| `valor.toFixed is not a function` | é o motivo `falha` ou `normal`, que não têm valor: eles saem pela saída 5 |
| `Bad Request: chat not found` | Chat ID errado, ou você nunca falou com o bot primeiro |
| Manda `{{ $json.device }}` literal | o campo Text está em modo fixo, não expressão |
| Telegram parou de responder | a taxa: veja o aviso acima |

O fluxo completo está em [fluxo_mqtt.json](fluxo_mqtt.json) — Import from File, para comparar com o seu.

Uma equipe, um workflow ativo. Dois workflows no mesmo tópico mandam a mensagem duas vezes.
