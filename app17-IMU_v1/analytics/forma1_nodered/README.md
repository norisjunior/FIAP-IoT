# Forma 1 — Ponte MQTT → InfluxDB via Node-RED

Fluxo do **Node-RED** que assina o tópico MQTT do `app17-9-EdgeJanelaFixaInflux`
(`FIAPIoT/motor/features`) e grava cada janela de features em um **InfluxDB na nuvem**.

> **Por que Node-RED e não um script Python?** É a ferramenta de integração que a
> disciplina já usa (ver `app16-Edge`). O fluxo fica visível, sem código de servidor para
> manter, e é o caminho de mercado para "assinar MQTT e persistir em banco de séries
> temporais". (Telegraf seria a alternativa "produção"; Node-RED é mais didático.)

## O que o fluxo faz

```
[mqtt in]  FIAPIoT/motor/features
   → [function]  Monta ponto InfluxDB (label vira TAG; features viram fields)
   → [influxdb out]  measurement vibracao_features (InfluxDB nuvem)
```

- **Não há botão no Node-RED.** A condição (`normal` / `anomalia`) vem do **botão do
  próprio ESP32** (app17-9), que já manda `label` = `ligado_normal` / `ligado_anomalia`
  no JSON. O fluxo só repassa esse `label` como **tag**.
- A **tag** gravada é `label`; os **fields** são as 10 medidas numéricas
  (`mean_ax/ay/az`, `std_ax/ay/az`, `rms_ax/ay/az`, `rms_mag`) + `ts_epoch_ms`.
  Os notebooks usam só **7 features**: `mean_ax/ay/az`, `std_ax/ay/az`, `rms_mag`.

## Pré-requisitos

1. **IoT-platform** da disciplina no ar (Node-RED + Mosquitto locais) — ver
   `IoT-platform/README.md`. O Node-RED roda em http://localhost:1880 (admin / FIAPIoT).
2. O nó **`node-red-contrib-influxdb`** instalado (Menu → *Manage palette* → *Install*).
3. Uma conta/instância **InfluxDB Cloud** com um **bucket** e um **token de escrita**.

## Como importar e configurar

1. No Node-RED: **Menu (≡) → Import** → cole o conteúdo de
   [`flow_features_influx.json`](flow_features_influx.json) → **Import**.
2. Abra o nó de configuração **InfluxDB Cloud (EDITAR url/org/token)** e preencha com os
   **seus** valores (não vêm no arquivo, por serem segredos):
   - **URL**: a da sua região, ex.: `https://us-east-1-1.aws.cloud2.influxdata.com`
   - **Token**: o token do InfluxDB Cloud (com permissão de escrita).
   - **Version**: `2.0`.
3. Abra o nó **InfluxDB nuvem (vibracao_features)** e ajuste:
   - **Organization** (`org`): o nome/ID da sua org no Cloud.
   - **Bucket**: o bucket de destino (ex.: `sensores`).
   - **Measurement**: `vibracao_features` (já preenchido).
4. Confira o nó **MQTT** (`Mosquitto da IoT-platform`): `broker = mosquitto`, `port = 1883`
   (nome do serviço na rede Docker da plataforma). Se rodar o Node-RED fora do Docker, troque
   por `localhost`.
5. **Deploy**.

## Como usar (coleta)

1. Compile/suba o **app17-9** (Wokwi ou ESP32 físico) — ele publica em
   `FIAPIoT/motor/features`.
2. No ESP32: botão **25** seleciona NORMAL/ANOMALIA (com a coleta parada); botão **26**
   inicia/para a coleta. **Mantenha a condição estável** durante toda a coleta e descarte
   os primeiros segundos (transição — Aula 14, slide 22).
3. Observe o nó **debug** e o `status` verde da função (`ligado_normal | rms_mag=…`).
4. Confira no **InfluxDB Cloud** (Data Explorer) o measurement `vibracao_features`.

### Conferir no InfluxDB (Flux)

```flux
from(bucket: "sensores")
  |> range(start: -1h)
  |> filter(fn: (r) => r._measurement == "vibracao_features")
  |> pivot(rowKey: ["_time"], columnKey: ["_field"], valueColumn: "_value")
```

## Mapeamento Sprint

- **Sprint 4 – item 1 (base analítica):** features extraídas por janela, persistidas para
  treino. A visualização e o modelo ficam nos notebooks **1.3** e **1.5**.

## Limitações (Forma 1)

- **Edge perde o raw:** só as 7 features chegam ao banco — não dá para re-janelar nem testar
  outras features depois. Para isso existe a **Forma 2** (raw em CSV).
- **Janela fixa pode cortar o fenômeno:** um evento curto pode cair na fronteira de duas
  janelas (Aula 14, slide 9).
- **Tempo do ponto:** este fluxo usa o **horário de gravação no servidor**; o `ts_epoch_ms`
  (NTP, medido no ESP32) vai como *field* de referência. Há latência de rede entre **medir**
  e **gravar** (Aula 14, slide 23).
- **Sem rodadas numeradas:** por ora a tag é só `label` (normal/anomalia). Para um split
  treino/teste 100% sem vazamento, o ideal é coletar em rodadas separadas
  (`normal_01`, `normal_02`…) — evolução futura.
