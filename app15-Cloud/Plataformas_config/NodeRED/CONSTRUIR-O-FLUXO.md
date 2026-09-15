# Construir o fluxo do app15 no Node-RED, do zero

Duas iterações. Cada uma roda.

1. A plataforma decide e acende o LED do ESP32.
2. O mesmo LED espelhado no dashboard.

No app14 o Node-RED só mostra. Aqui ele **decide e responde**: lê as medições, combina
duas delas e manda um comando de volta.

Comece do fluxo do app14 já funcionando —
[CONSTRUIR-O-FLUXO.md do app14](../../../app14_CPS_e_Automation/Plataformas_config/NodeRED/CONSTRUIR-O-FLUXO.md).
Firmware do app15 gravado, assinando `cmd`:
[CONSTRUIR-O-FIRMWARE.md](../../app15_NexoLog_PUB_SUB/CONSTRUIR-O-FIRMWARE.md).

> Desative o fluxo do app14 antes de ativar este. Os dois assinam o mesmo tópico e
> montariam dois dashboards concorrentes.

---

## Iteração 1 — Decidir e mandar o comando

O alerta só vale com a tampa aberta **e** a caixa sacudindo. Tampa aberta sozinha não é
alerta: uma entrega parada, sendo conferida, tem a tampa aberta.

**a) Dois nós switch, em série.** É assim que se faz um **E** sem escrever código: o
segundo só recebe quem passou pelo primeiro.

Ligue o **primeiro no nó `json`**, não no `separarDadosSensores` — a decisão precisa das
duas medidas na mesma mensagem, e o `separar` já as mandou para widgets diferentes.

| Nó | Property | Regra 1 | Regra 2 |
|---|---|---|---|
| `Tampa aberta?` | `msg.payload.dist` | `>` `25` | `otherwise` |
| `Movimento brusco?` | `msg.payload.movimentacao` | `>` `3` | `otherwise` |

Em cada switch, o `+ add` cria a segunda regra; a segunda saída aparece sozinha.

**b) Dois nós change**, montando o comando. Set `msg.payload` **to** `{}` JSON:

| Nó | Valor |
|---|---|
| `Alerta ON` | `{"alerta": "ON"}` |
| `Alerta OFF` | `{"alerta": "OFF"}` |

No campo do valor, troque o tipo de `string` para **JSON** (`{}` no seletor). Se ficar
em string, o ESP32 recebe aspas dentro de aspas e o `deserializeJson` recusa.

**c) Um nó mqtt out**, `LED da caixa`, tópico `FIAPIoT/nexolog/equipe01/cmd`. Mesmo
broker do `mqtt in`, já configurado no app14.

**As ligações:**

```
json ──▶ Tampa aberta? ──1──▶ Movimento brusco? ──1──▶ Alerta ON ──┐
                        │                          │               ├──▶ LED da caixa
                        └──2──────────────────────┴──2──▶ Alerta OFF ──┘
```

A saída 2 dos **dois** switches vai para o `Alerta OFF`. Fechou a tampa: OFF. Parou de
sacudir: OFF.

**d) Um nó debug** em `Alerta ON`, chamado `Alerta disparado`. É o que você vai mostrar
em aula: a mensagem exata que sai para o dispositivo.

**Deploy.**

**Funcionou?** No Wokwi:

- [ ] Caixa fechada e parada: LED apagado, nada no Debug
- [ ] Distância de 10 para 40 cm, sem mexer no MPU: **LED continua apagado**
- [ ] Arraste o MPU com a tampa aberta: **o LED acende**
- [ ] Debug mostra `{"alerta":"ON"}`; no Serial do ESP32, `[MQTT] Recebido`
- [ ] Feche a tampa: apaga

| Deu errado | Onde olhar |
|---|---|
| Acende só com a tampa aberta | os switches estão em paralelo, os dois no `json`. O segundo tem que vir da saída 1 do primeiro |
| Nunca acende | o switch está lendo `payload` em vez de `payload.dist` |
| `[CMD] JSON invalido` no ESP32 | o Change está mandando string: troque o tipo para JSON |
| Nada chega no ESP32 | tópico: o firmware assina `cmd`, o `mqtt out` publica em `cmd` |
| Acende e apaga sem parar | normal: a plataforma reavalia a cada leitura, 1 por segundo |

---

## Iteração 2 — O LED no dashboard

O LED está na bancada. Quem olha a tela não vê. Um widget espelha o estado.

**a) Um nó change**, `Só o alerta`. Set `msg.payload` **to** `msg` `payload.alerta`.

O `ui_led` lê um texto em `msg.payload`, e o comando é um objeto. Este nó tira só o
`"ON"` de dentro dele.

**b) Um nó ui_led**, `Espelho do LED`:

| Campo | Valor |
|---|---|
| Group | Caixa equipe01 |
| Size | 4×3 |
| Label | `LED` |
| Color for value | `OFF` → cinza `#5b5b5b` · `ON` → vermelho `#ff0000` |
| Value type | `str` nos dois |

> `ui_led` é o `node-red-contrib-ui-led`. Menu ≡ > Manage palette > Install.

**c) Ligue os dois nós de comando nele:** `Alerta ON` → `Só o alerta` e
`Alerta OFF` → `Só o alerta` → `Espelho do LED`. Sem o OFF ligado, o widget acende e
nunca mais apaga.

**d) Centralize o widget.** Como ele tem 4 de largura num grupo de 12, sozinho na linha
ele encosta à esquerda. Um **ui_spacer** de 4×1 antes dele empurra para o meio; os
demais, depois, seguram o `Conexão` embaixo.

**Deploy.** `http://localhost:1880/ui/`

**Funcionou?**

- [ ] Abra a tampa e sacuda: o widget fica vermelho junto com o LED do Wokwi
- [ ] Feche: os dois apagam
- [ ] Desligue o ESP32 e dispare o alerta: **o widget acende mesmo assim**

O último item não é defeito, é o que o widget significa. Ele espelha o comando que a
plataforma **mandou**, não o que o dispositivo **fez**. Para dizer a verdade, faltaria
o ESP32 publicar de volta o que executou — veja [o README](../../README.md).

| Deu errado | Onde olhar |
|---|---|
| Widget sempre cinza | o `Só o alerta` tem que ser tipo **msg**, não string com o texto `payload.alerta` |
| Acende e não apaga | faltou ligar o `Alerta OFF` no `Só o alerta` |
| Widget encostado na esquerda | o `ui_spacer` de antes dele, ou a ordem dos widgets |
| Nó não aparece na paleta | falta instalar o `node-red-contrib-ui-led` |

---

O fluxo completo está em
[Fluxo_1_dashboard_graphs_e_cmd.json](Fluxo_1_dashboard_graphs_e_cmd.json) — ≡ > Import,
para comparar com o seu.

Falta o histórico: [Fluxo_2_envio_InfluxDB.json](Fluxo_2_envio_InfluxDB.json) e os
[dashboards do Grafana](../Grafana/README.md).
