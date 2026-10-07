# app-FIoT-Audio — do sinal de áudio à wake word

Trilha de áudio para a disciplina de IoT: do sinal cru do microfone até uma palavra que aciona
um atuador. Cada etapa acrescenta **um** conceito e roda sozinha.

**Hardware:** ESP32 DevKit v1 + microfone I2S **INMP441**.
**Wake word:** **"Ei Fioti"** — o dispositivo dorme até ouvir; acordado, obedece a um comando.

```text
              ┌──────────────────────────────┐
              │          DORMINDO            │
              │  só reage a  "Ei Fioti"      │
              └──────────────┬───────────────┘
                             │  "Ei Fioti"
                             ▼
              ┌──────────────────────────────┐
timeout 5 s   │       ATIVO — LED azul       │
 ◄────────────│                              │
              └──────────────┬───────────────┘
                             │
        ┌────────────────────┼────────────────────┐
        ▼                    ▼                    ▼
    "ALERTA"           "LOCALIZAÇÃO"        "EMERGÊNCIA"
 motor de vibração       posição GPS         buzzer + MQTT
     (etapa 5)            (etapa 5)         (etapa 6 — a fazer)
```

## A trilha

### Bloco 1 — observar o sinal (PlatformIO)

Nada é acionado. O aluno olha o Teleplot e entende a onda. São **duas etapas**, de propósito: a
turma é de IA e o objetivo é chegar rápido no Edge Impulse.

| # | Pasta | O que o aluno vê | Conceito novo |
|---|---|---|---|
| 1 | [app-1-mic-RAW](app-1-mic-RAW/) | a onda da voz reagindo ao vivo | o microfone entrega número pronto |
| 2 | [app-2-mic-VOZ_FEATURES](app-2-mic-VOZ_FEATURES/) | 16 000 números/s viram 4 | janela, RMS, `calcStd`, `calcZCR` |

### Bloco 2 — o modelo (Edge Impulse)

**O modelo já está treinado.** O projeto `WakeWord-EiFIOT` (MFCC + CNN, janela 1,5 s a 16 kHz,
classes `WakeWord`/`desconhecido`/`ruido`) foi gravado e validado no trilho Android — 93,1% de
validação. As etapas 3 a 5 só fazem outro Deployment dele.

| | |
|---|---|
| [docs/coleta-edge-impulse.md](docs/coleta-edge-impulse.md) | exportar para o ESP32; e como gravar, se for treinar de novo |
| [android-wakeword/](android-wakeword/) | o trilho irmão, que produziu o modelo |

### Bloco 3 — inferir e agir (de volta ao PlatformIO)

Aqui, e só aqui, o dispositivo passa a acionar coisas.

| # | Pasta | O que o aluno vê | Conceito novo |
|---|---|---|---|
| 3 | [app-3-mic-FIOT](app-3-mic-FIOT/) | "Ei Fioti" acende o LED azul | MFCC, janela deslizante, máquina de estados |
| 4 | [app-4-mic-FIOT-MQTT](app-4-mic-FIOT-MQTT/) | a detecção chega no Node-RED | a borda decide, a nuvem registra |
| 5 | [app-5-mic-ASSISTENTE](app-5-mic-ASSISTENTE/) | + ALERTA (vibra) e LOCALIZAÇÃO (GPS) | multiclasse e atuadores — *demonstração* |
| 6 | [app-6-mic-EMERGENCIA](app-6-mic-EMERGENCIA/) | o comando EMERGÊNCIA | **a fazer** — exige projeto EI novo |

As etapas 5 e 6 são extras, fora do roteiro da turma.

## Os dois alvos, um modelo

O mesmo impulse do Edge Impulse roda nos dois lados. É o argumento inteiro do Edge AI: o modelo
é portátil, o porting é que muda.

| | ESP32 + INMP441 | Android (Galaxy A52) |
|---|---|---|
| Onde está | aqui, etapas 3 a 5 | [android-wakeword/](android-wakeword/) (docs) · `FIAP-IoT-eval` (código) |
| Deployment | Arduino library, **int8** | Android library C++, **float32** + EON |
| Ponte | firmware direto | Flutter → Dart FFI → `libwakeword.so` |
| Inferência | a medir | **2 a 4 ms** |
| Estado | etapas 3 a 6 a fazer | **funcionando** |

## O arco

Cada etapa existe porque a anterior bateu num limite que dá para ver na tela:

| Etapa | O que trava | O que a próxima faz |
|---|---|---|
| 1 | a onda inteira não cabe numa decisão | descrever a janela com quatro números |
| 2 | as features dizem *quão alto* e *quão agudo*, nunca *o quê* | **Edge Impulse** |

O último degrau é o ponto da trilha: as features dizem quanta energia existe, não quais
frequências vieram em qual ordem. Tocar "Ei Fioti" de trás para frente dá as mesmas quatro
features. É isso que justifica o MFCC — em vez de a rede neural aparecer como mágica.

> **Material de apoio, fora do roteiro:** [app-0-mic-SOM](app-0-mic-SOM/) mostra o microfone
> lido com a biblioteca `<I2S.h>` crua, e o gráfico sai picotado. Só serve se alguém perguntar
> por que o app-1 precisa do `INMP441.hpp`.

> **Nas etapas 1 e 2 não há FFT, MFCC, filtro digital — nem atuador.** São observação pura, de
> propósito. Ver [CLAUDE.md](CLAUDE.md).

## Pinagem

As etapas 1 e 2 usam **só o microfone**. Os atuadores entram a partir da etapa 3.

| INMP441 | ESP32 | | Atuador | GPIO | Etapa |
|---|---|---|---|---|---|
| VDD | 3V3 | | LED azul (ATIVO) | 21 | 3, 4, 5, 6 |
| GND | GND | | Motor de vibração (ALERTA) | 18 | 5 — via transistor |
| L/R | GND | | GPS NEO-6M | RX2 16 / TX2 17 | 5 |
| SCK | 26 | | Buzzer (EMERGÊNCIA) | 19 | 6 |
| WS | 25 | | LED onboard (status MQTT) | 2 | 4, 5, 6 |
| SD | 33 | | | | |

O motor de vibração **nunca** vai direto no GPIO: transistor NPN + diodo de roda livre.

## Como rodar qualquer etapa

```bash
cd app-N-mic-XXX
pio run -t upload
pio device monitor -b 921600
```

As etapas 1 e 2 usam o **Teleplot** (extensão do VS Code) para o gráfico ao vivo — configure-o
também para **921600**.

### Outra placa

Cada etapa traz um `platformio.ini.esp32s3`. Para rodar no **ESP32-S3 Super Mini**, renomeie-o
para `platformio.ini` (guarde o original como `platformio.ini.devkit1`). **O código em `src/`
não muda**: a flag `PLACA_S3` troca os pinos dentro do `INMP441.hpp`.

| | DevKit v1 (turma) | S3 Super Mini |
|---|---|---|
| SCK | 26 | 4 |
| WS | 25 | 5 |
| SD | 33 | 6 |

**Não há simulação no Wokwi**: o simulador não tem peça INMP441. Todas as etapas exigem
hardware físico.

## Documentos

| | |
|---|---|
| [PLANO.md](PLANO.md) | o plano completo, com as decisões e o porquê de cada uma |
| [CLAUDE.md](CLAUDE.md) | convenções técnicas — leia antes de mexer no código |
| [docs/coleta-edge-impulse.md](docs/coleta-edge-impulse.md) | Edge Impulse: exportar, e gravar se precisar |
| [docs/slides-aula-25.1.md](docs/slides-aula-25.1.md) | especificação dos slides da aula do ESP32 |
| [android-wakeword/](android-wakeword/) | plano, auditoria, coleta e aula do trilho Android |
