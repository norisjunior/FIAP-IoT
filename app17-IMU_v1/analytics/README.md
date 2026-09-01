# Analytics — Sprints 3 e 4 (vibração) + validação com dados reais

Três trilhas. As duas primeiras respondem a mesma pergunta de engenharia (o ativo está
**normal** ou em **anomalia**?) por **duas formas complementares** — o contraste entre elas é
o ponto didático. A terceira estende o problema para **5 classes de um motor real** (hardware
físico), e a quarta valida o pipeline inteiro com um **dataset público de motor real** (Analog
Devices), sem depender de coleta nenhuma.

| | **Forma 1 — Edge / janela fixa** | **Forma 2 — Raw / Serial→CSV** |
|---|---|---|
| Firmware | `app17-9-EdgeJanelaFixaInflux` (features no ESP32) | `app17-10-RawSerialCSV` (só `ax,ay,az`) |
| Transporte | MQTT → **Node-RED** → InfluxDB **nuvem** | Serial → **Python** → CSV |
| O que trafega | 12 features por janela (1 JSON/janela) | sinal bruto a 100 Hz |
| Análise | Colab lendo o InfluxDB | Colab lendo o CSV |
| Vantagem | eficiente (pouco dado) | guarda o raw; re-janela à vontade |
| Limitação | perde o raw; janela fixa pode cortar o fenômeno | timestamp é da recepção no PC |

As duas terminam no **mesmo modelo**: Random Forest + StandardScaler, exportável para o ESP32
com **micromlgen** (AIoT embarcado — passo futuro, igual aos `app21`/`app22`).

**`app17-11-MotorMultiClasse`** é uma terceira trilha, também Forma 1 (features no edge), mas
com **5 classes** de um motor real com hélice (`desligado`, `operando`, `inclinado_frente`,
`inclinado_tras`, `anomalia`) — ver seção própria abaixo.

## Estrutura

```
app17-IMU/
├── app17-9-EdgeJanelaFixaInflux/    # Forma 1 (binária) — firmware
├── app17-10-RawSerialCSV/           # Forma 2 (binária) — firmware (streaming raw)
├── app17-11-MotorMultiClasse/       # Forma 1 (5 classes) — firmware, hardware físico
└── analytics/
    ├── forma1_nodered/
    │   ├── flow_features_influx.json       # Forma 1 binária
    │   ├── flow_multiclasse_influx.json    # app17-11 (5 classes)
    │   ├── README.md
    │   └── README-multiclasse.md
    ├── coletor_raw/                 # Forma 2 — Serial→CSV
    │   ├── coletor_raw.py
    │   ├── requirements.txt
    │   └── README.md
    └── notebooks/
        ├── 1.3_visualizacao_features_influx.ipynb
        ├── 1.5_random_forest_influx.ipynb
        ├── 2.3_visualizacao_raw_csv.ipynb
        ├── 2.4_janelas_features_split.ipynb
        ├── 2.5_random_forest_raw.ipynb
        ├── 2.6_multiclasse_motor.ipynb      # app17-11 (5 classes)
        └── 3.1_dataset_real_cbm.ipynb       # validação com motor real (Analog Devices)
```

## Ordem de execução

**Forma 1 (binária):** subir IoT-platform → app17-9 (Wokwi/físico) → importar e configurar o
fluxo Node-RED → coletar (botão do ESP32) → notebook **1.3** (visualizar) → **1.5** (treinar +
exportar).

**Forma 2 (binária):** app17-10 (Wokwi com `rfc2217` / físico) → `coletor_raw.py` (uma sessão
por classe, várias rodadas por sessão — ver abaixo) → notebook **2.3** (visualizar) → **2.4**
(janelar + extrair features) → **2.5** (treinar + exportar).

**app17-11 (5 classes, hardware físico):** montar o MPU6500 no motor com o gabarito 3D →
gravar o firmware → protocolo de coleta pelos 2 botões (ver `app17-11/README.md`) → notebook
**2.6** (treinar + exportar), lendo o CSV salvo do Serial Monitor ou, com WiFi/broker
configurados, do InfluxDB via `flow_multiclasse_influx.json`.

**3.1 (validação com dados reais):** roda sozinho, sem hardware nenhum — baixa um subconjunto
do [CbM-Datasets](https://github.com/analogdevicesinc/CbM-Datasets) (Analog Devices) em
runtime e reaplica o mesmo pipeline de janela/features/split.

## Mapa arquivo → item da Sprint

| Arquivo | Sprint / item |
|---|---|
| `app17-10-RawSerialCSV/` | **S3-1** coleta 100 Hz, `ax,ay,az` no Serial |
| `coletor_raw/coletor_raw.py` | **S3-2** timestamp por amostra → CSV; **S3-3** base por condição |
| `notebooks/2.3` | **S3-4** visualização raw normal × anômala |
| `app17-9-EdgeJanelaFixaInflux/` + `forma1_nodered/` | **S4-1** base analítica (features por janela) |
| `notebooks/2.4` | **S4-1** features por janela a partir do raw (dataset balanceado, ≥30/classe) |
| `notebooks/1.3` | visualização das features (Forma 1) |
| `notebooks/1.5` e `2.5` | **S4-2** treino/teste + métricas + matriz de confusão; **S4-3** feature importance + interpretação |
| `app17-11-MotorMultiClasse/` + `notebooks/2.6` | extensão: 5 classes, hardware físico, o mesmo pipeline reaplicado sem passar pelo `2.4` (features já vêm prontas do edge) |
| `notebooks/3.1` | validação de ponta a ponta com motor real, sem depender de coleta |

## As features (12 no total — nada foi removido)

`mean_ax/ay/az`, `std_ax/ay/az`, `rms_mag` (as 7 originais) + `std_mag`, `p2p_mag`,
`crest_mag`, `kurt_mag`, `zcr_mag` (5 novas, sobre a magnitude do sinal). As 7 originais foram
**mantidas de propósito** — o notebook `2.5` faz o aluno comparar três conjuntos de features
(`FEATURES_ORIG`, `FEATURES_AC`, `FEATURES_TUDO`) e descobrir, pela importância e pela
interpretação física, por que `mean_*`/`rms_mag` são um atalho a evitar no problema binário
(codificam orientação/saturam na gravidade, não medem vibração).

**O gancho para o `app17-11`:** no problema de 5 classes, `mean_*` deixa de ser atalho — é a
**única família que separa `inclinado_frente` de `inclinado_tras`**. Mesma feature, veredicto
oposto, dependendo da pergunta que o modelo precisa responder. O notebook `2.6` fecha esse
ciclo.

## Split treino/teste: por rodada, sem vazamento (Aula 14, slide 25)

Nunca `train_test_split` aleatório: janelas vizinhas no tempo são quase iguais e vazam
informação. O split é **por rodada** — a última rodada de cada classe vai inteira para teste,
as demais para treino (fallback cronológico 70/30 se só houver 1 rodada). O `coletor_raw.py`
numera as rodadas **automaticamente** dentro de uma sessão (ENTER inicia, ENTER encerra e abre
a próxima, sem reiniciar o script); o `app17-11` numera pela sequência cíclica de classes
(`rodada` incrementa a cada volta completa pelas 5 classes, não a cada toque de botão). Os
notebooks `2.5`/`2.6` também rodam `LeaveOneGroupOut` por rodada e uma curva de aprendizado por
número de rodadas — a resposta medida, não estimada, para "quantos dados eu preciso".

## Limitações (resumo para o vídeo/entrega)

- **Forma 1 (binária) e `app17-11`:** edge perde o raw; janela fixa pode pegar o fenômeno pela
  metade; tempo do ponto é o de gravação (latência de rede, quando há MQTT).
- **Forma 2:** timestamp é o de recepção no PC, não o da medição.
- **Ambas as formas binárias:** chacoalhar/dar tapas (ou sacudir o MPU no Wokwi) **não
  substitui** um ensaio de vibração industrial — serve para aprender o pipeline. O `app17-11`
  já é mais próximo de um ensaio real (motor de verdade, desbalanceamento com fita/massa), mas
  ainda é hardware de bancada didática, não um ensaio industrial.
- **`app17-11` roda a 500 Hz, não 100 Hz** — Nyquist a 100 Hz (50 Hz) fica em cima ou abaixo da
  fundamental de rotação de um motor pequeno (~50–167 Hz). O valor de 500 Hz é provisório até
  medir o RPM real do motor (FFT do raw do `app17-10` reconfigurado a 500 Hz); ver comentário no
  topo do `.cpp`.
- **Dependência de biblioteca:** a `FlixPeriph` (`okalachev/FlixPeriph`) publicou uma versão
  1.12.x que renomeou a classe base (`IMUInterface` → `IMU`), quebrando quem estivesse pinado
  em `^1.10.3`. Os `platformio.ini` do `app17-9`, `app17-10` e `app17-11` estão pinados em
  `1.11.0` (última versão compatível com o código dos três) — não soltar esse pin sem revisar
  o código para a API nova.
