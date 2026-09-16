# Grafana — dois dashboards da NexoLog

Grafana **local**, lendo o **InfluxDB Cloud** — o mesmo banco onde o
[Fluxo_2](../NodeRED/Fluxo_2_envio_InfluxDB.json) do Node-RED grava.

O Node-RED e o n8n olham o dado passando. O Grafana olha o dado guardado.

| Arquivo | O que é |
|---|---|
| [dashboard_nexolog.json](dashboard_nexolog.json) | gauges, nível e séries. O de todo dia |
| [dashboard_nexolog_canvas.json](dashboard_nexolog_canvas.json) | a foto da bag com as medições em cima |
| [CONSTRUIR-O-DASHBOARD.md](CONSTRUIR-O-DASHBOARD.md) | montar os dois do zero, em vez de importar |

---

## 1. A fonte de dados, uma vez só

Connections > Add new connection > **InfluxDB**.

| Campo | Valor |
|---|---|
| **Product** | **InfluxDB Cloud Serverless** |
| URL | `https://<sua-regiao>.aws.cloud2.influxdata.com` |
| **Query language** | **SQL** |
| Database | o seu bucket |
| Token | o mesmo do Node-RED |

**Product vem antes de tudo.** As opções de *Query language* dependem dele: SQL só
aparece em produtos InfluxDB 3. Se o Product ficar em `InfluxDB OSS 2.x` ou
`InfluxDB Cloud (TSM)`, o Grafana oferece Flux e InfluxQL, mostra campos de
*Organization* e *Default Bucket*, e não há SQL em lugar nenhum.

`Database` é o nome do **bucket** — mudou o nome do campo, é a mesma coisa.
A URL é a mesma de sempre: o SQL usa Flight (gRPC) no mesmo endereço, não outra porta.

**Save & test** tem que responder *datasource is working*.

O Grafana roda local, o banco está na nuvem: quem precisa de internet é o Grafana,
não o ESP32.

## 2. Importar

Dashboards > New > **Import** > Upload JSON file.

Na tela de import, o Grafana pergunta qual é a fonte de dados InfluxDB — escolha a que
você acabou de criar. Não há caixa de **Bucket** no topo: em SQL o database mora na
fonte de dados, não na consulta.

## 3. O dashboard de medições

Sete painéis, com os mesmos limites do n8n:

| Painel | Tipo | Faixas |
|---|---|---|
| Temperatura | gauge | verde até 25, amarelo até 30, vermelho acima |
| Umidade | gauge | verde até 60, amarelo até 70, vermelho acima |
| Tampa | bar gauge vertical | verde até 20 cm, amarelo até 25, vermelho acima |
| Movimentação | gauge | verde até 1,5, amarelo até 3, vermelho acima |
| Temperatura e umidade | série temporal | — |
| Movimentação | série temporal | — |
| Distância até a tampa | série temporal | — |

Os quatro de cima terminam em `ORDER BY time DESC LIMIT 1`: o valor mais recente. As
séries usam `$__dateBin(time)`, que agrupa os pontos conforme o zoom — é o que deixa um
mês de dados abrir rápido — e `$__timeFilter(time)`, que é o que faz elas obedecerem ao
seletor de tempo do topo.

Repare que é o mesmo painel do Node-RED, com uma diferença: aqui você pode voltar no
tempo. Mude o intervalo no topo para 24 horas e veja a entrega inteira.

## 4. O dashboard canvas

A foto de fundo já vem resolvida: o painel aponta para a
[SmartDeliveryBag.png](SmartDeliveryBag.png) deste repositório, pela URL raw do
GitHub. Nada para copiar, nada para instalar. Para usar a sua própria foto, suba num
lugar que o navegador alcance e troque a URL em **Background > Image**.

Dez elementos sobre a foto:

| Elemento | O que é |
|---|---|
| Temperatura, Umidade | valor + ícone, no céu — são do ambiente, não da bag |
| Distância, Movimentação | rótulo + valor, com **seta** saindo da bag |
| Duas elipses | os pontos de origem: tampa em cima, movimentação na base |

**A seta muda de cor com o valor.** Verde dentro do limite, amarelo na faixa de
atenção, vermelho fora — e a borda da caixa acompanha, porque as duas leem o mesmo
campo. Abra a tampa no Wokwi e a seta fica vermelha antes de você ler o número.

| Campo | Verde | Amarelo | Vermelho |
|---|---|---|---|
| temp | < 25 °C | 25 a 30 | > 30 |
| umid | < 60 % | 60 a 70 | > 70 |
| dist | < 20 cm | 20 a 25 | > 25 |
| movimentacao | < 1,5 m/s² | 1,5 a 3 | > 3 |

Para mover qualquer elemento: **Edit** no painel, clique, arraste — a seta acompanha
sozinha. Depois de ajustar, **exporte de volta** e substitua o arquivo aqui.

A consulta do canvas é uma só, em SQL, com os quatro campos no `SELECT` e
`ORDER BY time DESC LIMIT 1`: uma linha, quatro colunas, que é o que cada elemento
procura pelo nome. (No Flux isso exigia um `pivot()` no fim; em SQL cada field já é
coluna.)

## Deu errado

| Sintoma | Onde olhar |
|---|---|
| Painel vazio, com `Data outside time range` | o intervalo da consulta não é o do seletor de tempo |
| `unauthorized` | o token da fonte de dados |
| Só tem Flux e InfluxQL, sem SQL | o **Product** da fonte de dados não é um InfluxDB 3 |
| Gauge com "No data" e a série cheia | a consulta do gauge olha os últimos 5 minutos: o ESP32 parou de publicar |
| Canvas com fundo escuro | a URL da imagem em **Background > Image**, no frame de fora |
| Canvas mostra o rótulo e não o número | o elemento está em Fixed em vez de Field, ou o nome do campo não bate |
| Tudo "No data" | confira antes no InfluxDB, Data Explorer. Se não tem lá, o problema é o Node-RED |
