# app16-4-Fusao1Hz-MQTT

O coletor do dataset da caixa de fumaça (o [app16-3](../app16-3-Fusao1Hz/)) com MQTT. A mesma
linha por segundo sai em dois lugares:
- **no Serial**, em CSV (uma linha por segundo, com cabeçalho no boot), para o `coleta.py` (painel e dataset, sem rede);
- **no MQTT**, em JSON, para o Node-RED gravar no InfluxDB Cloud. O Grafana mostra as curvas,
  e o Colab treina e prevê.

**Pronto quando:** o Grafana recebe um ponto por segundo e o notebook do professor lê a última
leitura.

## O que muda em relação ao app16-3

| | app16-3 | app16-4 |
|---|---|---|
| Saída | Serial em Teleplot (`coleta.py`) | Serial em CSV (`coleta.py`) e MQTT (`FIAPIoT/fumaca/caixa`, 1 JSON por segundo, ArduinoJson) |
| Rede | não tem | Wi-Fi e broker no começo do `.ino` |
| Sem rede | — | tenta o Wi-Fi por 10 s e segue só pelo Serial; sem broker, tenta de novo a cada 5 s sem travar a leitura |

As colunas, os sensores, o botão BOOT, o protocolo e a coleta do professor de 11/11 estão no
[README do app16-3](../app16-3-Fusao1Hz/README.md).

## O caminho

```
caixa (1 linha/s) → MQTT → Node-RED → InfluxDB Cloud → Grafana (ver)
                                                      → Colab (treinar e prever)
```

| Peça | Arquivo | O que faz |
|---|---|---|
| Firmware | `src/app16-4-fusao1hz-mqtt.ino` | publica a linha em `FIAPIoT/fumaca/caixa` |
| Node-RED | [`NodeRED/flow_fumaca_influx.json`](NodeRED/flow_fumaca_influx.json) | assina o tópico e grava no measurement `fumaca` |
| Grafana | [`Grafana/dashboard_fumaca_INFLUXDBCLOUD.json`](Grafana/dashboard_fumaca_INFLUXDBCLOUD.json) | o agora, as 4 curvas, o estímulo no tempo e as últimas linhas |
| Colab (professor) | [`colab/treinamento_fumaca.ipynb`](colab/treinamento_fumaca.ipynb) | lê o banco; um dia treina, o outro testa; a última leitura; exporta o dataset dos alunos |
| Colab (aluno) | [`colab/caixa_de_fumaca.ipynb`](colab/caixa_de_fumaca.ipynb) | abre o `dataset_detector_fumaca_fiap.csv`: a fusão, a árvore, a matriz fire/not fire, a inferência |
| Colab (aluno) | [`colab/estudo_caso_kaggle.ipynb`](colab/estudo_caso_kaggle.ipynb) | abre o `dataset_detector_fumaca_kaggle.csv`: 0,99 no split aleatório, perto de zero por gravação |
| Dataset | [`colab/montar_dataset_fiap.py`](colab/montar_dataset_fiap.py) | monta o `dataset_detector_fumaca_fiap.csv` a partir das coletas brutas |

**1. Firmware.** A rede fica no começo do `.ino`, como no app17-11: o Wi-Fi `NorisIoT` e o
broker na máquina da IoT-platform (`172.16.10.101`).

**2. Node-RED** (IoT-platform no ar). Menu → Import → o `flow_fumaca_influx.json`. No nó
**Gravar no InfluxDB**:
- a conexão: URL da sua região e token. Se você já tem a conexão "InfluxDB Cloud" do app15
  ou do app17, escolha ela e apague a que veio com o fluxo;
- a org e o bucket (`IoTSensores`, o mesmo do app17-7).

| | O quê |
|---|---|
| measurement | `fumaca` |
| tags | `coleta` (o dia, no horário de Brasília: `2026-11-19`) e `device` |
| fields | as 16 colunas do CSV do app16-3, todas decimais |

**Uma coleta por dia:** o Node-RED marca a tag `coleta` com a data. Duas coletas no mesmo dia
viram uma.

**3. Grafana** (local, lendo o InfluxDB Cloud em SQL). A fonte de dados é a mesma do app15:
`influxdb-sql`, com Query language **SQL**. Dashboards → New → Import → o
`dashboard_fumaca_INFLUXDBCLOUD.json` → escolha a fonte. A caixa **Measurement** já vem com
`fumaca`. Schema v2 do Grafana 13.2, como os do app15.

**4. Colab (professor).** Abra o `treinamento_fumaca.ipynb`, ponha o token nos Secrets
(`INFLUX_TOKEN`) e rode em ordem:
- **1b, uma vez:** sobe as duas coletas de 11/11 (tags `2026-11-11 18:48` e `2026-11-11 22:07`);
- **prepara o dataset:** tira a rodada 1, os 30 s depois de cada aperto (120 s no ar) e as
  colunas que não viram feature; o alvo é `incendio` = `fire` / `not fire`;
- **um dia treina, o outro testa** (`DIA_TREINO`, `DIA_TESTE`): árvore de 3 níveis, matriz de
  confusão fire/not fire, o que o modelo disse por estímulo e o split aleatório para comparar;
- salva o `modelo_fumaca_incendio.pkl`;
- **a última leitura:** uma consulta (`LIMIT 1`) e o modelo responde. Opcional: ao vivo, a cada
  5 s;
- **exporta o dataset** no formato do `dataset_detector_fumaca_fiap.csv`.

## Dia de treino, dia de teste

- **Treino: 11/11**, as duas coletas, que têm fumaça e chama.
- **Teste: outro dia**, gravado pelo MQTT só com condições normais e álcool (vapor, se der).
  Fumaça e chama não entram em sala, nem em caixa fechada. No teste só há `not fire`: a matriz
  mede o **alarme falso**.
- **Para chegar ao álcool**, o BOOT passa por chama e fumaça: passe em menos de 30 s. O preparo
  descarta esses segundos, e nenhuma linha do dia de teste sai como `fire`.

## Os datasets dos alunos

Ficam em `colab/`, com os notebooks. O aluno recebe os CSV e os `.ipynb` e sobe no Colab.

**`dataset_detector_fumaca_fiap.csv`**, já preparado (o aluno não corta nada):

| Coluna | O que é |
|---|---|
| `coleta` | `2026-11-11 18:48` ou `2026-11-11 22:07` (depois, o dia de teste) |
| `estimulo` | ar, álcool, vapor, fumaça, chama (o botão BOOT) |
| `temp`, `umid` | calor (°C) e umidade (%), BME280 |
| `raw_h2`, `raw_ethanol` | gás, valor cru do SGP30: **cai** quando o gás sobe |
| `pm2_5`, `pm10`, `nc0_5` | partícula, SPS30 |
| `incendio` | `fire` (fumaça ou chama) ou `not fire` (ar, vapor, álcool) |

- **3.081 linhas**, 334 de `fire`: 1.383 da coleta das 18:48 e 1.698 da das 22:07.
- **7 features**, as que os dois programas da caixa medem igual. A coleta das 18:48 é do teste
  dos fenômenos (app16-2): o `tvoc` e o `eco2` saíram sem a correção de umidade, e o `pm1_0`, o
  `nc1_0` e o `nc2_5` não foram gravados.
- **Para refazer:** `python montar_dataset_fiap.py`, nesta pasta `colab/`. Ele lê os brutos de
  `coleta_professor/` (o log das 18:48 já convertido em
  `../app16-2-TesteFenomenos/coleta_professor/coleta-20261111-1848-dataset.csv`; se faltar,
  converte de novo) e aplica o mesmo preparo do notebook do professor.
- **Com o dia de teste no banco:** a seção 11 do notebook do professor exporta o mesmo arquivo
  com o dia novo. O notebook do aluno treina em 11/11 e testa nele sozinho.

**`dataset_detector_fumaca_kaggle.csv`:** o CSV do
[Smoke Detection Dataset](https://www.kaggle.com/datasets/deepcontractor/smoke-detection-dataset),
que no Kaggle se chama `smoke_detection_iot.csv`.

**Testado em 12/11/2026, sem o banco de verdade:**
- o notebook do professor rodou inteiro contra um InfluxDB simulado: as duas coletas de 11/11 e
  um dia de teste com ar, álcool e passagens de 10 s por chama e fumaça (descartadas);
- o CSV exportado por ele, na parte de 11/11, é idêntico ao do `montar_dataset_fiap.py`;
- no notebook do aluno, treinando na coleta das 18:48 e testando na das 22:07: alarme falso de
  30% (todo no ar) e nenhum fogo perdido; o pico do vapor e o da fumaça (a partícula quase
  empata) saem certos.

## Decisões

- **Um alvo só, `fire` / `not fire`:** o mesmo do `Fire Alarm` do Kaggle.
- **Árvore de 3 níveis:** dá para ler a regra, e o `.pkl` serve para a nuvem e a borda.
- **Correção de umidade:** é o uso previsto do SGP30. Quem fizer a inferência depois (nuvem
  ou borda) precisa ler os sensores do mesmo jeito: com a correção.
- **Linha de base do SGP30:** não é salva nem restaurada. Os valores de gás de um dia não se
  comparam com os de outro. Isso é data drift, e o dia de teste mostra.
- **A linha só sai com os 3 sensores lidos.** Se um falha, o Serial avisa, e aquele segundo
  fica fora do dataset.
