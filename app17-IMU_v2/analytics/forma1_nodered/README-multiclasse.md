# app17-11 — Ponte MQTT → InfluxDB (multiclasse)

Fluxo do **Node-RED** que assina o tópico MQTT do `app17-11-MotorMultiClasse`
(`FIAPIoT/motor/multiclasse`) e grava cada janela de features em um **InfluxDB na nuvem**.
Mesmo padrão do [`flow_features_influx.json`](flow_features_influx.json) (Forma 1 binária,
`app17-9`), adaptado para as 5 classes e a coluna `rodada`.

## O que o fluxo faz

```
[mqtt in]  FIAPIoT/motor/multiclasse
   → [function]  Monta ponto InfluxDB (classe e rodada viram TAGS; o resto vira fields)
   → [influxdb out]  measurement vibracao_multiclasse (InfluxDB nuvem)
```

- **Não há botão no Node-RED.** A classe atual vem da **sequência cíclica do próprio ESP32**
  (`app17-11`) — o fluxo só repassa `classe` e `rodada` como **tags**.
- **Tags:** `classe` (`desligado`/`operando`/`inclinado_frente`/`inclinado_tras`/`anomalia`) e
  `rodada` (identifica a volta completa pela sequência — é o que o notebook `2.6` usa no
  `LeaveOneGroupOut`).
- **Fields:** `janela`, `fs_real` e as 12 features (`mean_ax/ay/az`, `std_ax/ay/az`, `rms_mag`,
  `std_mag`, `p2p_mag`, `crest_mag`, `kurt_mag`, `zcr_mag`).

## Pré-requisitos

Os mesmos do fluxo binário — ver [`README.md`](README.md#pré-requisitos) desta pasta:
IoT-platform no ar, `node-red-contrib-influxdb` instalado, conta InfluxDB Cloud com bucket e
token de escrita.

## Como importar e configurar

1. No Node-RED: **Menu (≡) → Import** → cole o conteúdo de
   [`flow_multiclasse_influx.json`](flow_multiclasse_influx.json) → **Import**.
2. Abra o nó **InfluxDB Cloud (EDITAR url/org/token)** e preencha URL/token/`Version: 2.0` —
   pode reaproveitar a mesma configuração do fluxo binário se já estiver pronta.
3. Abra o nó **InfluxDB nuvem (vibracao_multiclasse)** e ajuste `Organization` e `Bucket`.
   `Measurement` já vem preenchido (`vibracao_multiclasse`).
4. Confira o nó **MQTT** (`Mosquitto da IoT-platform`): `broker = mosquitto`, `port = 1883`.
5. **Deploy**.

## Como usar (coleta)

1. Suba o **app17-11** com WiFi/broker configurados (bloco B do `.cpp`, para ESP32 físico).
2. Siga o protocolo de coleta do [README do app17-11](../../app17-11-MotorMultiClasse/README.md)
   (botão MOTOR + botão COLETA, sequência cíclica de 5 classes).
3. Observe o nó **debug** e o `status` verde da função (`classe rN | fs_real=…`).
4. Confira no **InfluxDB Cloud** (Data Explorer) o measurement `vibracao_multiclasse`.

### Conferir no InfluxDB (Flux)

```flux
from(bucket: "sensores")
  |> range(start: -1h)
  |> filter(fn: (r) => r._measurement == "vibracao_multiclasse")
  |> pivot(rowKey: ["_time"], columnKey: ["_field"], valueColumn: "_value")
```

## Limitações

As mesmas da Forma 1 binária (edge perde o raw; janela fixa pode cortar o fenômeno; o horário
gravado é o de recepção no servidor — `fs_real` mede a taxa efetiva no ESP32, não substitui um
timestamp por amostra).
