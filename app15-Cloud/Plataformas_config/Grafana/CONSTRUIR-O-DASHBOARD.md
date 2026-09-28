# Construir os dashboards no Grafana, do zero

Três iterações no dashboard de medições e uma no canvas. Cada uma roda.

1. Os indicadores: quatro gauges e dois números de saúde.
2. As séries: o histórico, com a linha mudando de cor na faixa de alerta.
3. A tabela: as últimas leituras, cruas, como no Data Explorer.
4. **Outro dashboard**, o canvas: as medições em cima da foto da bag.

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
| Name | `influxdb-sql` |
| Query language | **SQL** |
| URL | `https://<sua-regiao>.aws.cloud2.influxdata.com` |
| Database | o nome do seu bucket |
| Token | o mesmo do Node-RED |
| Insecure Connection | desligado |

**Save & test** tem que responder *datasource is working*.

> **A linguagem é da conexão, não do painel.** Uma conexão em Flux só fala Flux. Para
> ter as duas, crie duas conexões.

### Como o SQL enxerga o dado

No SQL, a measurement é uma **tabela** e cada campo é uma **coluna**. O Node-RED grava
tudo num ponto só, uma linha por segundo:

| time | device | temp | umid | dist | movimentacao | accel_x | accel_y | accel_z |
|---|---|---|---|---|---|---|---|---|
| 14:02:01 | NexoLogEquipe01 | 24 | 55 | 12 | 0.4 | 0.1 | 9.8 | 0.2 |

Toda consulta do dashboard sai de um modelo só:

```sql
SELECT time, <COLUNAS>
FROM "nexolog"
WHERE $__timeFilter(time)
ORDER BY time
```

- **`time` no SELECT** — é o eixo X. Sem ele, o gráfico não tem onde desenhar.
- **`$__timeFilter(time)`** — o Grafana troca pelo intervalo escolhido no topo da tela.
- **`"nexolog"` entre aspas duplas** — é o nome da tabela. Se o seu Node-RED grava em
  outra measurement, troque aqui.

Teste antes no InfluxDB, **Data Explorer**, com `now() - interval '1 hour'` no lugar do
`$__timeFilter(time)` — o `$__` só existe dentro do Grafana.

---

## Iteração 1 — Os indicadores

New dashboard > Add visualization > escolha `influxdb-sql`. O editor abre no modo
**Builder**: mude para **Code**.

> **Cuidado com o "Expression".** Se aparecer um campo **Operation** com a opção SQL,
> você está numa *SQL Expression* do Grafana, não numa consulta ao InfluxDB. Ela só
> enxerga o resultado de outras consultas do painel. Apague e escolha a fonte de dados
> de novo.

**a) O primeiro gauge.** Cole:

```sql
SELECT time, temp
FROM "nexolog"
WHERE time >= now() - interval '5 minutes'
ORDER BY time DESC
LIMIT 1
```

Aqui o modelo se inverte: `DESC` + `LIMIT 1` pega **só a linha mais recente**. E o
filtro é fixo em 5 minutos, não o do topo — um indicador mostra o agora.

Visualização **Gauge**. Na barra da direita:

| Seção | Valor |
|---|---|
| Standard options > Unit | Celsius (°C) |
| Standard options > Min / Max | `0` / `50` |
| Thresholds | Base verde · `25` amarelo · `30` vermelho |

**b) Os outros três**, iguais, trocando a coluna e os ajustes:

| Painel | Coluna | Tipo | Unit | Min–Max | Thresholds |
|---|---|---|---|---|---|
| Temperatura | `temp` | Gauge | celsius | 0–50 | 25 · 30 |
| Umidade | `umid` | Gauge | humidity (%) | 0–100 | 60 · 70 |
| Tampa | `dist` | **Bar gauge** | centimeters | 0–50 | 20 · 25 |
| Movimentação | `movimentacao` | Gauge | acceleration m/s² | 0–20 | 1.5 · 3 |

No **Bar gauge**, ponha Orientation **Vertical** e Display mode **Gradient**: fica com
cara de nível de tanque, igual ao widget da tampa no Node-RED.

**c) O ESP32 está vivo?** Um painel **Stat**:

```sql
SELECT CAST(max(time) AS BIGINT) / 1000000 AS ultima
FROM "nexolog"
WHERE time >= now() - interval '1 day'
```

Unit: **From Now** (em Date & time). O painel mostra "há 2 segundos". O `max(time)` é o
instante do último ponto; o `CAST` e a divisão o transformam em milissegundos, que é o
que a unidade espera.

**d) Quanto dado tem na tela?** Outro **Stat**:

```sql
SELECT count(*) AS leituras
FROM "nexolog"
WHERE $__timeFilter(time)
```

Mude o intervalo do topo de 15 minutos para 1 hora: o número sobe perto de 3.600, um
ponto por segundo. É o mesmo total de linhas que o Data Explorer mostra.

**Save dashboard.**

**Funcionou?**

- [ ] Os quatro indicadores com número, mudando junto com o Wokwi
- [ ] Abra a tampa: a barra fica vermelha
- [ ] Pare a simulação: a "Última leitura" começa a envelhecer — "há 30 segundos", "há 1 minuto"
- [ ] Mude o intervalo do topo: a contagem muda, os gauges não

O último item é a diferença entre as duas consultas: `now() - interval '5 minutes'`
ignora o seletor de tempo, `$__timeFilter(time)` obedece.

| Deu errado | Onde olhar |
|---|---|
| `No data` em tudo | confira antes no InfluxDB > Data Explorer. Se não tem lá, o problema é o Node-RED |
| `No data` só nos gauges | eles olham 5 minutos: o ESP32 parou de publicar. A "Última leitura" confirma |
| `table 'nexolog' not found` | o nome entre aspas duplas não é o da sua measurement, ou o Database da conexão não é o seu bucket |
| O editor tem "Operation" | é Expression, não consulta. Veja o aviso no começo da iteração |
| Última leitura mostra um número enorme | falta a Unit **From Now** |

---

## Iteração 2 — As séries

Mesmo caminho, agora com o modelo na forma original:

```sql
SELECT time, temp, umid
FROM "nexolog"
WHERE $__timeFilter(time)
ORDER BY time
```

Duas colunas no SELECT, duas linhas no gráfico. Visualização **Time series**.

**a) Temperatura e umidade, cada uma no seu eixo.** Graus e porcentagem na mesma escala
achatam uma das duas. Em **Overrides**:

`+ Add field override` > `Fields with name` > `umid` > `+ Add override property`:

| Propriedade | Valor |
|---|---|
| Axis > Placement | Right |
| Standard options > Unit | humidity (%) |
| Color scheme | Single color, roxo |

Repita para `temp`: Unit Celsius, cor laranja. Agora cada uma tem sua régua.

**b) Distância e Movimentação, com a faixa de alerta.** Um painel para cada, com
`SELECT time, dist` e `SELECT time, movimentacao`. Os mesmos limiares dos gauges, e mais
três ajustes:

| Seção | Valor |
|---|---|
| Graph styles > Gradient mode | **Scheme** |
| Standard options > Color scheme | **From thresholds (by value)** |
| Thresholds > Show thresholds | **As lines (dashed)** |

A linha **muda de cor** quando cruza o limiar. Abra a tampa: o trecho sobe e fica
vermelho, e a linha tracejada mostra onde o alerta começa.

**c) O acelerômetro.** `SELECT time, accel_x, accel_y, accel_z`, Unit acceleration
m/s². Parada, a bag mostra a gravidade: um dos eixos fica perto de 9,8. Sacuda o MPU no
Wokwi e os três se mexem.

A **movimentação** é o resumo dessas três: o pico, a cada segundo, do quanto a
aceleração fugiu do repouso.

**Save dashboard.**

**Funcionou?**

- [ ] Umidade com a régua do lado direito
- [ ] Abra a tampa: a série da distância fica vermelha naquele trecho
- [ ] Mude o intervalo do topo para 6 horas: as séries mostram a entrega inteira
- [ ] Os gauges continuam no valor de agora

| Deu errado | Onde olhar |
|---|---|
| Gráfico vazio, mas a tabela (Table view) tem dado | faltou `time` no SELECT |
| As duas séries na mesma régua | o override é por nome: `umid` exatamente como vem da consulta |
| A linha não muda de cor | Color scheme tem que ser **From thresholds**, e Gradient mode **Scheme** |

> **Quando ficar lento.** Um ponto por segundo são 86 mil por dia. Para olhar semanas,
> agrupe por janela — o Grafana escolhe o tamanho conforme o zoom:
>
> ```sql
> SELECT $__dateBin(time) AS time, avg(temp) AS temp
> FROM "nexolog"
> WHERE $__timeFilter(time)
> GROUP BY 1
> ORDER BY 1
> ```
>
> Na movimentação, use `max` em vez de `avg`: é um pico, e a média esconde o tranco.

---

## Iteração 3 — A tabela

O dado cru, sem gráfico. Visualização **Table**:

```sql
SELECT time, device, temp, umid, dist, movimentacao
FROM "nexolog"
WHERE $__timeFilter(time)
ORDER BY time DESC
LIMIT 20
```

`DESC` põe a mais nova em cima; `LIMIT 20` corta o resto.

Para colorir o número da coluna, em **Overrides**, um por coluna:

`Fields with name` > `temp` > propriedades:

| Propriedade | Valor |
|---|---|
| Standard options > Unit | Celsius (°C) |
| Thresholds | 25 amarelo · 30 vermelho |
| Standard options > Color scheme | From thresholds (by value) |
| Cell options > Cell type | **Colored text** |

Repita para `umid`, `dist` e `movimentacao`, com os limiares da iteração 1.

**Save dashboard.**

**Funcionou?**

- [ ] Vinte linhas, a mais recente em cima, entrando uma por segundo
- [ ] Abra a tampa: o número da coluna `dist` fica vermelho
- [ ] A coluna `device` mostra o nome do seu ESP32

É a mesma consulta que dá para rodar no Data Explorer. A diferença é que aqui ela
atualiza sozinha e colore o que passou do limite.

---

## Iteração 4 — O canvas (outro dashboard)

Um dashboard **à parte**: a foto da bag vira o painel, e as medições ficam em cima dela.
New dashboard > Add visualization > `influxdb-sql`.

**a) Painel novo, visualização Canvas.** A consulta é **uma só** para os quatro campos:

```sql
SELECT time, temp, umid, dist, movimentacao
FROM "nexolog"
WHERE time >= now() - interval '5 minutes'
ORDER BY time DESC
LIMIT 1
```

É a consulta do gauge com quatro colunas em vez de uma. Uma linha só, com os quatro
valores lado a lado — que é o que os elementos do canvas procuram pelo nome.

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

> É o mesmo override da tabela, na iteração 3: quando um painel carrega mais de um
> campo, cada um ganha os seus limiares. Nos gauges, com um campo só, a seção de cima
> bastava.

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
| Só um campo aparece, os outros vazios | a consulta não traz as quatro colunas no SELECT |
| Nada muda de cor | Color mode em Standard options tem que ser **From thresholds** |
| A cor muda na caixa mas não na seta | a seta tem o próprio seletor de cor: ponha em **Field** |
| A seta não nasce | arraste **do ponto na borda** da elipse, não do meio dela |
| Fundo escuro, sem foto | a URL da imagem, ou o elemento selecionado era um item e não o frame |

---

Os dois prontos estão em [dashboard_nexolog.json](dashboard_nexolog.json) e
[dashboard_nexolog_canvas.json](dashboard_nexolog_canvas.json) — Import, para comparar
com os seus.

> **Antes de importar os prontos:** na importação, escolha `influxdb-sql` como fonte de
> dados. As consultas usam a variável **Measurement**, na caixa do topo, em vez de
> `"nexolog"` fixo — se a sua measurement tem outro nome, é lá que se troca.
