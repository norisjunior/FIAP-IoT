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

## Iteração 2 — Avisar no Telegram

**a) Nó Code**, depois do trigger. Mode: `Run Once for Each Item`.

`message` chega como texto — sem o `JSON.parse` os campos vêm `undefined`:

```javascript
const data = typeof $json.message === "string" ? JSON.parse($json.message) : $json.message;
const num = (v, casas) => Number.isFinite(v) ? v.toFixed(casas) : "sem leitura";
return {json: {...data, texto: `NexoLog | ${data.device}
${data.estado}
Temperatura: ${num(data.temp, 1)} °C
Umidade: ${num(data.umid, 1)} %
Distância: ${num(data.dist, 1)} cm
Movimentação: ${num(data.movimentacao, 2)} m/s²
${data.timestamp}`}};
```

Execute o nó. O campo `texto` tem que sair pronto, com número em toda linha.

**b) Nó Telegram**, action `Send a Text Message`.

| Campo | Valor |
|---|---|
| Credential | token do @BotFather |
| Chat ID | o seu — mande um "oi" para o bot e pegue em `api.telegram.org/bot<TOKEN>/getUpdates` |
| Text | `{{ $json.texto }}` (com o botão de expressão ligado) |

**Save** e **Active**.

**Funcionou?** Abra a tampa no Wokwi:

```
NexoLog | NexoLogEquipe01
Tampa aberta
Temperatura: 24.0 °C
Umidade: 40.0 %
Distância: 40.0 cm
Movimentação: 0.03 m/s²
2026-09-11T13:20:05.412Z
```

- [ ] Chega uma mensagem ao abrir, outra ao fechar
- [ ] Nenhum campo com "sem leitura" ou `undefined`

| Deu errado | Onde olhar |
|---|---|
| `undefined` nos campos | faltou o `JSON.parse` do (a) |
| `Bad Request: chat not found` | Chat ID errado, ou você nunca falou com o bot primeiro |
| Manda o texto `{{ $json.texto }}` literal | o campo Text está em modo fixo, não expressão |
| Uma enxurrada de mensagens | o nó "Somente mudança de estado" do Node-RED ficou de fora |

O fluxo completo está em [fluxo_mqtt.json](fluxo_mqtt.json) — Import from File, para comparar com o seu.

Uma equipe, um workflow ativo. Dois workflows no mesmo tópico mandam a mensagem duas vezes.
