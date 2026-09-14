# Construir o fluxo no Node-RED, do zero

Duas iterações. Cada uma roda.

1. O dado chegando no Debug.
2. O dado virando dashboard.

Aqui o Node-RED só **mostra**. Quem decide se algo está errado e avisa é o
[n8n](../n8n/CONSTRUIR-O-FLUXO.md), que assina o mesmo tópico.

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

Pronto: [o histórico no InfluxDB](Fluxo_2_envio_InfluxDB.json) e
[o aviso no n8n](../n8n/CONSTRUIR-O-FLUXO.md).

O fluxo completo está em [dashboard.json](dashboard.json) — ≡ > Import, para comparar
com o seu.
