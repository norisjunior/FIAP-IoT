# PROMPT / spec da resposta-modelo (raiz do `app17-IMU`)

> Documento alinhado ao que foi **efetivamente entregue**. Descreve a arquitetura da
> resposta-modelo das Sprints 3 e 4. Leia também o `CLAUDE.md` (convenções) e
> `docs/Sprints_3_e_4.md`, `docs/Aula14_resumida.md`.

A infra **já existe**: é a **IoT-platform da disciplina** (`../IoT-platform/`, no WSL2).
**Não criar `docker-compose.yml` nem pasta `analytics/infra/`.** O MQTT é o Mosquitto local da
plataforma; o **InfluxDB de destino é na NUVEM** (o Colab precisa alcançá-lo).

A resposta-modelo resolve o mesmo problema (normal × anomalia) por **duas formas
complementares** — o contraste entre elas é o ponto didático:

- **Forma 1 — Edge / janela fixa:** features calculadas no ESP32 (janela fixa) → MQTT →
  **Node-RED** → InfluxDB nuvem. Eficiente, mas perde o raw e a janela pode cortar o fenômeno.
- **Forma 2 — Raw / Serial→CSV:** ESP32 envia só `ax,ay,az` pela Serial → `coletor_raw.py` põe
  timestamp + label e gera CSV. Guarda o raw e permite **fatiar o dataset no momento exato** do
  fenômeno (re-janelamento no Colab).

As duas terminam no **mesmo ML**: **Random Forest + StandardScaler**, exportável para o ESP32
via **micromlgen** (AIoT embarcado — passo futuro, igual aos `app21`/`app22`). Os firmwares
derivam do `app17-8` (não foram reescritos do zero).

---

## Forma 1 — Edge Analytics no dispositivo

```
app17-IMU/app17-9-EdgeJanelaFixaInflux/      # ✅ PRONTO — NÃO ALTERAR O FIRMWARE
└── src/app17-9-edgejanelafixa.cpp
    # janela fixa 100 amostras @ 100 Hz, 7 features, label por botão (25),
    # 1 JSON/janela em MQTT (FIAPIoT/motor/features), ts_epoch_ms via NTP.

analytics/forma1_nodered/
├── flow_features_influx.json
│   # fluxo Node-RED (modelo no app16-Edge) com node-red-contrib-influxdb.
│   # assina FIAPIoT/motor/features; função: label -> TAG, features -> fields;
│   # grava no measurement vibracao_features no InfluxDB NUVEM.
│   # SEM botão no Node-RED: a condição vem do botão do ESP32 (campo label).
└── README.md                                # import, config do nó InfluxDB nuvem, placeholders

analytics/notebooks/
├── 1.3_visualizacao_features_influx.ipynb
│   # conecta no InfluxDB nuvem (Colab), consulta vibracao_features,
│   # séries + boxplots das features por label (ligado_normal vs ligado_anomalia).
└── 1.5_random_forest_influx.ipynb
    # StandardScaler + RandomForest (n_estimators=20, max_depth=8),
    # entrada = 7 features (mean_ax/ay/az, std_ax/ay/az, rms_mag).
    # classes: ligado_normal vs ligado_anomalia (descarta "parado").
    # split CRONOLÓGICO por classe (NÃO train_test_split aleatório).
    # métricas + matriz de confusão + classification_report
    # + feature importance (nativa do RF + permutation_importance) com gráfico de barras.
    # + export micromlgen: AIoTVibracaoRF_micromlgen.hpp + AIoTVibracaoScaler.hpp.
```

## Forma 2 — Coleta raw via Serial e analytics no Colab

```
app17-IMU/app17-10-RawSerialCSV/
├── (estrutura PlatformIO/Wokwi: platformio.ini, wokwi.toml, diagram.json, README.md, .gitignore)
│   # platformio.ini: só FlixPeriph. wokwi.toml: rfc2217ServerPort = 4000.
│   # diagram.json: só ESP32 + MPU6050 (SEM botões/LED).
└── src/app17-10-rawserialcsv.cpp
    # Derivado do app17-8, mas SEM features, SEM MQTT, SEM botões/LED:
    # imprime "ax,ay,az" na Serial a 100 Hz (millis(), nunca delay()), contínuo.
    # A label é informada no coletor Python (uma rodada = uma classe).

analytics/coletor_raw/
├── coletor_raw.py
│   # pyserial, linear (estilo app17-0-Plot). Conexão: rfc2217://localhost:4000 (Wokwi)
│   # ou COM (físico). Lê a Serial, adiciona timestamp (hora do PC) e a label da rodada,
│   # grava CSV: timestamp,ax,ay,az,label (formato exato da Sprint 3).
├── requirements.txt                          # pyserial
└── README.md

analytics/notebooks/
├── 2.3_visualizacao_raw_csv.ipynb        # série temporal por eixo + magnitude, normal vs anômala (lê o CSV).
├── 2.4_janelas_features_split.ipynb      # janela fixa a partir do raw recortado (descarta transição),
│                                         # extrai as 7 features, split cronológico por classe; salva features_from_raw.csv.
└── 2.5_random_forest_raw.ipynb           # mesmo RF + micromlgen sobre as features do Colab.
```

## Requisitos transversais

1. Siga TODAS as convenções do CLAUDE.md (PlatformIO, FlixPeriph/MPU6500, pinagem SDA=19/SCL=18,
   m/s², comentários em português, sem abstrações).
2. `app17-10` é **streaming contínuo**: `diagram.json` só ESP32 + MPU6050 (sem botões/LED).
3. **Broker (Forma 1):** físico = IP da IoT-platform; Wokwi = `Wokwi-GUEST` +
   `host.wokwi.internal:1883`. **NUNCA `test.mosquitto.org`.** (A Forma 2 não usa MQTT.)
4. **Serial no Wokwi (Forma 2):** não há COM real — exponha via `rfc2217ServerPort = 4000` no
   `wokwi.toml` e conecte o Python em `rfc2217://localhost:4000`.
5. No Wokwi a anomalia é simulada **sacudindo o MPU** no painel; comente isso no código.
6. README.md em cada bloco mapeando arquivo → item da Sprint 3/4 e ordem de execução; README raiz
   em `analytics/README.md`.
7. Documentar nos READMEs as limitações de cada forma:
   - Forma 1: edge perde o raw; janela fixa pode pegar o fenômeno pela metade; tempo do ponto é o
     de gravação no servidor (latência de rede).
   - Forma 2: o timestamp é de recepção no PC (não da medição); Wokwi simula anomalia sacudindo o MPU.

## Estrutura entregue (ordem de execução)

```
1. (plano técnico aprovado)
2. analytics/forma1_nodered/        — fluxo Node-RED + README
3. notebooks 1.3 e 1.5              — Forma 1, lendo do InfluxDB nuvem
4. app17-10-RawSerialCSV/           — firmware raw via Serial (derivado do app17-8/9)
5. analytics/coletor_raw/           — script Serial→CSV + requirements + README
6. notebooks 2.3, 2.4 e 2.5         — Forma 2, lendo do CSV
7. analytics/README.md              — mapa arquivo → Sprint + contraste entre as formas
8. analytics/AUTOAVALIACAO.md       — autoavaliação com docs/checklist-revisao.md
```

> `app17-9-EdgeJanelaFixaInflux/` já estava pronto — **o firmware dele não foi alterado**.
>
> **Passo futuro (não incluído):** firmware de inferência embarcada no ESP32 (AIoT) consumindo
> os `.hpp` gerados pelos notebooks (micromlgen + scaler), nos moldes do `app21`/`app22`.
