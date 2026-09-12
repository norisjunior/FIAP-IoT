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
tampa ao mesmo tempo, saem duas mensagens — cada uma com o número que a disparou.

**a) Nó Code**, depois do trigger. Deixe o Mode em `Run Once for All Items`: este nó
recebe um evento e pode devolver vários itens.

```javascript
const num = (v, casas) => Number.isFinite(v) ? v.toFixed(casas) : "sem leitura";

const mapa = {
  "Temperatura alta":    {sensor: "temperatura",  linha: (d) => `Temperatura: ${num(d.temp, 1)} °C (limite 30)`},
  "Umidade alta":        {sensor: "umidade",      linha: (d) => `Umidade: ${num(d.umid, 1)} % (limite 70)`},
  "Tampa aberta":        {sensor: "distancia",    linha: (d) => `Distância: ${num(d.dist, 1)} cm (limite 25)`},
  "Movimentação brusca": {sensor: "movimentacao", linha: (d) => `Movimentação: ${num(d.movimentacao, 2)} m/s² (limite 3)`},
};

const saida = [];

for (const item of $input.all()) {
  const d = typeof item.json.message === "string" ? JSON.parse(item.json.message) : item.json.message;
  const motivos = d.motivos ?? [];

  // Sem motivo: a entrega voltou ao normal. Uma mensagem só.
  if (motivos.length === 0) {
    saida.push({json: {...d, sensor: "normal",
      texto: `NexoLog | ${d.device}\n${d.estado}\n${d.timestamp}`}});
    continue;
  }

  // Um item por limiar ultrapassado: dois limiares, duas mensagens.
  for (const motivo of motivos) {
    const info = mapa[motivo] ?? {sensor: "outro", linha: () => motivo};
    saida.push({json: {...d, sensor: info.sensor, motivo,
      texto: `NexoLog | ${d.device}\n${motivo}\n${info.linha(d)}\n${d.timestamp}`}});
  }
}

return saida;
```

`message` chega como texto — sem o `JSON.parse` os campos vêm `undefined`. E é o
campo `motivos`, a lista que o Node-RED publica, que permite separar; o `estado` é
só o texto grudado.

Execute o nó com a tampa aberta: tem que sair **um** item, com `sensor: "distancia"`.

**b) Nó Switch**, chamado `Qual limiar?`. Routing Rules sobre `{{ $json.sensor }}`,
`is equal to`, uma saída por valor:

| Saída | Valor | Rename output |
|---|---|---|
| 1 | `temperatura` | Temperatura |
| 2 | `umidade` | Umidade |
| 3 | `distancia` | Distância |
| 4 | `movimentacao` | Movimentação |

Em **Options**, acrescente `Fallback Output` = `Extra Output`. É a saída 5, por onde
sai o "voltou ao normal".

**c) Cinco nós Telegram**, um por saída, action `Send a Text Message`. Todos com o
mesmo conteúdo — o texto já vem pronto do Code:

| Campo | Valor |
|---|---|
| Credential | token do @BotFather |
| Chat ID | o seu — mande um "oi" para o bot e pegue em `api.telegram.org/bot<TOKEN>/getUpdates` |
| Text | `{{ $json.texto }}` (com o botão de expressão ligado) |

Nomeie cada um pelo limiar: `Avisar: temperatura`, `Avisar: umidade`, `Avisar: tampa`,
`Avisar: movimentação`, `Avisar: normalizado`.

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
- [ ] Nenhum campo com "sem leitura" ou `undefined`

| Deu errado | Onde olhar |
|---|---|
| `undefined` nos campos | faltou o `JSON.parse` do (a) |
| Tudo cai na saída 5 | `motivos` não está no payload: veja a função "Estado da entrega" no Node-RED |
| Uma mensagem só, com tudo junto | o Code está devolvendo um item por evento em vez de um por motivo |
| `Bad Request: chat not found` | Chat ID errado, ou você nunca falou com o bot primeiro |
| Manda o texto `{{ $json.texto }}` literal | o campo Text está em modo fixo, não expressão |
| Uma enxurrada de mensagens | o nó "Somente mudança de estado" do Node-RED ficou de fora |

O fluxo completo está em [fluxo_mqtt.json](fluxo_mqtt.json) — Import from File, para comparar com o seu.

Uma equipe, um workflow ativo. Dois workflows no mesmo tópico mandam a mensagem duas vezes.
