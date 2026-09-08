# app-FIoT-Audio

Trilha didática de áudio para a disciplina de IoT (FIAP, 2º ano): do sinal cru do microfone
até uma wake word que aciona um atuador. Seis projetos PlatformIO independentes, em ordem
crescente de conceito. O plano completo está em [PLANO.md](PLANO.md).

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

Atuadores **só a partir da etapa 4** — as etapas 1 a 3 não acionam nada:

| Atuador | GPIO | Etapa |
|---|---|---|
| LED azul (dispositivo ATIVO) | 21 | 4, 5, 6 |
| Buzzer (EMERGÊNCIA) | 19 | 4, 5, 6 |
| Motor de vibração 3 V (ALERTA) | 18 | 6 — **sempre** via transistor NPN + diodo de roda livre |
| GPS NEO-6M (LOCALIZAÇÃO) | RX2 16 / TX2 17 | 6 |
| LED onboard (status MQTT) | 2 | 5, 6 |

## Estrutura de cada etapa

```text
app-N-mic-XXX/
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
hardware físico. Não recrie esses arquivos.

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
- **Sem estado global esperto** nas etapas 1 a 3: nada de sobreposição de janela com `memmove`,
  buffer circular ou máquina de estados.

O código que o professor digita tem que caber na cabeça de quem está vendo pela primeira vez.

### Duas formas de ler o microfone, e cada uma tem seu lugar

| Etapa | Como | Por quê |
|---|---|---|
| **0** | `<I2S.h>`, a lib do próprio core — `I2S.read()` | mostrar que sai número com o mínimo de código, sem cabeçalho nenhum |
| **1 em diante** | `INMP441.hpp` (nosso) — `INMP441::ler(som, N)` | ler em bloco, com DMA, sem perder amostra |

A etapa 0 **tem** que ficar limitada: `I2S.read()` uma amostra por vez não acompanha 16 kHz e o
sinal sai picotado. Esse defeito é o que justifica o `INMP441.hpp` na etapa 1 — não conserte a
etapa 0.

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
- Serial a 115200 em todas as etapas.

## Restrições didáticas — não "melhore" isto

### As etapas 1 a 3 são observação pura

**Não acionam nada.** Sem LED, sem buzzer, sem relé, sem `digitalWrite` em atuador. Elas
existem para mostrar o comportamento da onda e o que se extrai dela — o aluno olha o Teleplot,
não uma lâmpada.

O acionamento só aparece **depois do Edge Impulse**, no código de inferência (etapas 4 a 6).
Essa separação é o arco da trilha: primeiro entender o sinal, depois treinar o modelo, e só
então agir. Se um limiar precisar ser mostrado nas etapas 1 a 3, ele vira **uma linha no
gráfico**, nunca um comando.

### Nas etapas 1 a 3 não há transformada

**Proibido FFT, MFCC, filtro digital ou biblioteca de DSP.** Só estatística de janela: média,
desvio-padrão, pico a pico, RMS, máximo absoluto e cruzamentos por zero.

O ponto pedagógico é o aluno **bater no teto** dessas features e concluir sozinho que "FIOT" e
"EMERGÊNCIA" empatam nelas — é isso que motiva o Edge Impulse na etapa 4. Antecipar a
transformada destrói a aula. Se algo parecer "fácil de melhorar com uma FFT", é de propósito.

A partir da etapa 4, o MFCC vem pronto dentro da biblioteca exportada pelo Edge Impulse.

## Etapas 4 a 6 — Edge Impulse

- A biblioteca exportada vai em `lib/<Nome>_inferencing/` (mesma convenção do
  `app33-MagicWand` no repositório principal). É grande; é versionada mesmo assim.
- `board_build.partitions = huge_app.csv` no `platformio.ini`, senão o binário estoura a
  partição padrão de 1,3 MB.
- O dataset é gravado **pelo celular** no Edge Impulse Studio, não por firmware. Ver
  [docs/coleta-edge-impulse.md](docs/coleta-edge-impulse.md).
- A máquina de estados DORMINDO → ATIVO fica no firmware, não no modelo. As palavras de comando
  só valem com o dispositivo ATIVO.

## Verificação

Antes de dar qualquer etapa por pronta:

```bash
pio run -e esp32          # dentro da pasta da etapa
```

As etapas 4 a 6 só compilam depois que a biblioteca do Edge Impulse estiver em `lib/`.

Não versione `.pio/`.

## README de cada etapa

Runbook curto, no padrão do `app24` do repositório principal: título com uma linha do que faz,
tabela de pinos, passos numerados de uso, exemplo real da saída. **Não** é ensaio — se passar
de ~80 linhas, está errado. Os READMEs originais desta pasta eram cópias de 436 linhas do app
de irrigação; não repita isso.
