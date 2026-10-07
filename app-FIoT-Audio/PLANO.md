# Plano — app-FIoT-Audio: do sinal de áudio ao assistente de voz (ESP32 + INMP441)

**Versão 4** — 16/09/2026. Consolida o trilho Android no plano principal: a wake word passa a
ser **"Ei Fioti"**, com o modelo do Edge Impulse **já treinado e validado**. O bloco de
observação encolhe de três etapas para duas, e as pastas são renumeradas.

---

## 0. O que mudou da versão 3 para a 4

| # | Antes (v3) | Agora (v4) | Por quê |
|---:|---|---|---|
| 1 | wake word **FIOT** + comando **EMERGÊNCIA**, projeto EI a gravar | wake word **"Ei Fioti"**, projeto `WakeWord-EiFIOT` **treinado** (93,1% val, F1 0,92) | o trilho Android já gravou, treinou e validou. Regravar custaria uma aula inteira para chegar no mesmo lugar |
| 2 | 4 classes (`fiot`·`emergencia`·`ruido`·`desconhecido`) | 3 classes (`WakeWord`·`desconhecido`·`ruido`) | é o que existe treinado |
| 3 | bloco 1 com 3 etapas (RAW · PALMA · VOZ_FEATURES) | bloco 1 com **2 etapas** (RAW · VOZ_FEATURES) | a turma é de IA; o `app-2-mic-PALMA` ensinava "a janela vira um número", que o `calcStd` da etapa seguinte já é. Ver §3 |
| 4 | EMERGÊNCIA na etapa 4 (roteiro da turma) | EMERGÊNCIA na **etapa 6**, a fazer, fora do roteiro | exige um projeto EI novo. Não bloqueia a aula |
| 5 | janela do impulse "em aberto, recomendo 1500 ms" | **1500 ms / stride 500 ms / 16 kHz**, fechado | validado em campo no A52 |
| 6 | trilho Android invisível no plano | `android-wakeword/` é parte do plano | é o que produziu o modelo |

A versão 3 continua no histórico do Git, com a trilha de 6 etapas e a wake word FIOT.

---

## 1. A aplicação

Assistente de voz. **"Ei Fioti"** acorda o dispositivo; com ele acordado, uma palavra de
comando dispara a ação.

```text
                    ┌──────────────────────────────┐
                    │          DORMINDO            │
                    │    só reage a "Ei Fioti"     │
                    └──────────────┬───────────────┘
                                   │  "Ei Fioti"
                                   ▼
                    ┌──────────────────────────────┐
      timeout 5 s   │           ATIVO              │
   ◄────────────────│      LED AZUL aceso          │
                    └──────────────┬───────────────┘
                                   │
              ┌────────────────────┼────────────────────┐
              ▼                    ▼                    ▼
          "ALERTA"          "LOCALIZAÇÃO"         "EMERGÊNCIA"
      motor de vibração       posição GPS         buzzer + MQTT
          (etapa 5)            (etapa 5)        (etapa 6 — a fazer)
```

**Por que a máquina de estados é conteúdo, e não detalhe de implementação:** o modelo é o mesmo
nos dois casos, mas as palavras de comando só valem enquanto o dispositivo está ATIVO. Isso
derruba o falso positivo sem tocar no modelo. É a lição de que **arquitetura vale tanto quanto
acurácia** — um assistente sempre armado dispara sozinho na conversa da sala.

### O modelo já existe

| | |
|---|---|
| Projeto | `WakeWord-EiFIOT` |
| Classes | `WakeWord` (131 janelas) · `desconhecido` (144) · `ruido` (158) |
| Impulse | Audio (MFCC) → Classification, janela **1500 ms**, stride **500 ms**, 16 kHz |
| Treino | 200 ciclos, data augmentation ligada |
| Resultado | **93,1%** validação, ROC 0,98 · Model testing 86,73%, F1 0,92 na wake word |
| Deployments | Android C++ float32 + EON (**feito**) · Arduino int8 (a fazer, etapa 3) |

A turma **não precisa gravar nada** para a aula acontecer. O roteiro de coleta continua
documentado, em [android-wakeword/ETAPA1-COLETA.md](android-wakeword/ETAPA1-COLETA.md), para
quem quiser refazer ou acrescentar classes.

---

## 2. Hardware

**Placa:** ESP32 DevKit v1 (`board = esp32dev`). O professor também roda num ESP32-S3 Super
Mini — mesmo código, pinos trocados pela flag `PLACA_S3`.

**Microfone:** INMP441 — MEMS I2S, 24 bits. Não se lê com `analogRead()`: o dado chega por
barramento serial síncrono (I2S), já digitalizado dentro do microfone.

| INMP441 | ESP32 | |
|---|---|---|
| VDD | 3V3 | não usar 5V |
| GND | GND | |
| L/R | GND | canal esquerdo (mono) |
| SCK (BCLK) | GPIO 26 | |
| WS (LRCL) | GPIO 25 | |
| SD (DOUT) | GPIO 33 | dados → ESP32 |

As etapas 1 e 2 usam **só o microfone** — são observação pura. Atuadores a partir da etapa 3:

| Atuador / periférico | GPIO | Etapa | Observação |
|---|---|---|---|
| LED **azul** — dispositivo ATIVO | 21 | 3, 4, 5, 6 | com resistor de 220 Ω |
| Motor de vibração 3 V — ALERTA | 18 | 5 | **nunca direto no GPIO**: transistor NPN (BC337/2N2222) + diodo de roda livre (1N4148) |
| GPS NEO-6M — LOCALIZAÇÃO | RX2 16 / TX2 17 | 5 | UART2 a 9600 baud, `TinyGPSPlus` |
| Buzzer — EMERGÊNCIA | 19 | 6 | buzzer **ativo** vai direto (~25 mA); se for passivo, `ledcWriteTone()` |
| LED onboard — status Wi-Fi/MQTT | 2 | 4, 5, 6 | |

Sem conflito com o I2S (25, 26, 33).

---

## 3. Por que o bloco 1 encolheu para duas etapas

A v3 tinha três etapas de observação: onda crua → RMS da janela → quatro features. A do meio
(`app-2-mic-PALMA`) saiu.

**O argumento:** o `calcStd` da etapa de features **é** o RMS da etapa da palma — depois de
subtrair a média, desvio-padrão e RMS são a mesma conta. A etapa intermediária gastava uma
rodada de upload, Teleplot e explicação para ensinar uma linha que a etapa seguinte contém.

O que a PALMA ensinava e **continua sendo ensinado**, agora dentro do
[app-2-mic-VOZ_FEATURES](app-2-mic-VOZ_FEATURES/):

| Lição da PALMA | Onde foi parar |
|---|---|
| a onda oscila em torno de zero; a média é sempre ~0 | seção "`desvio` é o RMS" do README |
| por que elevar ao quadrado antes de somar | mesma seção |
| palma e voz alta sobem igual → um número diz *quão alto*, nunca *o quê* | teste 1 do README |

**O que se perdeu, conscientemente:** o degrau "um número" → "quatro números" deixa de ser um
upload separado e vira uma leitura de tabela. Em troca, a turma chega no Edge Impulse **uma
etapa mais cedo** — que é o objetivo declarado da trilha, já que o bloco 1 é rampa, não
conteúdo.

Os dois `.txt` históricos do 1º ano que moravam na pasta (`mic-palma-ky038`,
`mic-voz-max9814-ANALOGICO`) foram para `app-1-mic-RAW/src/`, junto com os outros três.

---

## 4. A trilha

A fronteira entre os blocos importa: **só depois do Edge Impulse o dispositivo aciona alguma
coisa.**

**Bloco 1 — observar o sinal (PlatformIO).** Nada é acionado; o aluno olha o Teleplot.

| # | Pasta | O que o aluno vê | Conceito novo |
|---|---|---|---|
| 1 | `app-1-mic-RAW` | a onda da voz reagindo ao vivo | o microfone entrega número pronto |
| 2 | `app-2-mic-VOZ_FEATURES` | 16 000 números/s viram 4 | janela, RMS, `calcStd`, `calcZCR` |

**Bloco 2 — o modelo (Edge Impulse).** Já treinado; a etapa é escolher o Deployment.

**Bloco 3 — inferir e agir (de volta ao PlatformIO).**

| # | Pasta | O que o aluno vê | Conceito novo |
|---|---|---|---|
| 3 | `app-3-mic-FIOT` | "Ei Fioti" acende o LED azul | MFCC, janela deslizante, **máquina de estados** |
| 4 | `app-4-mic-FIOT-MQTT` | a detecção chega no Node-RED | AIoT: a borda decide, a nuvem registra |
| 5 | `app-5-mic-ASSISTENTE` *(extra)* | + ALERTA (vibra) e LOCALIZAÇÃO (GPS) | multiclasse, atuadores com transistor, fusão com GPS |
| 6 | `app-6-mic-EMERGENCIA` *(a fazer)* | o comando EMERGÊNCIA | projeto EI novo, 4 classes |

### Etapa 1 — `app-1-mic-RAW`: ver o som

Init I2S (API legada), leitura em blocos, `GANHO` exposto no `INMP441.hpp` para o aluno ver o
efeito e ver o sinal **saturar** quando exagera. Offset DC removido pela média do bloco.
Teleplot `>som:` decimado 1 a cada 8. Exercício: silêncio · falar · assobiar · palma.

Os cinco `.txt` da pasta são o material analógico do 1º ano (KY-038, MAX9814): leitura crua,
comparador, detector de palma com limiar e a `calibrarCentro()`. Fora do build, servem de
contraste com o I2S.

### Etapa 2 — `app-2-mic-VOZ_FEATURES`: descrever a janela

Janela de 4096 amostras (256 ms), sem sobreposição. Quatro features, uma função cada, com os
mesmos nomes do **app17** (acelerômetro): `calcMean`, `calcStd`, `calcPtP`, `calcZCR`.

| Feature | O que captura |
|---|---|
| `media` | offset DC (≈0 após a correção) — é o slide "RMS vs média" da Aula 13 na veia |
| `desvio` | energia / volume — **é o RMS** |
| `p2p` | pico a pico — transientes |
| `zcr` | cruzamentos por zero — grave × agudo, **sem FFT** |

Fecho obrigatório: **"Ei Fioti" e qualquer outra palavra no mesmo volume têm as mesmas quatro
features.** Tocada de trás para frente, a wake word dá os mesmos números. A informação está em
*quais frequências, em qual ordem*. É a deixa para o Edge Impulse.

### Etapa 3 — `app-3-mic-FIOT`: o dispositivo acorda

1. Studio → projeto `WakeWord-EiFIOT` → **Deployment → Arduino library → Quantized (int8)**.
2. `lib/WakeWord-EiFIOT_inferencing/` (convenção do `app33-MagicWand`).
3. Firmware a partir do `esp32_microphone_continuous` da própria biblioteca, adaptado aos pinos
   do INMP441, **mais a máquina de estados** DORMINDO/ATIVO.
4. `board_build.partitions = huge_app.csv`.
5. No topo do arquivo: `LIMIAR_CONFIANCA` (**0.85**, o valor medido no A52), `TIMEOUT_ATIVO_MS`
   (5000).
6. Se o int8 não couber na RAM: baixar a janela de 1500 para 1000 ms e retreinar. "Ei Fioti"
   tem 3 sílabas e cabe em 1 s — ao contrário de EMERGÊNCIA.

O `ww_classify` do `wakeword_ffi.cpp` (lado Android) serve de referência: a chamada ao
`run_classifier` é idêntica, muda só o porting.

### Etapa 4 — `app-4-mic-FIOT-MQTT`: a detecção sai do dispositivo

Etapa 3 + `PubSubClient` + `ArduinoJson`. Publica em `FIAPIoT/audio/wakeword`:

```json
{"device":"IoTDevAudio001","label":"WakeWord","score":0.94,"ts_epoch_ms":1749760205123}
```

LED onboard aceso = conectado ao broker. Fecha o arco com o app06/app07 e o Node-RED da turma.

### Etapa 5 — `app-5-mic-ASSISTENTE` (extra, demonstração do professor)

Acrescenta ALERTA (motor de vibração via transistor) e LOCALIZAÇÃO (NEO-6M em UART2,
`TinyGPSPlus`, publica lat/lon no MQTT). Exige as classes correspondentes no projeto EI.

### Etapa 6 — `app-6-mic-EMERGENCIA` (a fazer)

Projeto EI **novo**, não retreino do atual: `WakeWord` · `emergencia` · `desconhecido` ·
`ruido`. Deixar o `WakeWord-EiFIOT` intacto mantém as etapas 3 a 5 funcionando enquanto a
coleta nova acontece. Detalhes em
[app-6-mic-EMERGENCIA/README.md](app-6-mic-EMERGENCIA/README.md).

---

## 5. O trilho Android

O mesmo impulse rodando no celular, e o lado que **está funcionando hoje**.

| | |
|---|---|
| Documentação | [android-wakeword/](android-wakeword/) — plano, auditoria, coleta, aula |
| Implementação | `FIAP-IoT-eval`, em `app40-WakeWord/app40-1-SmartphoneAndroid` |
| Stack | Flutter + Dart FFI → `libwakeword.so` (SDK C++ do Edge Impulse) |
| Deployment | Android library C++, **Unoptimized (float32)** + EON |
| Inferência | **2 a 4 ms** no Galaxy A52 |
| Extra | ao detectar, captura 5 s do acelerômetro e volta |

**Por que os dois repositórios.** A documentação fica no `develop`, junto com o resto do
material de aula; o código do app fica no `eval`, com os outros apps entregáveis. O SDK do Edge
Impulse (29 MB, ~1400 arquivos) não é versionado — o `.zip` é a fonte, e um `Expand-Archive` o
recria antes do build.

**A pendência conhecida:** o limiar que serve numa sala fechada (0,90) não é o que serve num
ambiente barulhento (0,79), porque `WakeWord` foi gravado quase todo em ambiente controlado. O
padrão fixado é 0,85. A correção é dado, não parâmetro: ~30 amostras com ruído real e
retreinar. Isso vale para o ESP32 também.

---

## 6. Slides

### 6.1 O que já existe e serve para áudio sem reescrever

A maior parte da teoria de áudio já está nos slides de IMU — só nunca foi apontada para o áudio.

| Conceito de áudio | Já existe em | Ajuste |
|---|---|---|
| áudio é sinal temporal | **A12 s6** — já lista "Áudio" | nenhum, só citar |
| por que 16 kHz | **A12 s25** — Nyquist | voz até ~8 kHz. Exemplo perfeito do teorema |
| ganho e saturação | **A12 s30–33** — range 2G–16G | o `GANHO` do INMP441 é o mesmo raciocínio |
| raw não cabe na rede | **A13 s3–5** — 100 Hz já derruba o Node-RED | áudio é 160× pior |
| **RMS × média** | **A13 s14** — `[-3,+3,-3,+3,-3]` | áudio é o caso canônico: a onda **sempre** oscila em torno de zero |
| pico e pico a pico | **A13 s15–16** | a palma é o "impacto isolado", em áudio |
| zero crossing | **A14 s23** — já está na lista | merece virar slide: separa "sss" de "ahh" |
| janela e **efeito de borda** | **A14 s18–22** | a wake word é o "evento curto" do s19 — justifica o stride |
| **FFT: o próximo nível** | **A14 s26** — "desafio extra, não obrigatório" | em áudio deixa de ser opcional. **É a dobradiça** |
| label vem do experimento | **A14 s28** | o labeling no Studio é literalmente isso |
| protocolo de coleta | **A14 s30** — descartar a transição | vira o protocolo de gravação |
| duas formas de anotar | **A14 s29** | ganha uma terceira coluna: anotar **na plataforma** |

**Lacuna encontrada:** a agenda da **Aula 13 (slide 2)** já promete "Observação com dados de
acelerômetro **e áudio**" — e os 28 slides não têm uma linha de áudio.

### 6.2 Teoria genuinamente nova (6 conceitos)

1. **Microfone digital**: o sensor entrega o número pronto, por I2S. 24 bits, offset DC, ganho
   por deslocamento de bits.
2. **Outra ordem de grandeza**: 100 Hz → 16 000 Hz.
3. **Por que estatística de janela não distingue palavras**: as quatro features empatam, e a
   wake word tocada de trás para frente dá os mesmos números.
4. **Espectrograma e MFCC em um slide**: fatias de ~25 ms → energia por faixa → imagem
   tempo × frequência → a rede olha a imagem.
5. **Janela deslizante + máquina de estados**: stride, falso positivo × falso negativo, limiar
   de confiança, e por que a wake word existe.
6. **Descasamento de domínio**: treinar no celular, inferir no INMP441 — com o número medido
   (0,90 em sala × 0,79 no barulho).

### 6.3 O deck novo

Arquivo alvo:
`C:\Users\noris\OneDrive\Aulas\FIAP\FIAP-Slides-CPs\IoT\IoT - Aula 25.1 - EI - Áudio.pptx`

A especificação slide a slide está em [docs/slides-aula-25.1.md](docs/slides-aula-25.1.md).

Segue o `INSTRUÇÕES PARA POWERPOINT.md`: bloco fixo + uma seção por vez, âncora pelo título da
divisória, máx. 5 marcadores, elemento visual em **todo** slide feito com formas do PowerPoint,
notas do apresentador em todos. Código de cores da trilha, com uma convenção nova para áudio:
**AZUL** = onda crua vinda do microfone · **LARANJA** = qualquer feature calculada (RMS, ZCR,
MFCC) · **VERDE/VERMELHO** = detectado / não detectado.

O deck do trilho Android é separado:
[android-wakeword/AULA-WakeWord.md](android-wakeword/AULA-WakeWord.md).

### 6.4 Patches nos decks existentes (depois do deck novo)

- **Aula 13** — 3 slides de áudio depois do slide 16, cumprindo a agenda que a própria aula
  promete.
- **Aula 14** — 2 slides: o ZCR sai da lista do s23 e vira slide; o s26 (FFT) vira a dobradiça
  para o MFCC.

---

## 7. Estrutura final

```text
app-FIoT-Audio/
├── README.md                          índice da trilha
├── CLAUDE.md                          convenções deste diretório
├── PLANO.md                           este arquivo
├── docs/
│   ├── coleta-edge-impulse.md         exportar para o ESP32; gravar, se precisar
│   └── slides-aula-25.1.md            especificação dos slides (formato do professor)
├── android-wakeword/                  docs do trilho Android (código no FIAP-IoT-eval)
├── app-0-mic-SOM/                     apoio, fora do roteiro
├── app-1-mic-RAW/
├── app-2-mic-VOZ_FEATURES/
├── app-3-mic-FIOT/
├── app-4-mic-FIOT-MQTT/
├── app-5-mic-ASSISTENTE/
└── app-6-mic-EMERGENCIA/
```

Cada `app-N-*/`: `platformio.ini` (cabeçalho comentado) · `README.md` (runbook curto, padrão
app24) · `src/*.cpp` (+ `.txt` das variantes) · `.gitignore`. **Sem** `diagram.json` e **sem**
`wokwi.toml` — o Wokwi não tem peça INMP441.

---

## 8. Ordem de execução

| Fase | Entrega | Status |
|---|---|---|
| A | `CLAUDE.md`, `README.md` índice, limpeza dos herdados, `platformio.ini` | **feito** |
| B | Etapa 1 (I2S + Teleplot) | **feito** — `pio run` ok |
| C | Etapa 2 (features + `INMP441.hpp`) | **feito** — `pio run` ok |
| D | Trilho Android: coleta, treino, app Flutter | **feito** — reconhece bem no A52 |
| E | Consolidação: renumeração, wake word "Ei Fioti", docs | **feito** (v4) |
| F | `docs/slides-aula-25.1.md` | **feito** — falta capturar os prints do Studio |
| G | Etapa 3 (deployment int8 + máquina de estados) | a fazer |
| H | Etapa 4 (MQTT) | a fazer |
| I | Etapa 5 (assistente estendido, extra) | a fazer |
| J | Etapa 6 (EMERGÊNCIA, projeto EI novo) | a fazer |
| K | APK release assinado do Android | a fazer |

Falta validar na bancada o que só o hardware responde: o `GANHO` (hoje em 13) e se o canal é
`ONLY_LEFT` ou `ONLY_RIGHT` neste módulo INMP441.

---

## 9. Decisões tomadas

| # | Decisão |
|---:|---|
| 1 | Wake word **"Ei Fioti"**, modelo `WakeWord-EiFIOT` já treinado, reaproveitado nos dois alvos |
| 2 | Janela **1500 ms**, stride **500 ms**, 16 kHz — fechado, validado em campo |
| 3 | Limiar de confiança padrão **0.85** (0,90 em sala fechada, 0,79 em ambiente barulhento) |
| 4 | Deployment **int8** no ESP32, **float32** no Android |
| 5 | Bloco 1 com duas etapas: `app-2-mic-PALMA` removido, sua lição absorvida pelo `calcStd` |
| 6 | Pastas renumeradas; `app-0-mic-SOM` mantido como apoio, fora do roteiro |
| 7 | **EMERGÊNCIA** vira a etapa 6, com projeto EI novo, depois do assistente |
| 8 | Atuadores: LED azul GPIO 21, motor GPIO 18 (com transistor), GPS UART2 16/17, buzzer GPIO 19 |
| 9 | MQTT na etapa 4, notificando a detecção |
| 10 | Docs do Android no `develop`, código no `eval` — não juntar |
| 11 | `diagram.json` e `wokwi.toml` apagados de todas as etapas |

**Em aberto:** nada bloqueando. As pendências são de execução (fases G a K), não de decisão.
