# Checklist de revisão — Resposta-modelo Sprints 3 e 4

Use após cada etapa gerada pelo Claude Code. Adaptado às convenções reais do repositório.

## 1. Consistência com o repositório
- [ ] Forma 1 = app17-9-EdgeJanelaFixaInflux (PRONTO, firmware não alterado); Forma 2 = app17-10-RawSerialCSV
- [ ] Estrutura PlatformIO: platformio.ini, src/, wokwi.toml, diagram.json, README.md, .gitignore
- [ ] lib_deps: FlixPeriph + (Forma 1) PubSubClient/ArduinoJson; a Forma 2 (Serial) não usa MQTT/JSON
- [ ] Classe MPU6500 com comentário para trocar por MPU6050
- [ ] ACCEL_RANGE_8G
- [ ] Pinagem do app17-8: SDA=19, SCL=18, BTN_COLETA=26, BTN_ANOMALIA=25, LED=27 (sem pinos de motor)
- [ ] Nenhum código antigo (app17-0 a app17-9) foi alterado
- [ ] app17-10 é streaming contínuo: diagram.json só ESP32 + MPU6050 (SEM botões/LED — a label vem do coletor Python)
- [ ] wokwi.toml do app17-10 expõe a serial via rfc2217ServerPort = 4000
- [ ] Aceleração em m/s² em código, comentários e exemplos

## 2. Labels e tags
- [ ] Labels do firmware: parado / ligado_normal / ligado_anomalia (sem variações)
- [ ] ML usa apenas ligado_normal vs ligado_anomalia; "parado" descartado no notebook
- [ ] Tag gravada = label (Forma 1: do botão do ESP32 via JSON; Forma 2: da rodada do coletor). Sem tag coleta/rodada numerada por enquanto
- [ ] Split treino/teste cronológico por classe (70/30), NUNCA train_test_split aleatório; rodadas numeradas ficam como evolução futura

## 3. Firmware
- [ ] app17-9 (Forma 1, PRONTO): janela fixa 100 amostras @ 100 Hz, 1 msg MQTT/janela, JSON com label + ts_epoch_ms
- [ ] app17-10 (Forma 2): raw ax,ay,az @ 100 Hz impresso na Serial (sem features, sem MQTT)
- [ ] Amostragem via millis(), sem delay()
- [ ] Debounce de botão no padrão do app17-7/8
- [ ] Forma 1: constantes WiFi/broker no topo, físico (broker da IoT-platform) vs Wokwi (Wokwi-GUEST + host.wokwi.internal). NUNCA test.mosquitto.org

## 4. Infra e scripts Python
- [ ] Infra = IoT-platform da disciplina (NÃO criar docker-compose.yml nem analytics/infra/)
- [ ] MQTT local (mosquitto:1883 / host.wokwi.internal); InfluxDB de destino na NUVEM, url/org/token/bucket como placeholders
- [ ] Forma 1 — fluxo Node-RED (flow_features_influx.json + node-red-contrib-influxdb): assina FIAPIoT/motor/features, label vira tag, grava em vibracao_features na nuvem; SEM botão no Node-RED; README cita Telegraf/script Python como alternativa
- [ ] Forma 2 — coletor_raw.py: linear, lê Serial (rfc2217 no Wokwi / COM no físico), gera CSV timestamp,ax,ay,az,label

## 5. Notebooks
- [ ] Células markdown explicando o conceito antes de cada código
- [ ] 1.3 (features/Influx) / 2.3 (raw/CSV): visualização normal vs anômala
- [ ] 2.4: janela a partir do raw recortado + split por coleta, com célula explicando o vazamento
- [ ] 1.5/2.5: StandardScaler + Random Forest (pequeno: n_estimators≈20, max_depth≈8) + export micromlgen (.hpp do modelo e do scaler)
- [ ] Métricas: accuracy, matriz de confusão, classification_report
- [ ] Feature importance nativa do RF + permutation_importance, com gráfico de barras (item de 25 pts da Sprint 4)
- [ ] Célula relacionando features importantes ao fenômeno físico
- [ ] Forma 1 NÃO promete visualizar raw (só features foram armazenadas); Forma 2 guarda o raw e fatia o fenômeno

## 6. Cobertura das Sprints (enunciado oficial em docs/Sprints_3_e_4.md)
- [ ] Sprint 3: coleta 100 Hz, ~6000 amostras/classe, CSV timestamp,ax,ay,az,label
- [ ] Sprint 3: visualização comparando classes
- [ ] Sprint 4: features por janela, dataset balanceado, mínimo 30 registros/classe
- [ ] Sprint 4: treino/teste separados, métricas, matriz de confusão
- [ ] Sprint 4: feature importance + interpretação física + limitações
- [ ] README raiz mapeia cada arquivo ao item da Sprint correspondente

## 7. Didática e limitações
- [ ] Código simples, sem classes customizadas nem abstrações
- [ ] Comentários em português
- [ ] READMEs explicam: edge perde o raw; timestamp Python é de recepção;
      Wokwi simula anomalia chacoalhando o MPU; tapas/chacoalho não substituem ensaio industrial
