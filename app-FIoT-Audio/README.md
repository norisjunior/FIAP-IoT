# app-FIoT-Audio — do sinal de áudio à wake word

Trilha de áudio para a disciplina de IoT: seis projetos PlatformIO independentes, do sinal cru
do microfone até uma palavra que aciona um atuador. Cada etapa acrescenta **um** conceito e
roda sozinha.

**Hardware:** ESP32 DevKit v1 + microfone I2S **INMP441**.
**Aplicação:** o dispositivo dorme até ouvir **FIOT**; acordado, obedece a um comando.

```text
              ┌──────────────────────────────┐
              │          DORMINDO            │
              │   só reage à palavra FIOT    │
              └──────────────┬───────────────┘
                             │  "FIOT"
                             ▼
              ┌──────────────────────────────┐
timeout 5 s   │       ATIVO — LED azul       │
 ◄────────────│                              │
              └──────────────┬───────────────┘
                             │
        ┌────────────────────┼────────────────────┐
        ▼                    ▼                    ▼
  "EMERGÊNCIA"          "ALERTA"           "LOCALIZAÇÃO"
   buzzer + MQTT      motor de vibração       posição GPS
    (etapas 4-5)          (etapa 6)            (etapa 6)
```

## A trilha

### Bloco 1 — observar o sinal (PlatformIO)

Nada é acionado. O aluno olha o Teleplot e entende a onda.

| # | Pasta | O que o aluno vê | Conceito novo |
|---|---|---|---|
| 0 | [app-0-mic-SOM](app-0-mic-SOM/) | sai número do microfone — mas picotado | microfone digital, escravo I2S |
| 1 | [app-1-mic-RAW](app-1-mic-RAW/) | a onda inteira, sem buracos | leitura em bloco, DMA, ganho |
| 2 | [app-2-mic-PALMA](app-2-mic-PALMA/) | 16 000 números/s viram 31 | janela, média, RMS |
| 3 | [app-3-mic-VOZ_FEATURES](app-3-mic-VOZ_FEATURES/) | 4096 amostras viram 4 números | `calcMean`, `calcStd`, `calcPtP`, `calcZCR` |

### Bloco 2 — treinar o modelo (Edge Impulse)

| | | |
|---|---|---|
| — | [docs/coleta-edge-impulse.md](docs/coleta-edge-impulse.md) | gravar pelo celular, rotular, split, MFCC, treinar, exportar |

### Bloco 3 — inferir e agir (de volta ao PlatformIO)

Aqui, e só aqui, o dispositivo passa a acionar coisas.

| # | Pasta | O que o aluno vê | Conceito novo |
|---|---|---|---|
| 4 | [app-4-mic-FIOT](app-4-mic-FIOT/) | FIOT acorda; EMERGÊNCIA toca o buzzer | MFCC, janela deslizante, máquina de estados |
| 5 | [app-5-mic-FIOT-MQTT](app-5-mic-FIOT-MQTT/) | a emergência chega no Node-RED | a borda decide, a nuvem registra |
| 6 | [app-6-mic-ASSISTENTE](app-6-mic-ASSISTENTE/) | + ALERTA (vibra) e LOCALIZAÇÃO (GPS) | multiclasse e atuadores — *demonstração* |

A etapa 6 é extra, para demonstração — não entra no roteiro da turma.

## O arco

Cada etapa do bloco 1 existe porque a anterior **bateu num limite que dá para ver na tela**:

| Etapa | O que trava | O que a próxima faz |
|---|---|---|
| 0 | uma amostra por vez não acompanha 16 kHz — o sinal sai picotado | ler em bloco |
| 1 | a onda inteira não cabe numa decisão | resumir a janela num número |
| 2 | um número diz *quão alto*, nunca *o quê* | descrever a janela com quatro números |
| 3 | as features não distinguem *FIOT* de *EMERGÊNCIA* | **Edge Impulse** |

O último degrau é o ponto da trilha: as features dizem quanta energia existe, não quais
frequências vieram em qual ordem. É isso que justifica o MFCC — em vez de a rede neural
aparecer como mágica.

> **Nas etapas 1 a 3 não há FFT, MFCC, filtro digital — nem atuador.** São observação pura, de
> propósito. Ver [CLAUDE.md](CLAUDE.md).

## Pinagem

As etapas 1 a 3 usam **só o microfone**. Os atuadores entram a partir da etapa 4.

| INMP441 | ESP32 | | Atuador | GPIO | Etapa |
|---|---|---|---|---|---|
| VDD | 3V3 | | LED azul (ATIVO) | 21 | 4, 5, 6 |
| GND | GND | | Buzzer (EMERGÊNCIA) | 19 | 4, 5, 6 |
| L/R | GND | | Motor de vibração (ALERTA) | 18 | 6 — via transistor |
| SCK | 26 | | GPS NEO-6M | RX2 16 / TX2 17 | 6 |
| WS | 25 | | LED onboard (status MQTT) | 2 | 5, 6 |
| SD | 33 | | | | |

O motor de vibração **nunca** vai direto no GPIO: transistor NPN + diodo de roda livre.

## Como rodar qualquer etapa

```bash
cd app-N-mic-XXX
pio run -t upload
pio device monitor -b 115200
```

As etapas 1 a 3 usam o **Teleplot** (extensão do VS Code) para o gráfico ao vivo.

**Não há simulação no Wokwi**: o simulador não tem peça INMP441. Todas as etapas exigem
hardware físico.

## Documentos

| | |
|---|---|
| [PLANO.md](PLANO.md) | o plano completo, com as decisões e o porquê de cada uma |
| [CLAUDE.md](CLAUDE.md) | convenções técnicas — leia antes de mexer no código |
| [docs/coleta-edge-impulse.md](docs/coleta-edge-impulse.md) | gravar o dataset pelo celular |
| [docs/slides-aula-25.1.md](docs/slides-aula-25.1.md) | especificação dos slides da aula |
