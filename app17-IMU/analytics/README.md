# Analytics — Sprints 3 e 4 (vibração: normal × anomalia)

Mesma pergunta de engenharia (o ativo está **normal** ou em **anomalia**?) resolvida por
**duas formas complementares**. O contraste entre elas é o ponto didático.

| | **Forma 1 — Edge / janela fixa** | **Forma 2 — Raw / Serial→CSV** |
|---|---|---|
| Firmware | `app17-9-EdgeJanelaFixaInflux` (features no ESP32) | `app17-10-RawSerialCSV` (só `ax,ay,az`) |
| Transporte | MQTT → **Node-RED** → InfluxDB **nuvem** | Serial → **Python** → CSV |
| O que trafega | 7 features por janela (1 JSON/janela) | sinal bruto a 100 Hz |
| Análise | Colab lendo o InfluxDB | Colab lendo o CSV |
| Vantagem | eficiente (pouco dado) | guarda o raw; re-janela à vontade |
| Limitação | perde o raw; janela fixa pode cortar o fenômeno | timestamp é da recepção no PC |

As duas terminam no **mesmo modelo**: Random Forest + StandardScaler, exportável para o ESP32
com **micromlgen** (AIoT embarcado — passo futuro, igual aos `app21`/`app22`).

## Estrutura

```
app17-IMU/
├── app17-9-EdgeJanelaFixaInflux/   # Forma 1 — firmware (PRONTO)
├── app17-10-RawSerialCSV/          # Forma 2 — firmware (streaming raw)
└── analytics/
    ├── forma1_nodered/             # Forma 1 — fluxo MQTT→InfluxDB (Node-RED)
    │   ├── flow_features_influx.json
    │   └── README.md
    ├── coletor_raw/                # Forma 2 — Serial→CSV
    │   ├── coletor_raw.py
    │   ├── requirements.txt
    │   └── README.md
    └── notebooks/
        ├── 1.3_visualizacao_features_influx.ipynb
        ├── 1.5_random_forest_influx.ipynb
        ├── 2.3_visualizacao_raw_csv.ipynb
        ├── 2.4_janelas_features_split.ipynb
        └── 2.5_random_forest_raw.ipynb
```

## Ordem de execução

**Forma 1:** subir IoT-platform → app17-9 (Wokwi/físico) → importar e configurar o fluxo
Node-RED → coletar (botão do ESP32) → notebook **1.3** (visualizar) → **1.5** (treinar + exportar).

**Forma 2:** app17-10 (Wokwi com `rfc2217` / físico) → `coletor_raw.py` (1 rodada por classe) →
notebook **2.3** (visualizar) → **2.4** (janelar + extrair features) → **2.5** (treinar + exportar).

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

## Regra de ouro do ML (Aula 14, slide 25)

Nunca `train_test_split` aleatório: janelas vizinhas no tempo são quase iguais e vazam
informação. Aqui usamos **split cronológico por classe** (primeiras 70% → treino, últimas 30%
→ teste). Evolução ideal: coletar **rodadas numeradas** (`normal_01`, `normal_02`…) e separar
treino/teste por rodada.

## Limitações (resumo para o vídeo/entrega)

- **Forma 1:** edge perde o raw; janela fixa pode pegar o fenômeno pela metade; tempo do ponto
  é o de gravação (latência de rede).
- **Forma 2:** timestamp é o de recepção no PC, não o da medição.
- **Ambas:** chacoalhar/dar tapas (ou sacudir o MPU no Wokwi) **não substitui** um ensaio de
  vibração industrial — serve para aprender o pipeline.
