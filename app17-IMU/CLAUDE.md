# Contexto do projeto

Resposta-modelo de professor (FIAP) para as Sprints 3 e 4 de IoT: vibração de motor com ESP32 + MPU6050(GY-521)/MPU6500, transmissão MQTT, armazenamento em InfluxDB, análise e ML no Google Colab.

Documentos de referência (ler antes de gerar qualquer código):
- `docs/Sprints_3_e_4.md` — enunciado oficial das Sprints
- `docs/Aula14_resumida.md` — conceitos: janelas, features, anotação, treino/teste

# CÓDIGO DE REFERÊNCIA — OBRIGATÓRIO SEGUIR

O diretório `app17-IMU/` contém os apps já dados em aula. TODO código novo deve seguir o estilo, a estrutura e as convenções desses apps. Antes de escrever qualquer firmware, leia:

- `app17-IMU/app17-8-MotorML/src/app17-8-MotorML.cpp` — **base canônica**: já implementa janela fixa (100 amostras @ 100 Hz), features, label por botão e publicação MQTT em JSON. Os novos firmwares são DERIVADOS dele, não reescritos do zero.
- `app17-IMU/app17-5-JanelaFeatures/` — funções de features (calcMean, calcStd, calcRMS, calcRMSMagnitude)
- `app17-IMU/app17-2-ColetaVisual/` — leitura simples ax,ay,az
- `app17-IMU/app17-0-Plot/` — padrão dos scripts Python (lineares, simples)

# Convenções do repositório (NÃO mudar)

1. **PlatformIO**, não Arduino IDE. Cada app é uma pasta `app17-N-Nome/` com:
   - `platformio.ini` (env:esp32, board esp32dev)
   - `src/app17-N-nome.cpp` (um único arquivo)
   - `diagram.json` + `wokwi.toml` (simulação Wokwi)
   - `README.md` e `.gitignore` (.pio, .vscode)
2. **Bibliotecas** (lib_deps): `okalachev/FlixPeriph`, `knolleary/PubSubClient`, `bblanchon/ArduinoJson`. Nada além disso no firmware.
3. **Sensor**: classe `MPU6500 mpu(Wire);` com comentário para trocar por `MPU6050` se for o sensor original. `setAccelRange(IMUInterface::ACCEL_RANGE_8G)`.
4. **Aceleração em m/s²** (FlixPeriph retorna m/s²; eixo Z em repouso ≈ 9,81). Manter a unidade em todos os códigos e comentários.
5. **Pinagem** (montagem mínima: só ESP32 + MPU6050 sobre a mesa, SEM motor):
   - SDA = 19, SCL = 18
   - BTN_COLETA = 26 (inicia/para a coleta), BTN_ANOMALIA = 25 (seleciona normal/anomalia, só com a coleta parada)
   - LED_PIN = 27 (aceso = coletando anomalia)
   - Sem pinos de motor (o app17-8 usava MOTOR_IN1=22 / MOTOR_IN2=23; removidos nos apps novos).
6. **Amostragem**: AMOSTRA_MS = 10 (100 Hz), via `millis()`, nunca `delay()`. **Timestamp**: sincronizar a hora pela Internet (NTP, UTC) no boot e enviar `ts_epoch_ms` = base NTP + `millis()` decorrido (sem RTC dedicado). Em sala, citar a importância da sincronização de relógio.
7. **Estilo**: funções livres simples, sem classes customizadas, sem abstrações, comentários em português, debounce de botão como no app17-7/app17-8.
8. **Labels** (mantidas por consistência com notebooks/checklist): `parado`, `ligado_normal`, `ligado_anomalia`. Sem motor, "ligado_*" = "coletando": `ligado_normal` = MPU6050 parado sobre a mesa; `ligado_anomalia` = **tapas na mesa** durante a coleta. `parado` = coleta desligada (não gera janela). Para o ML das Sprints, as classes são `ligado_normal` vs `ligado_anomalia`.
9. **MQTT**: PubSubClient + ArduinoJson, `setBufferSize(512)`, tópico base `FIAPIoT/motor/...`. 
   O broker é o da **IoT-platform** da disciplina (Mosquitto local, subido no WSL — ver "Stack de dados").
   - Físico: `MQTT_SERVER` = IP da máquina que roda a plataforma (placeholder comentado), porta 1883.
   - Wokwi (extensão no VS Code): WiFi `Wokwi-GUEST` (senha vazia) e broker `host.wokwi.internal:1883`, que alcança o Mosquitto local da plataforma (o WSL2 encaminha a porta 1883) — comentar a diferença no código.
   - **NÃO usar `test.mosquitto.org`** — o broker é sempre o local da IoT-platform.
10. **Numeração**: novos apps continuam a sequência (app17-9, app17-10, ...).

# Duas formas de coleta (o eixo da resposta-modelo)

A resposta-modelo ensina o mesmo problema (normal × anomalia) por dois caminhos complementares — o **contraste entre eles é o ponto didático**:

- **Forma 1 — Edge / janela fixa** (`app17-9-EdgeJanelaFixaInflux/`, **já pronto, NÃO alterar**): o ESP32 calcula as 7 features por **janela fixa de 100 amostras (1 s)** e publica 1 JSON/janela via MQTT; um **fluxo Node-RED** grava no InfluxDB (nuvem). Eficiente, mas **perde o sinal bruto** e a janela fixa pode cortar o fenômeno (pega só o início ou o fim).
- **Forma 2 — Raw / Serial→CSV** (`app17-10-RawSerialCSV/`): o ESP32 envia só `ax,ay,az` a 100 Hz pela **Serial** (streaming contínuo, **sem botões/LED/MQTT**); o script Python (`coletor_raw.py`) adiciona o **timestamp** e a **label** da rodada e grava **CSV**. Atende ao enunciado da Sprint 3 ao pé da letra e, principalmente, **guarda o raw** — o aluno re-janela no Colab e consegue **fatiar o dataset no momento exato** do fenômeno, em vez de depender de uma janela fixa que pode pegar a vibração pela metade.

As duas formas terminam no **mesmo ML**: **Random Forest + StandardScaler**, exportável para o ESP32 via **micromlgen** (AIoT embarcado, igual aos `app21`/`app22`) — não é uma rede neural. Ambas rodam no **Colab**: Forma 1 lê do **InfluxDB nuvem** (o Colab alcança a nuvem); Forma 2 lê do **CSV**.

# Stack de dados

- **Infra: a IoT-platform da disciplina** (`../IoT-platform/`, subida no WSL2 com `sudo ./start-linux.sh` — ver `IoT-platform/README.md`). Ela já sobe Mosquitto + Node-RED (+ n8n, Grafana, InfluxDB local) num único docker compose em `~/IoTStack`. **NÃO criar infra própria do projeto** — nada de `docker-compose.yml` nem de pasta `analytics/infra/`. O **MQTT** usado é o Mosquitto local da plataforma; o **InfluxDB de destino é na NUVEM** (InfluxDB Cloud), porque o Colab precisa alcançá-lo (não alcançaria o `localhost`).
- **Valores de conexão:** MQTT `localhost:1883` (físico) / `host.wokwi.internal:1883` (Wokwi); o Node-RED assina o Mosquitto local (`mosquitto:1883` na rede Docker). **InfluxDB Cloud:** `url`/`org`/`token`/`bucket` são **placeholders** (segredos do professor) — preencher no nó InfluxDB do Node-RED e nos notebooks 1.3/1.5. Measurement da Forma 1: `vibracao_features` (tag `label`). A Forma 2 **não usa InfluxDB** — gera CSV.
- **Forma 1 — MQTT → InfluxDB via Node-RED:** fluxo importável (`analytics/forma1_nodered/flow_features_influx.json`, modelo no `app16-Edge`), com `node-red-contrib-influxdb`. Assina `FIAPIoT/motor/features`, transforma `label`→tag e features→fields, grava em `vibracao_features` na nuvem. **Sem botão no Node-RED** — a condição vem do botão do próprio ESP32 (app17-9), no campo `label`. Telegraf seria a alternativa "produção"; um script Python (paho+influxdb-client) seria outra — o Node-RED foi escolhido por ser a ferramenta de integração da disciplina e deixar o fluxo visível.
- **Forma 2 — Coletor Serial → CSV:** script Python (pyserial), estilo `app17-0-Plot`. Lê a Serial, adiciona `timestamp` (hora do PC, na recepção) e a `label` informada ao iniciar (uma rodada = uma classe), e grava `timestamp,ax,ay,az,label` em CSV. No **Wokwi** não há COM real: a serial é exposta via `rfc2217ServerPort=4000` no `wokwi.toml` e o Python conecta em `rfc2217://localhost:4000`; no físico, abre a `COM` (com o Serial Monitor fechado).
- Colab: pandas, matplotlib, influxdb-client, scikit-learn, **micromlgen** (não keras/tensorflow). Notebooks com células markdown explicando o conceito antes de cada código. Ambas as formas rodam no Colab — Forma 1 conecta no InfluxDB nuvem; Forma 2 sobe o CSV.
- Timestamp: o firmware da Forma 1 (app17-9) envia `ts_epoch_ms` sincronizado por NTP (UTC), que vai como *field* de referência; o fluxo Node-RED usa o **horário de gravação no servidor** como tempo do ponto. Há latência de rede entre medir e gravar. Na Forma 2 o `timestamp` é o de recepção no PC.

# Janelas

- **Forma 1:** janela fixa de 100 amostras (1 s), calculada no **edge** — já existe no app17-8 e no app17-9.
- **Forma 2:** o janelamento é feito no **Colab**, sobre o raw do CSV. Como o aluno enxerga o sinal inteiro, ele pode **recortar o trecho exato** de cada condição antes de janelar — evitando janelas que pegam o fenômeno pela metade. Janelas **sem sobreposição** (não há app de janela deslizante na resposta-modelo).

# Feature importance (Sprint 4 — vale 25 pontos)

Todo notebook de ML deve incluir um gráfico de importância de features. Como o modelo é Random Forest, usamos a importância **nativa** (`feature_importances_`) **e** reforçamos com `sklearn.inspection.permutation_importance` no conjunto de teste, com célula markdown relacionando as features mais importantes ao comportamento físico (ex.: `std` e `rms_mag` sobem com a vibração; a média quase não muda).

# Regra de ouro do ML (Aula 14)

NUNCA usar `train_test_split` aleatório: janelas vizinhas no tempo são correlacionadas, então uma janela no treino "vaza" informação para uma janela quase idêntica no teste. Como hoje só há a tag `label` (sem rodadas numeradas), os notebooks fazem um **split cronológico por classe** (primeiras 70% das janelas → treino, últimas 30% → teste). Evolução ideal: coletar **rodadas numeradas** (`normal_01`, `normal_02`…) e separar por rodada:
- normal_01 + anomalia_01 → treino
- normal_02 + anomalia_02 → teste

# Estilo da resposta-modelo

- Cabeçalho comentado em cada arquivo: o que faz, item da Sprint que atende, como executar.
- README.md por pasta com ordem de execução.
- Tom: professor explicando para aluno. Direto, sem enrolação.
