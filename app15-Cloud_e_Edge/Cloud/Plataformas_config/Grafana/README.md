# Grafana — dois dashboards da NexoLog

Grafana **local**, lendo um destes dois bancos, o mesmo em que o Fluxo_2 do Node-RED grava:

| Banco | Fluxo do Node-RED | Linguagem | Dashboards |
|---|---|---|---|
| **InfluxDB Cloud** | [Fluxo_2_INFLUXDBCLOUD.json](../NodeRED/Fluxo_2_INFLUXDBCLOUD.json) | SQL | `*_INFLUXDBCLOUD.json` |
| **InfluxDB local**, o contêiner da IoT-platform | [Fluxo_2_INFLUXDBLOCAL.json](../NodeRED/Fluxo_2_INFLUXDBLOCAL.json) | Flux | `*_INFLUXDBLOCAL.json` |

Os painéis são os mesmos nos dois; só a consulta muda. O InfluxDB local é a versão 2.7, que
não entende SQL — por isso Flux.

O Node-RED e o n8n olham o dado passando. O Grafana olha o dado guardado.

| Arquivo | O que é |
|---|---|
| [dashboard_nexolog_INFLUXDBCLOUD.json](dashboard_nexolog_INFLUXDBCLOUD.json) | gauges, séries e tabela. O de todo dia — nuvem, SQL |
| [dashboard_nexolog_canvas_INFLUXDBCLOUD.json](dashboard_nexolog_canvas_INFLUXDBCLOUD.json) | a foto da bag com as medições em cima — nuvem, SQL |
| [dashboard_nexolog_INFLUXDBLOCAL.json](dashboard_nexolog_INFLUXDBLOCAL.json) | gauges, séries e tabela — contêiner local, Flux |
| [dashboard_nexolog_canvas_INFLUXDBLOCAL.json](dashboard_nexolog_canvas_INFLUXDBLOCAL.json) | a foto da bag — contêiner local, Flux |
| [CONSTRUIR-O-DASHBOARD.md](CONSTRUIR-O-DASHBOARD.md) | montar os dois do zero, em vez de importar |

---

## 1. A fonte de dados, uma vez só

Connections > Add new connection > **InfluxDB**.

| Campo | Valor |
|---|---|
| Name | `influxdb-sql` |
| Query language | **SQL** |
| URL | `https://<sua-regiao>.aws.cloud2.influxdata.com` |
| Database | o nome do seu bucket |
| Token | o mesmo do Node-RED |
| Insecure Connection | desligado |

**Save & test** tem que responder *datasource is working*.

O Grafana roda local, o banco está na nuvem: quem precisa de internet é o Grafana,
não o ESP32.

### InfluxDB local (IoT-platform), em Flux

Para os arquivos `*_INFLUXDBLOCAL.json`, uma segunda conexão. Os valores são os do `.env`
da IoT-platform:

| Campo | Valor |
|---|---|
| Name | `influxdb-local` |
| Query language | **Flux** |
| URL | `http://influxdb:8086` — o nome do contêiner, não `localhost` |
| Auth | tudo desligado |
| Organization | `fiapiot` (`INFLUXDB_ORG`) |
| Token | `INFLUXDB_TOKEN` |
| Default Bucket | `sensores` (`INFLUXDB_BUCKET`) |

**Save & test** responde *datasource is working. N buckets found*.

O Grafana e o InfluxDB estão na mesma rede Docker: nada sai do computador.

## 2. Importar

Dashboards > New > **Import** > Upload JSON file.

Na tela de import, o Grafana pergunta qual é a fonte de dados InfluxDB — escolha a que
você acabou de criar. Depois de abrir, no topo tem a caixa **Measurement**, que já
vem com `nexolog` — o nome que o Node-RED grava. Se o seu grava em outra, escreva o nome
e dê Enter. Todos os painéis usam essa caixa, então é um lugar só.

Nos `*_INFLUXDBLOCAL.json` há uma segunda caixa, **Bucket**, que já vem com `sensores`.

> Os arquivos foram exportados do Grafana 13.2.1, no schema `dashboard.grafana.app/v2`.
> Se a sua versão recusar a variável **Measurement** na importação, crie à mão:
> Dashboard settings > Variables > New > **Textbox**, nome `measurement`. É o nome que as
> consultas procuram, em `"${measurement}"`.

## 3. O dashboard de medições

Onze painéis, com os mesmos limites do n8n:

| Painel | Tipo | O que mostra |
|---|---|---|
| Temperatura, Umidade, Movimentação | gauge | o valor de agora, verde / amarelo / vermelho |
| Tampa | bar gauge vertical | a distância como nível de tanque |
| Última leitura | stat | "há 2 segundos" — o ESP32 está vivo? |
| Leituras no intervalo | stat | quantos pontos há no intervalo do topo |
| Temperatura e umidade | série temporal | uma régua de cada lado |
| Distância até a tampa | série temporal | a linha fica vermelha com a tampa aberta |
| Movimentação | série temporal | idem, acima de 3 m/s² |
| Acelerômetro (x, y, z) | série temporal | parada, um eixo fica em 9,8: a gravidade |
| Últimas leituras | tabela | as 20 linhas mais novas, com cor no que passou do limite |

| Faixa | Verde | Amarelo | Vermelho |
|---|---|---|---|
| temp | < 25 °C | 25 a 30 | > 30 |
| umid | < 60 % | 60 a 70 | > 70 |
| dist | < 20 cm | 20 a 25 | > 25 |
| movimentacao | < 1,5 m/s² | 1,5 a 3 | > 3 |

Todas as consultas saem do mesmo modelo — a measurement é uma tabela, cada campo é uma
coluna:

```sql
SELECT time, temp, umid
FROM "${measurement}"
WHERE $__timeFilter(time)
ORDER BY time
```

Os indicadores invertem: `ORDER BY time DESC LIMIT 1`, olhando os últimos 5 minutos.

No InfluxDB local, a mesma consulta em Flux. O `pivot` é o passo que transforma o Flux
— uma linha por campo — na tabela que o SQL já entrega, com `temp` e `umid` lado a lado:

```flux
from(bucket: "${bucket}")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "${measurement}")
  |> filter(fn: (r) => r._field == "temp" or r._field == "umid")
  |> pivot(rowKey: ["_time"], columnKey: ["_field"], valueColumn: "_value")
  |> group()
  |> sort(columns: ["_time"])
  |> keep(columns: ["_time", "temp", "umid"])
```

Os indicadores trocam o fim por `sort(columns: ["_time"], desc: true)` e `limit(n: 1)`,
com `range(start: -5m)`.

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

A consulta é uma só: a do gauge, com as quatro colunas no SELECT. Uma linha, os quatro
valores lado a lado — cada elemento pega o seu pelo nome.

## Deu errado

| Sintoma | Onde olhar |
|---|---|
| `table ... not found` | a caixa **Measurement** no topo, ou o Database da fonte de dados |
| `unauthorized` | o token da fonte de dados |
| `bucket ... not found` (local) | a caixa **Bucket** no topo: `sensores`, ou o `INFLUXDB_BUCKET` do seu `.env` |
| Conexão local recusada | a URL é `http://influxdb:8086`; `localhost` dentro do contêiner do Grafana é o próprio Grafana |
| Gauge com "No data" e a série cheia | o gauge olha os últimos 5 minutos: o ESP32 parou de publicar. A "Última leitura" confirma |
| Canvas com fundo escuro | a URL da imagem em **Background > Image**, no frame de fora |
| Canvas mostra o rótulo e não o número | o elemento está em Fixed em vez de Field, ou o nome do campo não bate |
| Tudo "No data" | confira antes no InfluxDB, Data Explorer. Se não tem lá, o problema é o Node-RED |
