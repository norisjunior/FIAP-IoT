# app36-Audio

Trilha didática de áudio para a disciplina de IoT (FIAP, 2º ano): do sinal cru do microfone
até uma wake word que aciona um atuador. Pastas independentes, em ordem crescente de conceito:

| Pasta | O que é |
|---|---|
| `app36-0-mic-SOM` | apoio, fora do roteiro (ver abaixo) |
| `app36-1-mic-RAW`, `app36-2-mic-VOZ_FEATURES` | observar o sinal do INMP441 (PlatformIO) |
| `app36-3-WakeWord-DockerHTTP` | a wake word do aluno no notebook/celular, Docker como motor |
| `app36-4-WakeWord-ESP32` | a wake word no ESP32-S3 + INMP441 (PlatformIO + lib EI) |
| `app36-5-WakeWord-Android` | a wake word no Android (Flutter + FFI), com `docs/` |
| `app36-6-DeskBuddy-ESP32` | o app36-4 com olhos na tela redonda GC9A01 (TFT_eSPI, SPI nos GPIOs 9–13) |
| `app36-7-DeskBuddy-Toque-ESP32` | o app36-6 com touch TTP223B no GPIO 4 e o LED RGB da placa (WS2812, GPIO 48) |
| `app36-Analogico-Wokwi/` | as versões antigas com microfone analógico (os antigos `app28-*`) |

O plano completo está em [PLANO.md](PLANO.md); a tabela no topo dele liga os nomes antigos
(`app-N-mic-*`, `app40-*`, `app28-*`) às pastas de hoje.

A wake word é **"Ei Fioti"**, e o modelo do Edge Impulse (`WakeWord-EiFIOT`) **já está
treinado** — veio do trilho Android em [app36-5-WakeWord-Android/docs/](app36-5-WakeWord-Android/docs/). As etapas com
Edge Impulse reaproveitam esse projeto; só a etapa EMERGÊNCIA (ainda no plano) exige coleta nova. Não proponha regravar o
dataset para fazer a trilha funcionar.

## Alvo e toolchain — leia antes de escrever qualquer linha

| | |
|---|---|
| Placa | ESP32 DevKit v1 — `board = esp32dev` |
| Plataforma | `platform = espressif32@6.12.0` → **Arduino core 2.0.17** (IDF 4.4) |
| Framework | `framework = arduino` |

**A API de I2S é a legada.** Use `#include <driver/i2s.h>` com `i2s_config_t`,
`i2s_driver_install()`, `i2s_set_pin()` e `i2s_read()`.

**Não** use `ESP_I2S.h` / `I2SClass` nem `driver/i2s_std.h` com `i2s_chan_handle_t`: são do
Arduino core 3.x e **não existem** neste core. A maioria dos tutoriais de INMP441 na internet
usa a API nova — eles não compilam aqui. Se for preciso mudar de core, é decisão do professor,
não ajuste silencioso.

Detalhes que já custaram tempo:

- os campos de `i2s_config_t` são inicializados **na ordem de declaração** (C++ exige isso em
  designated initializers): `mode`, `sample_rate`, `bits_per_sample`, `channel_format`,
  `communication_format`, `intr_alloc_flags`, `dma_buf_count`, `dma_buf_len`, `use_apll`,
  `tx_desc_auto_clear`, `fixed_mclk`;
- use `I2S_COMM_FORMAT_STAND_I2S` — `I2S_COMM_FORMAT_I2S` está marcado como *deprecated*;
- o INMP441 entrega **24 bits alinhados à esquerda num slot de 32 bits**: leia em `int32_t` e
  desloque. Ler em `int16_t` devolve lixo.

## Hardware — pinagem canônica, não inventa outra

| INMP441 | ESP32 |
|---|---|
| VDD | 3V3 (nunca 5V) |
| GND | GND |
| L/R | GND (canal esquerdo, mono) |
| SCK (BCLK) | GPIO 26 |
| WS (LRCL) | GPIO 25 |
| SD (DOUT) | GPIO 33 |

Atuadores **só a partir da etapa 3** — as etapas 1 e 2 não acionam nada:

| Atuador | GPIO | Etapa |
|---|---|---|
| LED azul (dispositivo ATIVO) | 21 | 3, 4, 5, 6 |
| Motor de vibração 3 V (ALERTA) | 18 | 5 — **sempre** via transistor NPN + diodo de roda livre |
| GPS NEO-6M (LOCALIZAÇÃO) | RX2 16 / TX2 17 | 5 |
| Buzzer (EMERGÊNCIA) | 19 | 6 |
| LED onboard (status MQTT) | 2 | 4, 5, 6 |

## Estrutura de cada etapa

```text
app36-N-mic-XXX/
├── platformio.ini    cabeçalho comentado dizendo o que a etapa faz
├── README.md         runbook curto
├── .gitignore        .pio / .vscode
└── src/
    ├── <nome>.cpp    o único arquivo compilável — o que se digita em aula
    ├── INMP441.hpp   driver do microfone, igual em todas as etapas
    └── *.txt         variantes didáticas, fora do build
```

**Um arquivo compilável por pasta.** Variantes e versões históricas ficam como `.txt` — é como
esta trilha guarda várias versões didáticas sem quebrar o build do PlatformIO. Se precisar de
mais uma variante, crie um `.txt`, nunca um segundo `.cpp`/`.ino`.

**Sem `diagram.json` e sem `wokwi.toml`.** O Wokwi não tem peça INMP441; estas etapas são só
hardware físico. Não recrie esses arquivos. A exceção é `app36-Analogico-Wokwi/` (microfone analógico
KY038/MAX9814), cujos arquivos de Wokwi vieram do app24 e ainda não foram revistos.

## Estilo do código de aula — a regra mais importante deste arquivo

**O `.cpp` de cada etapa é digitado à mão, ao vivo, na frente da turma.** Isso manda em tudo:

- **20 a 40 linhas.** Passou disso, alguma coisa que não é a aula entrou no arquivo.
- **Comentário só onde a linha não se explica**, e de uma linha só. Nada de cabeçalho de 30
  linhas: o "porquê" mora no `README.md` da etapa, que o aluno lê depois.
- **Correto o bastante, não robusto.** `float` em vez de `long long` para não ter conversa sobre
  overflow, `sqrt(soma/N)` direto, sem checar retorno de erro, sem calibração automática. É
  demonstração, não produção.
- **Nada de flag de configuração.** Sem `#if MODO_X`, sem caminho A/B. Um caminho só; quem
  quiser outro comportamento edita o número no topo.
- **Sem estado global esperto** nas etapas 1 e 2: nada de sobreposição de janela com `memmove`,
  buffer circular ou máquina de estados.

O código que o professor digita tem que caber na cabeça de quem está vendo pela primeira vez.

### O `app36-0-mic-SOM` está fora do roteiro

A trilha da aula começa no **app-1**. O `app-0` lê o microfone com a lib `<I2S.h>` crua e existe
só como **material de apoio**, para o caso de alguém perguntar por que o app-1 precisa do
`INMP441.hpp`.

Ele **tem** que continuar defeituoso: `I2S.read()` entrega uma amostra por chamada (sinal
picotado) e a `<I2S.h>` não tem modo mono (metade das amostras é canal vazio). **Não conserte o
app-0** — os dois defeitos são a resposta que ele existe para dar.

**Não existe biblioteca específica do INMP441**, e não é omissão do ecossistema: o chip não tem
registradores, não tem endereço e não aceita comando. A única configuração dele é o pino L/R, e
se faz com um fio. Uma "lib do INMP441" seria uma lib do periférico I2S do ESP32 — que é
exatamente o que o `<I2S.h>` é. Não saia procurando outra.

### O que fica escondido no `INMP441.hpp`

Só o barramento. **`analogRead()` também é um monte de código — só já está escondido no
framework**; fazer o aluno ver a struct do I2S é como fazer ele ver a calibração do ADC.

**Não mova mais nada para lá.** Média, RMS, features e janela são *a aula* e ficam no `.cpp`.

## Outras convenções

- Configuração em `#define` no topo do arquivo. O aluno mexe ali, não caça no meio do `loop()`.
- Português do Brasil em comentários e mensagens. Identificadores em português.
- **Teleplot** para gráfico ao vivo: `Serial.printf(">nome:%d\n", valor)`.
- Serial a **921600** nas etapas 1 e 2 (115200 não dá conta de plotar a onda: ver abaixo). O
  Teleplot precisa ser configurado na mesma taxa. Se um README disser 115200, é erro — corrija.

## Duas placas, um código

A turma usa **ESP32 DevKit v1**; o professor tem um **ESP32-S3 Super Mini**. Não duplique
projeto por causa disso.

Cada etapa tem um `platformio.ini` (DevKit v1) e um `platformio.ini.esp32s3`, que se **renomeia**
para trocar de alvo. O S3 exige `board = esp32-s3-devkitc-1`, `flash_size = 4MB`,
`partitions = default.csv` e `-DARDUINO_USB_CDC_ON_BOOT=1` (sem essa flag o Monitor Serial fica
mudo — a Super Mini usa USB nativo, não tem chip USB-serial).

Os pinos ficam num `#ifdef PLACA_S3` no topo do `INMP441.hpp`, e a flag vem do `.ini`. **O
código em `src/` é idêntico nas duas placas** — mantenha assim.

No S3, **nunca** use GPIO 26–32 (flash interna), 33–37 (PSRAM), 19–20 (USB), 0/45/46
(strapping), 43/44 (UART0) ou 48 (LED RGB).

## Quanto dá para imprimir

Cada linha `>som:-12345\n` tem ~12 bytes. A 921600 baud cabem ~7.700 linhas/s; a 115200, só
~960. A etapa 1 imprime 1 amostra a cada 8 → **2.000 pontos/s**, que é onde o Teleplot começa a
engasgar. Não adianta subir mais.

No S3 o baud é decorativo (USB CDC nativo), mas mantenha 921600 para o código servir nas duas.

## Restrições didáticas — não "melhore" isto

### As etapas 1 e 2 são observação pura

**Não acionam nada.** Sem LED, sem buzzer, sem relé, sem `digitalWrite` em atuador. Elas
existem para mostrar o comportamento da onda e o que se extrai dela — o aluno olha o Teleplot,
não uma lâmpada.

O acionamento só aparece **depois do Edge Impulse**, no código de inferência (etapas 3 a 6).
Essa separação é o arco da trilha: primeiro entender o sinal, depois treinar o modelo, e só
então agir. Se um limiar precisar ser mostrado nas etapas 1 e 2, ele vira **uma linha no
gráfico**, nunca um comando.

### Nas etapas 1 e 2 não há transformada

**Proibido FFT, MFCC, filtro digital ou biblioteca de DSP.** Só estatística de janela: média,
desvio-padrão, pico a pico, RMS, máximo absoluto e cruzamentos por zero.

O ponto pedagógico é o aluno **bater no teto** dessas features e concluir sozinho que "Ei
Fioti" empata com qualquer outra palavra do mesmo volume — é isso que motiva o Edge Impulse na
etapa 3. Antecipar a transformada destrói a aula. Se algo parecer "fácil de melhorar com uma
FFT", é de propósito.

A partir da etapa 3, o MFCC vem pronto dentro da biblioteca exportada pelo Edge Impulse.

## Wake word no ESP32 (`app36-4`) — Edge Impulse

- O código **não** é digitado do zero: é o exemplo `esp32_microphone_continuous` da biblioteca
  exportada, com as linhas alteradas marcadas `// AJUSTE`. Ver o README da pasta.
- O `.zip` da biblioteca vai para o Git em `lib/`; a pasta descompactada
  (`lib/<Nome>_inferencing/`) não (`lib/*/` no `.gitignore`).
- No ESP32-S3 o export padrão (float32, EON) cabe em `default.csv`. No DevKit v1, se o binário
  estourar a partição de 1,3 MB, use `board_build.partitions = huge_app.csv`.
- O dataset foi gravado **pelo celular** no Edge Impulse Studio, não por firmware, e **já
  existe**. Ver [docs/coleta-edge-impulse.md](docs/coleta-edge-impulse.md).
- O plano previa **int8** no ESP32 para economizar RAM; o `app36-4` acabou usando float32 no S3
  sem aperto. O Android usa float32. Não copie a configuração de um para o outro sem medir.
- O MFCC do Edge Impulse espera escala **int16 crua**, não `[-1, 1]`. Normalizar quebra o
  modelo **em silêncio**, sem erro. Já custou tempo no Android; não repita aqui.
- A máquina de estados DORMINDO → ATIVO fica no firmware, não no modelo. As palavras de comando
  só valem com o dispositivo ATIVO.

## O trilho Android

O mesmo impulse roda no celular, em `app36-5-WakeWord-Android/`: o código Flutter + FFI fica
em `app/` e a documentação (plano, auditoria, roteiro de coleta, aula) em `docs/`.

O SDK do Edge Impulse (29 MB, ~1400 arquivos) **não está no Git** do lado do app: o `.zip` é a
fonte, e um `Expand-Archive` o recria. Não versione o SDK extraído.

## Verificação

Antes de dar qualquer etapa por pronta:

```bash
pio run                   # dentro da pasta da etapa
```

O `app36-4` só compila depois que a biblioteca do Edge Impulse estiver descompactada em `lib/`.

Não versione `.pio/`.

## README de cada etapa

Runbook curto, no padrão do `app24` do repositório principal: título com uma linha do que faz,
tabela de pinos, passos numerados de uso, exemplo real da saída. **Não** é ensaio — se passar
de ~80 linhas, está errado. Os READMEs originais desta pasta eram cópias de 436 linhas do app
de irrigação; não repita isso.
