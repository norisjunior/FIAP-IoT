# Construir o fluxo no Node-RED, do zero

Duas iterações. Cada uma roda.

1. O dado chegando no Debug.
2. O dado virando dashboard, estado e evento.

Comece com uma aba vazia. Firmware publicando: [CONSTRUIR-O-FIRMWARE.md](../../app14_NexoLog/CONSTRUIR-O-FIRMWARE.md).

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

**Funcionou?** Na aba Debug, a cada 2,5 s:

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

## Iteração 2 — Dashboard, estado e evento

Continue do `json`.

**a) Separar os valores.** Nó **function**, 4 saídas (aba Setup > Outputs):

```javascript
const p = msg.payload;
let temp = {payload: p.temp, topic: "Temperatura"};
let umid = {payload: p.umid};
let dist = {payload: p.dist};
let movimentacao = {payload: p.movimentacao};
return [temp, umid, dist, movimentacao];
```

**b) Um widget por saída.** Crie o grupo uma vez, no primeiro widget (Group > Add > 12 de largura). Os quatro entram na mesma linha:

| Saída | Nó | Order | Size | Ajustes |
|---|---|---|---|---|
| 1 | ui_gauge | 1 | 3×3 | label `°C`, 0–50 |
| 2 | ui_gauge | 2 | 3×3 | label `%`, 0–100 |
| 3 | ui_level | 3 | 3×3 | label `Tampa`, unit `cm`, 0–50, warn 20, high 25, layout vertical |
| 4 | ui_gauge | 4 | 3×3 | label `m/s²`, 0–20, seg 1.5 e 3 |

Ligue a saída 1 também num **ui_chart** (order 5, 12×4).

> `ui_level` é o `node-red-contrib-ui-level`. Menu ≡ > Manage palette > Install.

**c) Decidir o estado.** Nó **function**, ligado ao `json`:

```javascript
const p = msg.payload;
const LIMIAR_TEMP = 30;
const LIMIAR_UMID = 70;
const LIMIAR_DIST = 25;
const LIMIAR_MOVIMENTACAO = 3;   // m/s2. Limite depende do contexto:
// caixa em prateleira aceita pouco; bag de entregador em moto passa disso na rua ruim.

let motivos = [];
if (![p.temp, p.umid, p.dist, p.movimentacao].every(Number.isFinite)) motivos.push("Falha de sensor");
if (Number.isFinite(p.temp) && p.temp > LIMIAR_TEMP) motivos.push("Temperatura alta");
if (Number.isFinite(p.umid) && p.umid > LIMIAR_UMID) motivos.push("Umidade alta");
if (Number.isFinite(p.dist) && p.dist > LIMIAR_DIST) motivos.push("Tampa aberta");
if (Number.isFinite(p.movimentacao) && p.movimentacao > LIMIAR_MOVIMENTACAO) motivos.push("Movimentação brusca");
msg.payload = {...p, alerta: motivos.length > 0, estado: motivos.join(" / ") || "Entrega em condição normal"};
return msg;
```

Ligue num **ui_text** (order 6, 12×1, label `Entrega`, Value format `{{msg.payload.estado}}`).

**d) Avisar só na mudança.** Nó **function**, depois do estado. Sem isso o n8n recebe 24 mensagens por minuto:

```javascript
const p = msg.payload;
const chave = p.device;
const anterior = context.get(chave);
context.set(chave, p.estado);
if (anterior === p.estado) return null;
if (anterior === undefined && !p.alerta) return null;
msg.payload = {...p, timestamp: new Date().toISOString()};
return msg;
```

Ligue em **json** (`Object to JSON`) → **mqtt out**, tópico `FIAPIoT/nexolog/equipe01/eventos`.

**Deploy.**

**Funcionou?** `http://localhost:1880/ui/`

- [ ] Quatro widgets numa linha só, com valor
- [ ] Texto do estado: "Entrega em condição normal"
- [ ] Sobe a distância no Wokwi de 10 para 40 cm → barra vermelha e "Tampa aberta"
- [ ] Volta para 10 cm → normal. Só duas mensagens saíram no tópico `eventos`

| Deu errado | Onde olhar |
|---|---|
| Widgets empilhados | somam mais de 12 de largura, ou estão em grupos diferentes |
| Gauge vazio | a saída da function não bate com o nó, ou o campo mudou de nome no firmware |
| Sempre "Falha de sensor" | o firmware está mandando `null` — volte para a iteração 1 do firmware |
| Evento repetindo sem parar | o nó (d) ficou de fora |

Pronto: [o histórico no InfluxDB](Fluxo_2_envio_InfluxDB.json) e [o aviso no n8n](../n8n/CONSTRUIR-O-FLUXO.md).

O fluxo completo está em [dashboard.json](dashboard.json) — ≡ > Import, para comparar com o seu.
