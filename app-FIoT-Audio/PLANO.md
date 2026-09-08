# Plano — app-FIoT-Audio: do sinal de áudio ao assistente de voz (ESP32 + INMP441)

**Versão 3** — decisões de 04/09/2026 fechadas. Coleta pelo celular no Edge Impulse;
wake word **FIOT** + comando **EMERGÊNCIA**; versão estendida do professor com ALERTA e
LOCALIZAÇÃO.

---

## 0. O que já existe (código lido)

Os sete arquivos de `src/` não são lixo — são uma trilha didática já pensada, construída
sobre o microfone errado. O que aproveito:

| Arquivo | O que faz hoje | Destino |
|---|---|---|
| `app-1/.../comparador-mic.ino` | `analogRead` em dois pinos (KY-038 no 34, MAX9814 no 35), Teleplot | **ideia mantida**: vira comparação analógico × I2S |
| `app-1/.../mic-raw-ky038.txt` · `mic-raw-max9814.txt` | `analogRead` puro, `delay(2)` | material histórico |
| `app-2/.../mic-palma-ky038.txt` | `LIMIAR 40` + `LED_DURACAO_MS 500` com `millis()` | **estrutura reaproveitada inteira** na etapa 2 |
| `app-2/.../mic-voz-max9814.ino` | `calibrarCentro()` com 500 amostras → `abs(raw - centro)` | **conceito central**: no INMP441 vira remoção do offset DC |
| `app-3/.../FEATURES-AMPLITUDE.txt` | janela `JANELA_MS 50`, pico da janela | **estrutura reaproveitada** na etapa 3 |
| `app-3/.../FEATURES-RMS.ino` | mesma janela, soma dos quadrados → RMS | **estrutura reaproveitada** na etapa 3 |

Convenções tiradas do código e mantidas:

1. **Um arquivo compilável por pasta; variantes parqueadas como `.txt`.** É como ter várias
   versões didáticas sem quebrar o build. Nos apps novos o compilável é `.cpp` (padrão do
   `app17-7` e do `app24/device`).
2. **Configuração em `#define`/`const` no topo do arquivo.**
3. **Saída Teleplot** (`>nome:valor`) para gráfico ao vivo.

Descartado: os `platformio.ini` (idênticos e vazios), os `diagram.json`/`wokwi.toml`
(descrevem o app de irrigação) e os três `README.md` (a mesma cópia de 436 linhas sobre
TensorFlow Lite e umidade do solo).

---

## 1. A aplicação

Assistente de voz de emergência. **FIOT** acorda o dispositivo; com ele acordado, uma palavra
de comando dispara a ação.

```text
                    ┌──────────────────────────────┐
                    │          DORMINDO            │
                    │   só reage à palavra FIOT    │
                    └──────────────┬───────────────┘
                                   │  "FIOT"
                                   ▼
                    ┌──────────────────────────────┐
      timeout 5 s   │           ATIVO              │
   ◄────────────────│      LED AZUL aceso          │
                    └──────────────┬───────────────┘
                                   │
              ┌────────────────────┼────────────────────┐
              ▼                    ▼                    ▼
       "EMERGÊNCIA"           "ALERTA"           "LOCALIZAÇÃO"
        buzzer + MQTT      motor de vibração        posição GPS
         (etapas 4-5)          (etapa 6)            (etapa 6)
```

**Por que a máquina de estados é conteúdo, e não detalhe de implementação:** o modelo é o
mesmo nos dois casos, mas as palavras de comando só valem enquanto o dispositivo está ATIVO.
Isso derruba o falso positivo sem tocar no modelo. É a lição de que **arquitetura vale tanto
quanto acurácia** — um assistente sempre armado dispara sozinho na conversa da sala.

### Divisão em duas versões

| | Palavras | Modelo | Quem faz |
|---|---|---|---|
| **Versão da turma** (etapas 4 e 5) | `fiot` · `emergencia` · `ruido` · `desconhecido` | projeto EI #1 | o aluno grava e treina |
| **Versão estendida** (etapa 6) | + `alerta` · `localizacao` | projeto EI #2 | só o professor, para demonstrar |

Dois projetos no Edge Impulse, não um. O aluno grava 2 palavras em vez de 4 e treina mais
rápido; a versão estendida é "mais dados, mesmo impulse", e não obriga ninguém a retreinar.

### ⚠ O ponto que precisa ser decidido antes de você gravar

A janela padrão de *keyword spotting* do Edge Impulse é **1000 ms**. Isso serve para "FIOT"
(2 sílabas), mas **"EMERGÊNCIA" e "LOCALIZAÇÃO" têm 5 sílabas** e, faladas em ritmo normal,
passam de 1 segundo. Numa janela de 1000 ms a palavra entra cortada, e o modelo aprende
pedaço de palavra.

**Recomendo janela de 1500 ms com stride de 500 ms**, e o mesmo 1500 ms no *Split sample* na
hora de cortar as gravações. Custa mais RAM e flash (a entrada do MFCC cresce ~50%), o que o
ESP32 absorve bem, e evita ter que regravar tudo depois. Se o EON Tuner mostrar que 1000 ms
resolve, dá para reduzir — o caminho contrário obriga a regravar.

---

## 2. Hardware

**Placa:** ESP32 DevKit v1 (`board = esp32dev`).

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

As etapas 1 a 3 usam **só o microfone** — são observação pura. Atuadores a partir da etapa 4:

| Atuador / periférico | GPIO | Etapa | Observação |
|---|---|---|---|
| LED **azul** — dispositivo ATIVO | 21 | 4, 5, 6 | com resistor de 220 Ω |
| Buzzer — EMERGÊNCIA | 19 | 4, 5, 6 | buzzer **ativo** vai direto (~25 mA); se for passivo, `ledcWriteTone()` |
| Motor de vibração 3 V — ALERTA | 18 | 6 | **nunca direto no GPIO**: transistor NPN (BC337/2N2222) + diodo de roda livre (1N4148) |
| GPS NEO-6M — LOCALIZAÇÃO | RX2 16 / TX2 17 | 6 | UART2 a 9600 baud, `TinyGPSPlus` |
| LED onboard — status Wi-Fi/MQTT | 2 | 5, 6 | |

Sem conflito com o I2S (25, 26, 33). GPIO 34/35 dos mics analógicos antigos ficam livres para
a comparação da etapa 1.

---

## 3. Restrições técnicas confirmadas nesta máquina

| Item | Situação | Consequência |
|---|---|---|
| `platform = espressif32@6.12.0` | → Arduino core **2.0.17** (IDF 4.4), pacote verificado | usar a API I2S **legada** `#include <driver/i2s.h>`. **Não** usar `ESP_I2S.h` nem `driver/i2s_std.h` — são do core 3.x. É a armadilha nº 1 dos tutoriais de INMP441 |
| `driver/i2s.h` no core instalado | presente | compila sem lib externa |
| PlatformIO Core 6.1.19 | instalado | dá para validar `pio run` em todas as etapas |
| Wokwi | **não tem peça INMP441** | `diagram.json` e `wokwi.toml` saem de todas as etapas |
| Partição | biblioteca do EI é grande | `board_build.partitions = huge_app.csv` nas etapas 4–6 |

**Vazão da serial.** 16 kHz × 16 bits = 32 kB/s; a 115200 baud cabem ~11 kB/s de texto. Não dá
para imprimir todas as amostras: a etapa 1 decima (1 a cada 32 → ~500 Hz no Teleplot) e as
etapas 2–3 imprimem só o resultado da janela. Isso é matéria, não bug.

---

## 4. A trilha

A trilha tem três blocos, e a fronteira entre eles importa: **só depois do Edge Impulse o
dispositivo passa a acionar alguma coisa.**

**Bloco 1 — observar o sinal (PlatformIO).** Nada é acionado; o aluno olha o Teleplot.

| # | Pasta | O que o aluno vê | Conceito novo |
|---|---|---|---|
| 0 | `app-0-mic-SOM` | sai número do microfone — mas picotado | microfone digital, escravo I2S, `<I2S.h>` |
| 1 | `app-1-mic-RAW` | a onda inteira, sem buracos | leitura em bloco, DMA, ganho |
| 2 | `app-2-mic-PALMA` | 16 000 números/s viram 31 | janela, média, RMS |
| 3 | `app-3-mic-VOZ_FEATURES` | 4096 amostras viram 4 números | `calcMean`, `calcStd`, `calcPtP`, `calcZCR` |

Cada etapa existe porque a anterior bateu num limite **visível na tela**: uma amostra por vez
não acompanha 16 kHz → ler em bloco; a onda inteira não cabe numa decisão → resumir a janela;
um número diz *quão alto* e nunca *o quê* → quatro features; as features não distinguem FIOT de
EMERGÊNCIA → Edge Impulse.

**Bloco 2 — treinar (Edge Impulse).** Coleta pelo celular, rótulo, split, MFCC, treino, export.

**Bloco 3 — inferir e agir (de volta ao PlatformIO).**

| # | Pasta | O que o aluno vê | Conceito novo |
|---|---|---|---|
| 4 | `app-4-mic-FIOT` | **FIOT** acende o LED azul; **EMERGÊNCIA** toca o buzzer | MFCC, janela deslizante, **máquina de estados** |
| 5 | `app-5-mic-FIOT-MQTT` | a emergência chega no Node-RED | AIoT: a borda decide, a nuvem registra |
| 6 | `app-6-mic-ASSISTENTE` *(extra do professor)* | + ALERTA (vibra) e LOCALIZAÇÃO (GPS) | multiclasse, atuadores com transistor, fusão com GPS |

### Etapa 1 — `app-1-mic-RAW`: ver o som

Init I2S (API legada), leitura em blocos, `#define GANHO_SHIFT` (partida em 11) exposto para o
aluno ver o efeito do ganho — e ver o sinal **saturar** quando exagera. Offset DC removido pela
média do bloco, herdando o `calibrarCentro()` do código atual. Teleplot `>amostra:` decimado +
`>env:` (pico do bloco). O comparador atual vira `.txt`: analógico × I2S lado a lado.

Exercício: assobiar · falar · palma · silêncio.

### Etapa 2 — `app-2-mic-PALMA`: a janela resume o som

Janela de 32 ms (512 amostras) → RMS, plotado ao vivo. **Não aciona nada**: a `REFERENCIA`
(calibrada no silêncio da sala nos 2 primeiros segundos) é só uma linha no gráfico mostrando
onde uma decisão *poderia* ser tomada.

O que observar: silêncio plano · palma como pico estreito e altíssimo · voz normal ondulando
com as sílabas · voz alta subindo **tanto quanto** a palma.

Fecho: a palma passa da referência, mas a voz alta também. Um número diz **quão alto**, nunca
**o quê**.

### Etapa 3 — `app-3-mic-VOZ_FEATURES`: descrever a janela

Janela de 250 ms, 50% de sobreposição (~8 janelas/s):

| Feature | O que captura |
|---|---|
| `mean` | offset DC (≈0 após a correção) — é o slide "RMS vs média" da Aula 13 na veia |
| `std` | energia / volume |
| `p2p` | pico a pico — transientes |
| `rms` | energia efetiva |
| `max_abs` | maior amplitude absoluta |
| `zcr` | cruzamentos por zero — grave × agudo, **sem FFT** |

Saída dupla: CSV para gravar + Teleplot para ver.

Fecho obrigatório: **"FIOT" e "EMERGÊNCIA" têm as mesmas 6 features.** A informação está em
*quais frequências, em qual ordem*. É a deixa para o Edge Impulse.

### Etapa 4 — `app-4-mic-FIOT`: o dispositivo acorda

1. Impulse: **Audio (MFCC) → Classification (1D Conv)**, janela **1500 ms**, stride 500 ms,
   16 kHz. Classes: `fiot`, `emergencia`, `ruido`, `desconhecido`.
2. Deploy → **Arduino library** → `lib/<Nome>_inferencing/` (convenção do `app33-MagicWand`).
3. Firmware a partir do `esp32_microphone_continuous` da própria biblioteca, adaptado aos pinos
   do INMP441, **mais a máquina de estados** DORMINDO/ATIVO.
4. `board_build.partitions = huge_app.csv`.
5. No topo do arquivo: `LIMIAR_CONFIANCA` (0.7), `TIMEOUT_ATIVO_MS` (5000), `BUZZER_MS`.

### Etapa 5 — `app-5-mic-FIOT-MQTT`: a emergência sai do dispositivo

Etapa 4 + `PubSubClient` + `ArduinoJson`. Publica em `FIAPIoT/audio/emergencia`:

```json
{"device":"IoTDevAudio001","label":"emergencia","score":0.94,"ts_epoch_ms":1749760205123}
```

LED onboard aceso = conectado ao broker. Fecha o arco com o app06/app07 e o Node-RED da turma.

### Etapa 6 — `app-6-mic-ASSISTENTE` (extra, demonstração do professor)

Projeto EI #2 com 6 classes. Acrescenta ALERTA (motor de vibração via transistor) e
LOCALIZAÇÃO (NEO-6M em UART2, `TinyGPSPlus`, publica lat/lon no MQTT). Não entra no roteiro
da turma — é o "olha onde isso chega".

---

## 5. Estrutura final

```text
app-FIoT-Audio/
├── README.md                          índice da trilha
├── CLAUDE.md                          convenções deste diretório
├── PLANO.md                           este arquivo
├── docs/
│   ├── coleta-edge-impulse.md         coleta pelo celular, passo a passo
│   └── slides-aula-25.1.md            especificação dos slides (formato do professor)
├── app-1-mic-RAW/
├── app-2-mic-PALMA/
├── app-3-mic-VOZ_FEATURES/
├── app-4-mic-FIOT/
├── app-5-mic-FIOT-MQTT/
└── app-6-mic-ASSISTENTE/
```

Cada `app-N-*/`: `platformio.ini` (cabeçalho comentado) · `README.md` (runbook curto, padrão
app24) · `src/*.cpp` (+ `.txt` das variantes) · `.gitignore`. **Sem** `diagram.json` e **sem**
`wokwi.toml`.

---

## 6. Coleta no Edge Impulse (pelo celular)

1. Studio → **Data acquisition** → **Show QR code** → o celular vira sensor, sem instalar nada.
2. Gravar **uma faixa longa por palavra**, repetindo com pausas (ex.: 60 s, ~25 repetições).
3. Na amostra → **Split sample**, segmento de **1500 ms** → o Studio corta sozinho em torno de
   cada evento.
4. Repetir para `ruido` (silêncio e conversa da sala) e `desconhecido` (outras palavras).
5. **Train/Test split** automático (80/20) no Studio.

O Studio mostra **forma de onda e espectrograma** de cada amostra — boa parte do "visualizar o
sinal" sai de graça na plataforma. O ESP32 fica com o que ela não dá: o sinal cru, ao vivo,
com as mãos do aluno.

**O risco, dito em aula:** treinar com o microfone do celular e inferir com o INMP441 é
**descasamento de domínio** — resposta em frequência, ganho e distância diferentes. Costuma
funcionar para uma wake word de sala, e é exatamente a lição da Aula 14 ("o dado de treino
precisa parecer o dado de operação"). Mitigações em ordem de custo: gravar a ~30 cm, caprichar
em `ruido` e `desconhecido`, ligar o *augmentation*; e só se decepcionar, gravar ~10 amostras
com o próprio INMP441.

---

## 7. Slides

### 7.1 O que já existe e serve para áudio sem reescrever

A maior parte da teoria de áudio já está nos slides de IMU — só nunca foi apontada para o áudio.

| Conceito de áudio | Já existe em | Ajuste |
|---|---|---|
| áudio é sinal temporal | **A12 s6** — já lista "Áudio" | nenhum, só citar |
| por que 16 kHz | **A12 s25** — Nyquist | voz até ~8 kHz. Exemplo perfeito do teorema |
| ganho e saturação | **A12 s30–33** — range 2G–16G | o `GANHO_SHIFT` é o mesmo raciocínio |
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

### 7.2 Teoria genuinamente nova (6 conceitos)

1. **Microfone digital**: o sensor entrega o número pronto, por I2S. 24 bits, offset DC, ganho
   por deslocamento de bits.
2. **Outra ordem de grandeza**: 100 Hz → 16 000 Hz.
3. **Por que estatística de janela não distingue palavras**: FIOT e EMERGÊNCIA empatam nas 6
   features.
4. **Espectrograma e MFCC em um slide**: fatias de ~25 ms → energia por faixa → imagem
   tempo × frequência → a rede olha a imagem.
5. **Janela deslizante + máquina de estados**: stride, falso positivo × falso negativo, limiar
   de confiança, e por que FIOT existe.
6. **Descasamento de domínio**: treinar no celular, inferir no INMP441.

### 7.3 O deck novo

Arquivo alvo (já criado):
`C:\Users\noris\OneDrive\Aulas\FIAP\FIAP-Slides-CPs\IoT\IoT - Aula 25.1 - EI - Áudio.pptx`

Estado atual — **8 slides**, esqueleto herdado do Magic Wand:

| # | Slide | Situação |
|---:|---|---|
| 1 | capa | manter |
| 2 | capa da disciplina | manter |
| 3 | Agenda — ainda diz "Aplicação: **Varinha mágica**" | **corrigir** |
| 4 | divisória **Áudio** | âncora |
| 5 | (Far/Extreme) Edge AI — reconhecimento de voz | manter |
| 6 | Materiais necessários (ESP32, "sensor para captação de áudio") | **detalhar**: INMP441, LED azul, buzzer |
| 7 | divisória **HANDS ON!** | âncora |
| 8 | encerramento | manter |

Os slides novos entram **entre o slide 6 e a divisória HANDS ON!**, em 4 seções:

| Seção | Título | Slides |
|---:|---|---:|
| 1 | O microfone e o sinal | 5 |
| 2 | Da janela à palavra | 4 |
| 3 | Coletar e treinar no Edge Impulse | 4 |
| 4 | FIOT: o dispositivo acorda | 4 |

Mais os ajustes nos slides 3 e 6. Resultado previsto: **27 slides**.

Esboço, para aprovar ou cortar:

| Seção | Slide | Visual |
|---|---|---|
| 1.1 | O microfone não entende palavras | espelho do "o sensor não entende vibração" (A14 s4) |
| 1.2 | Analógico × digital: KY-038 e INMP441 | duas colunas, `analogRead` × I2S |
| 1.3 | 100 Hz e 16 000 Hz | duas linhas do tempo na mesma escala |
| 1.4 | Nyquist na voz | régua de frequência, voz até ~8 kHz → 16 kHz |
| 1.5 | Ganho: a régua do microfone | onda saturada em VERMELHO, igual ao 2G–16G |
| 2.1 | A onda oscila em torno de zero | onda de voz real; média ≈ 0, RMS ≠ 0 |
| 2.2 | Palma: um número resolve | RMS + limiar, LED |
| 2.3 | **FIOT × EMERGÊNCIA: as features empatam** | tabela de 6 features com valores quase idênticos |
| 2.4 | O que muda é a frequência ao longo do tempo | os dois espectrogramas lado a lado |
| 3.1 | Espectrograma em 4 passos | fatia 25 ms → faixas → coluna → imagem |
| 3.2 | MFCC: a imagem que a rede vê | saída real do Studio |
| 3.3 | O celular é o sensor · Split | QR code → 60 s viram 25 amostras |
| 3.4 | Quatro classes, não duas | `fiot` · `emergencia` · `ruido` · `desconhecido` |
| 4.1 | A wake word é evento curto | reusa o efeito de borda da A14 |
| 4.2 | **A máquina de estados** | DORMINDO → ATIVO → comando → timeout |
| 4.3 | Limiar de confiança | falso positivo × falso negativo |
| 4.4 | Treinou no celular, roda no INMP441 | descasamento de domínio, honesto |

Segue o `INSTRUÇÕES PARA POWERPOINT.md`: bloco fixo + uma seção por vez, âncora pelo título da
divisória, máx. 5 marcadores, elemento visual em **todo** slide feito com formas do PowerPoint,
notas do apresentador em todos. Código de cores da trilha, com uma convenção nova para áudio:
**AZUL** = onda crua vinda do microfone · **LARANJA** = qualquer feature calculada (RMS, ZCR,
MFCC) · **VERDE/VERMELHO** = detectado / não detectado.

### 7.4 Patches nos decks existentes (depois do deck novo)

- **Aula 13** — 3 slides de áudio depois do slide 16, cumprindo a agenda que a própria aula
  promete.
- **Aula 14** — 2 slides: o ZCR sai da lista do s23 e vira slide; o s26 (FFT) vira a dobradiça
  para o MFCC.

---

## 8. Ordem de execução

| Fase | Entrega | Status |
|---|---|---|
| A | `CLAUDE.md`, `README.md` índice, limpeza dos herdados, `platformio.ini` | **feito** |
| B | Etapa 1 (I2S + Teleplot) | **feito** — `pio run` ok |
| C | Etapas 2 e 3 (+ `INMP441.hpp`) | **feito** — `pio run` ok |
| D | `docs/coleta-edge-impulse.md` | **feito** — falta você gravar e treinar |
| E | Etapa 4 (esqueleto; `lib/` do EI entra após o deploy) | a fazer |
| F | `docs/slides-aula-25.1.md` no formato do `INSTRUÇÕES PARA POWERPOINT.md` | a fazer |
| G | Etapa 5 (MQTT) | a fazer |
| H | Etapa 6 (assistente estendido, extra) | a fazer |

Falta validar na bancada o que só o hardware responde: o `GANHO_SHIFT` (padrão 11) e se o
canal é `ONLY_LEFT` ou `ONLY_RIGHT` neste módulo INMP441.

---

## 9. Decisões tomadas

| # | Decisão |
|---:|---|
| 1 | Wake word **FIOT** acorda o dispositivo (LED azul). Comandos: **EMERGÊNCIA** (buzzer) na versão da turma; **ALERTA** (motor de vibração) e **LOCALIZAÇÃO** (GPS) na versão estendida do professor |
| 2 | Atuadores: LED azul GPIO 21, buzzer GPIO 19, motor GPIO 18 (com transistor), GPS UART2 16/17 |
| 3 | MQTT entra na etapa 5, notificando a EMERGÊNCIA, antes da versão estendida |
| 4 | `diagram.json` e `wokwi.toml` apagados de todas as etapas |
| 5 | Deck novo `IoT - Aula 25.1 - EI - Áudio.pptx`, já criado; patches nas Aulas 13 e 14 depois |
| 6 | Dataset coletado pelo celular no Edge Impulse Studio |

**Em aberto:** a janela do impulse — recomendo **1500 ms** (ver §1), a confirmar antes de você
gravar.
