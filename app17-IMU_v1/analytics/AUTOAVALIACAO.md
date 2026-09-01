# Autoavaliação — `docs/checklist-revisao.md`

Revisão item a item do checklist (já **alinhado** à arquitetura desta resposta-modelo:
Node-RED → InfluxDB nuvem na Forma 1; raw Serial/rfc2217 → CSV na Forma 2; Random Forest +
micromlgen como modelo).

## 1. Consistência com o repositório
- ✅ Forma 1 = `app17-9` (PRONTO, não alterado); Forma 2 = `app17-10-RawSerialCSV`.
- ✅ `app17-10` com estrutura PlatformIO completa (`platformio.ini`, `src/`, `wokwi.toml`, `diagram.json`, `README.md`, `.gitignore`).
- ✅ `lib_deps` do `app17-10`: só FlixPeriph (Serial puro, sem MQTT/JSON).
- ✅ Classe `MPU6500` com comentário para trocar por `MPU6050`; `ACCEL_RANGE_8G`.
- ✅ Pinagem `app17-10`: SDA=19, SCL=18 (sem pinos de motor).
- ✅ `app17-10` é streaming contínuo: `diagram.json` só ESP32 + MPU6050 (sem botões/LED).
- ✅ `wokwi.toml` do `app17-10` expõe a serial via `rfc2217ServerPort = 4000`.
- ✅ Nenhum código antigo (app17-0…app17-9) alterado.
- ✅ Aceleração em m/s² em código, comentários e exemplos.

## 2. Labels e tags
- ✅ Firmware Forma 1 (app17-9): `parado` / `ligado_normal` / `ligado_anomalia`.
- ✅ ML usa só duas classes; `parado` é descartado nos notebooks.
- ✅ Tag gravada = `label` (Forma 1: do botão do ESP32; Forma 2: da rodada do coletor).
- ✅ Split treino/teste **cronológico por classe** (70/30), nunca aleatório. Rodadas numeradas
  (`normal_01`…) registradas como evolução futura.

## 3. Firmware
- ✅ app17-9 (Forma 1): intacto.
- ✅ app17-10 (Forma 2): raw `ax,ay,az` @100 Hz na Serial, sem features, sem MQTT.
- ✅ Amostragem via `millis()`, sem `delay()`.
- ✅ Forma 1: broker é o da IoT-platform (Wokwi-GUEST + host.wokwi.internal no app17-9). NUNCA `test.mosquitto.org`.

## 4. Infra e scripts
- ✅ Sem `docker-compose.yml` / `analytics/infra/` próprios.
- ✅ MQTT local (`mosquitto:1883` / `host.wokwi.internal`); InfluxDB de destino na **nuvem**, com `url/org/token/bucket` como placeholders.
- ✅ Forma 1: fluxo Node-RED (`flow_features_influx.json` + `node-red-contrib-influxdb`) grava em `vibracao_features` (tag `label`), sem botão no fluxo; README cita Telegraf/script Python como alternativas.
- ✅ Forma 2: `coletor_raw.py` linear, lê Serial (rfc2217 no Wokwi / COM no físico), gera CSV `timestamp,ax,ay,az,label`.

## 5. Notebooks
- ✅ Células markdown explicando o conceito antes de cada código.
- ✅ 1.3 (features/Influx) e 2.3 (raw/CSV): visualização normal × anômala.
- ✅ 2.4: janela a partir do raw + descarte de transição + split por ordem temporal, com explicação do vazamento.
- ✅ 1.5/2.5: StandardScaler + Random Forest (n_estimators=20, max_depth=8) + export micromlgen (`.hpp` do modelo e do scaler).
- ✅ Métricas: accuracy, matriz de confusão, classification_report.
- ✅ Feature importance: nativa do RF + `permutation_importance`, gráfico de barras (25 pts S4).
- ✅ Célula relacionando features importantes ao fenômeno físico.
- ✅ Forma 1 não promete visualizar raw; Forma 2 guarda o raw e fatia o fenômeno (2.4).

## 6. Cobertura das Sprints
- ✅ S3: coleta 100 Hz; CSV `timestamp,ax,ay,az,label`; alvo ~6000 amostras/classe.
- ✅ S3: visualização comparando classes (2.3).
- ✅ S4: features por janela; dataset balanceado; `assert` de ≥30 janelas/classe (2.4).
- ✅ S4: treino/teste separados, métricas, matriz de confusão (1.5/2.5).
- ✅ S4: feature importance + interpretação física + limitações.
- ✅ README raiz (`analytics/README.md`) mapeia cada arquivo ao item da Sprint.

## 7. Didática e limitações
- ✅ Código simples, sem classes customizadas nem abstrações; comentários em português.
- ✅ READMEs explicam: edge perde o raw; timestamp Python é de recepção; Wokwi simula anomalia
  sacudindo o MPU; chacoalho não substitui ensaio industrial.

## Pendências que dependem de você
1. **Credenciais do InfluxDB Cloud** (url/org/token/bucket) no fluxo Node-RED e nos notebooks 1.3/1.5.
2. **Instalar `node-red-contrib-influxdb`** no Node-RED da plataforma (Manage palette).
3. **Coletar os dados** (Wokwi/físico) para rodar os notebooks de ponta a ponta.

## Não verificado nesta sessão (sem hardware/dados)
- Compilação do `app17-10` no PlatformIO/Wokwi.
- Conexão `rfc2217://localhost:4000` ponta a ponta.
- Execução dos notebooks com dados reais e compilação dos `.hpp` no ESP32.
