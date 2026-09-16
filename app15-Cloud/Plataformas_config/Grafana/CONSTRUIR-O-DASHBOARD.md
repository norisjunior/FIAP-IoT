# Construir os dashboards no Grafana, do zero

Duas iterações. Cada uma roda.

1. O dashboard de sempre: gauges, nível e séries.
2. O canvas: as medições em cima da foto da bag, com setas que mudam de cor.

O Node-RED e o n8n olham o dado passando. O Grafana olha o dado **guardado** — então o
[Fluxo_2](../NodeRED/Fluxo_2_envio_InfluxDB.json) precisa estar gravando antes de começar.

> **A ordem importa em um ponto só:** a consulta vem primeiro. Os nomes `temp`, `dist` e
> companhia só existem depois que ela roda — antes disso não há o que escolher em
> nenhum seletor. Todo o resto é em qualquer ordem.

---

## Iteração 0 — A fonte de dados

Uma vez só, e os dois dashboards usam. Connections > Add new connection > **InfluxDB**.

| Campo | Valor |
|---|---|
| Query language | **Flux** |
| URL | `https://<sua-regiao>.aws.cloud2.influxdata.com` |
| Auth | tudo desligado |
| Organization | a sua |
| Token | o mesmo do Node-RED |
| Default Bucket | o seu |

**Save & test** tem que responder *datasource is working*.

---

## Iteração 1 — Gauges, nível e séries

New dashboard > Add visualization > escolha a fonte de dados.

**a) O primeiro painel.** Cole a consulta, em modo **Script Editor** (não o construtor
visual):

```flux
from(bucket: "SEU_BUCKET")
  |> range(start: -5m)
  |> filter(fn: (r) => r["_measurement"] == "nexolog")
  |> filter(fn: (r) => r["_field"] == "temp")
  |> last()
```

Visualização **Gauge**. Na barra da direita:

| Seção | Valor |
|---|---|
| Standard options > Unit | Celsius (°C) |
| Standard options > Min / Max | `0` / `50` |
| Thresholds | Base verde · `25` amarelo · `30` vermelho |

`last()` devolve só o ponto mais recente — é o que um indicador precisa.

**b) Os outros três**, iguais, trocando o campo e os ajustes:

| Painel | Campo | Tipo | Unit | Min–Max | Thresholds |
|---|---|---|---|---|---|
| Temperatura | `temp` | Gauge | celsius | 0–50 | 25 · 30 |
| Umidade | `umid` | Gauge | humidity (%) | 0–100 | 60 · 70 |
| Tampa | `dist` | **Bar gauge** | centimeters | 0–50 | 20 · 25 |
| Movimentação | `movimentacao` | Gauge | acceleration m/s² | 0–20 | 1.5 · 3 |

No **Bar gauge**, ponha Orientation **Vertical** e Display mode **Gradient**: fica com
cara de nível de tanque, igual ao widget da tampa no Node-RED.

**c) As séries.** Mesma coisa, com outra consulta — esta pega o intervalo da tela, não
os últimos 5 minutos:

```flux
from(bucket: "SEU_BUCKET")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r["_measurement"] == "nexolog")
  |> filter(fn: (r) => r["_field"] == "temp" or r["_field"] == "umid")
  |> aggregateWindow(every: v.windowPeriod, fn: mean, createEmpty: false)
```

Visualização **Time series**. Três painéis: `temp` + `umid` juntos, `movimentacao`
sozinho, `dist` sozinho.

`v.timeRangeStart` e `v.windowPeriod` vêm do seletor de tempo lá em cima. O
`aggregateWindow` agrupa conforme o zoom — é o que faz um mês de dados abrir rápido em
vez de mandar milhões de pontos para o navegador.

**Save dashboard.**

**Funcionou?**

- [ ] Os quatro indicadores com número, mudando junto com o Wokwi
- [ ] Abra a tampa: a barra fica vermelha
- [ ] Mude o intervalo do topo para 6 horas: as séries mostram a entrega inteira
- [ ] O gauge continua no valor de agora — ele não olha o intervalo

O último item é a diferença entre os dois tipos: `last()` ignora o seletor de tempo,
`range(v.timeRangeStart)` obedece.

| Deu errado | Onde olhar |
|---|---|
| `No data` em tudo | confira antes no InfluxDB > Data Explorer. Se não tem lá, o problema é o Node-RED |
| `No data` só nos gauges | o `last()` olha 5 minutos: o ESP32 parou de publicar |
| Erro de bucket | o nome entre aspas na consulta é o **seu** bucket |
| A série sobe em degraus | normal: `aggregateWindow` tira a média por janela |

---

## Iteração 2 — O canvas

Aqui a foto da bag vira o painel, e as medições ficam em cima dela.

**a) Painel novo, visualização Canvas.** A consulta é **uma só** para os quatro campos,
e termina em `pivot`:

```flux
from(bucket: "SEU_BUCKET")
  |> range(start: -5m)
  |> filter(fn: (r) => r["_measurement"] == "nexolog")
  |> filter(fn: (r) => r["_field"] == "temp" or r["_field"] == "umid"
                    or r["_field"] == "dist" or r["_field"] == "movimentacao")
  |> last()
  |> pivot(rowKey: ["_time"], columnKey: ["_field"], valueColumn: "_value")
```

**O `pivot` é obrigatório.** Sem ele o Flux devolve quatro linhas, uma por campo, e
cada elemento do canvas enxerga só uma. Com ele, os quatro viram colunas de uma linha
só — que é o que os seletores procuram pelo nome.

**b) A foto de fundo.** Selecione o quadro de fora (Element 1, o frame) e em
**Background > Image** cole a URL:

```
https://raw.githubusercontent.com/norisjunior/FIAP-IoT/refs/heads/fiapiot/eval/app15-Cloud/Plataformas_config/Grafana/SmartDeliveryBag.png
```

Size: **contain**. Para usar a sua própria foto, suba num lugar que o navegador
alcance e troque a URL.

**c) Os limiares, uma vez por campo.** Este painel tem quatro campos, então não dá para
usar a seção Thresholds de cima — ela vale para todos. Vá em **Overrides**, no fim da
barra:

`+ Add field override` > `Fields with name` > `dist` > `+ Add override property` >
`Thresholds`.

| Campo | Base | Amarelo | Vermelho |
|---|---|---|---|
| `temp` | verde | 25 | 30 |
| `umid` | verde | 60 | 70 |
| `dist` | verde | 20 | 25 |
| `movimentacao` | verde | 1.5 | 3 |

> É a mesma ferramenta da iteração 1. Lá cada painel tem um campo só, e os limiares vão
> direto na seção de cima; o override só aparece quando um painel carrega mais de um.

**d) As caixas de medição.** Add item > **Metric value**. Em cada uma:

| Campo | Valor |
|---|---|
| Text > Source | **Field** → escolha `dist` |
| Text > Size | 30 |
| Background color | fixo escuro, com transparência |
| Border color | **Field** → o mesmo campo |
| Border width | 3 |

A borda em Field é o que faz a caixa ficar vermelha sozinha. Um elemento **Text** por
cima, com o rótulo fixo, e a caixa está pronta. Arraste para o lugar.

**e) Os pontos e as setas.** Add item > **Ellipse**, pequena, sobre a tampa da bag.
Background color em **Field** → `dist`.

Para puxar a seta: passe o mouse na borda da elipse, aparece um ponto de ancoragem;
arraste dele até a caixa de destino. Clique na seta criada:

| Campo | Valor |
|---|---|
| Path | Straight |
| Size | 3 ou 4 |
| Color | **Field** → `dist` |

Repita com uma segunda elipse na base da bag, ligada à Movimentação. A posição conta a
história: os dois sensores estão na tampa na vida real, mas separar a origem deixa
óbvio que são duas medidas diferentes.

Temperatura e umidade não levam seta — elas não são da bag, são do ambiente. Um
**Icon** ao lado de cada uma basta.

**Save dashboard.**

**Funcionou?**

- [ ] Os quatro valores aparecem sobre a foto
- [ ] Abra a tampa no Wokwi: a caixa da Distância **e a seta dela** ficam vermelhas
- [ ] Sacuda o MPU: a outra seta acompanha
- [ ] Arraste uma caixa: a seta segue sozinha

| Deu errado | Onde olhar |
|---|---|
| Elemento mostra o rótulo e não o número | Text > Source está em **Fixed**, tem que ser **Field** |
| Só um campo aparece, os outros vazios | faltou o `pivot` na consulta |
| Nada muda de cor | Color mode em Standard options tem que ser **From thresholds** |
| A cor muda na caixa mas não na seta | a seta tem o próprio seletor de cor: ponha em **Field** |
| A seta não nasce | arraste **do ponto na borda** da elipse, não do meio dela |
| Fundo escuro, sem foto | a URL da imagem, ou o elemento selecionado era um item e não o frame |

---

Os dois prontos estão em [dashboard_nexolog.json](dashboard_nexolog.json) e
[dashboard_nexolog_canvas.json](dashboard_nexolog_canvas.json) — Import, para comparar
com os seus.

> **Antes de importar os prontos:** eles foram exportados de uma instalação específica,
> então a fonte de dados e o nome do bucket vêm preenchidos com os de lá. Depois de
> importar, abra cada consulta e troque o bucket pelo seu.
