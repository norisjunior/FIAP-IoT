# app17-10-RawSerialCSV — Forma 2: coleta RAW via Serial

Firmware do ESP32 que imprime **só `ax,ay,az` a 100 Hz na Serial**, continuamente.
Não calcula features e não usa MQTT/WiFi — é a evolução direta do `app17-8`/`app17-9`,
mantendo pinagem, lib (FlixPeriph) e estilo.

> **Forma 2 (raw):** o ESP32 manda o sinal bruto; o `coletor_raw.py` põe o **timestamp** e a
> **label** e grava o CSV. Guardamos o raw — no Colab dá para **ver o sinal inteiro**, recortar
> o trecho exato do fenômeno e **re-janelar** (o que a Forma 1, só com features, não permite).

## Montagem (mínima, sem motor / sem botões / sem LED)

Só **ESP32 + MPU6050 sobre a mesa**. A condição é escolhida no script Python, não no hardware:

- **NORMAL** = MPU6050 **parado** sobre a mesa (motor parado).
- **ANOMALIA** = **chacoalhar o ESP32/MPU**. No Wokwi, **sacudir o MPU6050** no painel.

| Função | GPIO |
|---|---|
| MPU SDA | 19 |
| MPU SCL | 18 |

## O que ele faz

- Coleta de aceleração X/Y/Z a 100 Hz, exibida no Monitor Serial no
  formato `ax,ay,az` (ex.: `0.12,-0.04,9.81`).

A captura com timestamp (item 2), a base por condição (item 3) e a visualização (item 4)
ficam no `analytics/coletor_raw/` e nos notebooks `2.3`/`2.4`/`2.5`.

## Como rodar

### Opção A — Wokwi (simulador no VS Code)

1. Compile e **abra o simulador** (deixe a aba do Wokwi **visível**, senão a simulação pausa).
2. O [`wokwi.toml`](wokwi.toml) já expõe a Serial via **RFC2217** em `rfc2217ServerPort = 4000`.
3. Rode o coletor apontando para `rfc2217://localhost:4000` (ver `analytics/coletor_raw/`).
4. Para gerar **anomalia**, **sacuda o MPU6050** no painel do Wokwi durante a coleta.

### Opção B — ESP32 físico

1. Compile e grave no ESP32.
2. **Feche o Serial Monitor** (só um programa abre a porta COM por vez).
3. Rode o coletor apontando para a sua porta (ex.: `COM6`).
4. Para gerar **anomalia**, **chacoalhe** o ESP32/MPU durante a coleta.

### Protocolo de coleta (Aula 14, slide 22)

1. Escolha a condição ao iniciar o `coletor_raw.py` (`normal` ou `anomalo`).
2. Mantenha a condição **estável** o tempo todo; **descarte os primeiros segundos** (transição).
3. Colete **~1 min por classe** (~6000 amostras a 100 Hz).
4. Faça **rodadas separadas** por classe — isso facilita o split treino/teste depois.

## Limitações (Forma 2)

- **Timestamp é de recepção no PC** (hora em que o Python leu a linha), não o instante exato da
  medição no ESP32. Há latência Serial/USB. (Aula 14, slide 23.)
- **Anomalia simulada:** chacoalhar o MPU (ou sacudir no Wokwi) **não substitui** um ensaio de
  vibração industrial — serve para aprender o pipeline.
- **Sem botão:** a separação entre normal e anomalia é feita por **rodada** (uma execução do
  coletor por classe), não por marcação dentro de um mesmo arquivo.
