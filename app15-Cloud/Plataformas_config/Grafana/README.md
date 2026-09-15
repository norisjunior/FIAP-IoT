# Grafana — dois dashboards da NexoLog

Grafana **local**, lendo o **InfluxDB Cloud** — o mesmo banco onde o
[Fluxo_2](../NodeRED/Fluxo_2_envio_InfluxDB.json) do Node-RED grava.

O Node-RED e o n8n olham o dado passando. O Grafana olha o dado guardado.

| Arquivo | O que é |
|---|---|
| [dashboard_nexolog.json](dashboard_nexolog.json) | gauges, nível e séries. O de todo dia |
| [dashboard_nexolog_canvas.json](dashboard_nexolog_canvas.json) | a foto da bag com as medições em cima |

---

## 1. A fonte de dados, uma vez só

Connections > Add new connection > **InfluxDB**.

| Campo | Valor |
|---|---|
| Query language | **Flux** |
| URL | `https://<sua-regiao>.aws.cloud2.influxdata.com` |
| Auth | tudo desligado |
| Organization | a sua |
| Token | o mesmo do Node-RED |
| Default Bucket | o seu |

**Save & test** tem que responder *datasource is working*.

O Grafana roda local, o banco está na nuvem: quem precisa de internet é o Grafana,
não o ESP32.

## 2. Importar

Dashboards > New > **Import** > Upload JSON file.

Na tela de import, o Grafana pergunta qual é a fonte de dados InfluxDB — escolha a que
você acabou de criar. Depois de abrir, no topo tem a caixa **Bucket**: escreva o nome
do seu bucket e dê Enter. Todos os painéis usam essa caixa, então é um lugar só.

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

Os quatro de cima usam `last()`: o valor mais recente. As séries usam
`aggregateWindow`, que agrupa os pontos conforme o zoom — é o que deixa um mês de
dados abrir rápido.

Repare que é o mesmo painel do Node-RED, com uma diferença: aqui você pode voltar no
tempo. Mude o intervalo no topo para 24 horas e veja a entrega inteira.

## 4. O dashboard canvas

**Antes de importar, coloque a imagem no lugar.** O canvas aponta para
`/public/img/bag.png`, que é como o Grafana serve qualquer arquivo da pasta
`public/img/`:

```bash
# Grafana em Docker
docker cp bag.png grafana:/usr/share/grafana/public/img/bag.png
docker restart grafana

# Grafana instalado na máquina (Linux)
sudo cp bag.png /usr/share/grafana/public/img/
```

Use uma foto da sua bag, de frente, fundo limpo. Sem a imagem o painel abre com fundo
escuro e os valores flutuando — funciona, mas perde a graça.

Os oito elementos são quatro pares de rótulo + valor. Para mover qualquer um:
**Edit** no painel, clique no elemento, arraste. É edição no próprio canvas, sem JSON.
Ajuste as posições à sua foto — as que vieram no arquivo são um ponto de partida, não
vão cair certo em cima da sua imagem.

Cores: tampa e movimentação ficam vermelhas sozinhas quando passam de 25 cm e 3 m/s²,
pelos thresholds do painel. Temperatura e umidade ficam brancas — são informação, não
alarme.

A consulta é uma só, com `pivot()` no fim. Sem o pivot, o Flux devolve uma linha por
campo e o canvas não acha `temp` e `dist` na mesma tabela; com ele, os quatro viram
colunas de uma linha só, que é o que cada elemento procura pelo nome.

## Deu errado

| Sintoma | Onde olhar |
|---|---|
| Painel vazio, sem erro | a caixa **Bucket** no topo ainda está com `SEU_BUCKET` |
| `unauthorized` | o token da fonte de dados, ou a organização |
| Gauge com "No data" e a série cheia | o `last()` olha os últimos 5 minutos: o ESP32 parou de publicar |
| Canvas com fundo escuro | a imagem não está em `public/img/`, ou o Grafana não foi reiniciado |
| Canvas mostra o rótulo e não o número | o elemento está em Fixed em vez de Field, ou o nome do campo não bate |
| Tudo "No data" | confira antes no InfluxDB, Data Explorer. Se não tem lá, o problema é o Node-RED |
