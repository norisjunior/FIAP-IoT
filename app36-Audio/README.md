# app36-Audio — do sinal de áudio à wake word

Trilha de áudio para a disciplina de IoT: do sinal cru do microfone até uma palavra que o
dispositivo reconhece sozinho. Cada etapa acrescenta **um** conceito e roda sozinha.

**Hardware:** ESP32 (DevKit v1 da turma ou ESP32-S3 Super Mini do professor) + microfone I2S
**INMP441**.
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
                    (planejado: ver PLANO.md)
```

## A trilha

### Bloco 1 — observar o sinal (PlatformIO)

Nada é acionado. O aluno olha o Teleplot e entende a onda. São **duas etapas**, de propósito: a
turma é de IA e o objetivo é chegar rápido no Edge Impulse.

| # | Pasta | O que o aluno vê | Conceito novo |
|---|---|---|---|
| 1 | [app36-1-mic-RAW](app36-1-mic-RAW/) | a onda da voz reagindo ao vivo | o microfone entrega número pronto |
| 2 | [app36-2-mic-VOZ_FEATURES](app36-2-mic-VOZ_FEATURES/) | 16 000 números/s viram 4 | janela, RMS, `calcStd`, `calcZCR` |

> **Material de apoio, fora do roteiro:** [app36-0-mic-SOM](app36-0-mic-SOM/) mostra o
> microfone lido com a biblioteca `<I2S.h>` crua, e o gráfico sai picotado. Só serve se alguém
> perguntar por que o app36-1 precisa do `INMP441.hpp`.

> **Versão antiga:** [app36-Analogico-Wokwi/](app36-Analogico-Wokwi/) tem as mesmas ideias com
> microfone analógico (KY038/MAX9814). Os arquivos de Wokwi dela ainda precisam de revisão.

### Bloco 2 — o modelo (Edge Impulse)

O projeto `WakeWord-EiFIOT` (MFCC + CNN, janela 1,5 s a 16 kHz, classes
`WakeWord`/`desconhecido`/`ruido`) já está treinado — 93,1% de validação. Cada aluno também
pode treinar a **própria** wake word, gravando pelo celular.

| | |
|---|---|
| [docs/coleta-edge-impulse.md](docs/coleta-edge-impulse.md) | como gravar e como exportar |
| [app36-5-WakeWord-Android/docs/](app36-5-WakeWord-Android/docs/) | plano, coleta e auditoria do modelo `WakeWord-EiFIOT` |

### Bloco 3 — a wake word rodando

O mesmo impulse, três alvos. A ordem é a da aula: primeiro onde todo aluno consegue testar,
depois a placa, e o celular nativo quando a turma já souber Android.

| # | Pasta | Onde roda | Deployment |
|---|---|---|---|
| 3 | [app36-3-WakeWord-DockerHTTP](app36-3-WakeWord-DockerHTTP/) | notebook, ou celular por uma página HTTPS | Docker container |
| 4 | [app36-4-WakeWord-ESP32](app36-4-WakeWord-ESP32/) | ESP32-S3 Super Mini + INMP441 | Arduino library |
| 5 | [app36-5-WakeWord-Android](app36-5-WakeWord-Android/) | Android (Flutter + FFI) | Android library C++ |
| 6 | [app36-6-DeskBuddy-ESP32](app36-6-DeskBuddy-ESP32/) | o app36-4 com olhos numa tela redonda, que abrem com a wake word | Arduino library |
| 7 | [app36-7-DeskBuddy-Toque-ESP32](app36-7-DeskBuddy-Toque-ESP32/) | o app36-6 com botão touch (acordar, modo mudo) e o LED RGB da placa mostrando o estado | Arduino library |
| 8 | [app36-8-DeskBuddy-DeepSleep-ESP32](app36-8-DeskBuddy-DeepSleep-ESP32/) | o app36-7 com deep sleep: o toque longo apaga a tela e põe para dormir, com o LED vermelho aceso | Arduino library |

As próximas etapas (MQTT, ASSISTENTE com ALERTA/LOCALIZAÇÃO e EMERGÊNCIA) ainda estão só no
[PLANO.md](PLANO.md).

## Um modelo, vários alvos

É o argumento inteiro do Edge AI: o modelo é portátil, o porting é que muda.

| | ESP32-S3 + INMP441 | Android (Galaxy A52) |
|---|---|---|
| Pasta | [app36-4](app36-4-WakeWord-ESP32/) | [app36-5](app36-5-WakeWord-Android/) |
| Deployment | Arduino library, float32 + EON | Android library C++, float32 + EON |
| Ponte | firmware direto | Flutter → Dart FFI → `libwakeword.so` |
| Inferência | DSP 29 ms + NN 23 ms por fatia de 375 ms | **2 a 4 ms** |

## O arco

Cada etapa existe porque a anterior bateu num limite que dá para ver na tela:

| Etapa | O que trava | O que a próxima faz |
|---|---|---|
| 1 | a onda inteira não cabe numa decisão | descrever a janela com quatro números |
| 2 | as features dizem *quão alto* e *quão agudo*, nunca *o quê* | **Edge Impulse** |

O último degrau é o ponto da trilha: as features dizem quanta energia existe, não quais
frequências vieram em qual ordem. Tocar "Ei Fioti" de trás para frente dá as mesmas quatro
features. É isso que justifica o MFCC — em vez de a rede neural aparecer como mágica.

> **Nas etapas 1 e 2 não há FFT, MFCC, filtro digital — nem atuador.** São observação pura, de
> propósito. Ver [CLAUDE.md](CLAUDE.md).

## Pinagem do INMP441

| INMP441 | DevKit v1 (turma) | S3 Super Mini |
|---|---|---|
| VDD | 3V3 (nunca 5V) | 3V3 |
| GND | GND | GND |
| L/R | GND (canal esquerdo) | GND |
| SCK | 26 | 5 |
| WS | 25 | 6 |
| SD | 33 | 7 |

## Como rodar as etapas PlatformIO

```bash
cd app36-N-...
pio run -t upload
pio device monitor -b 921600
```

As etapas 1 e 2 usam o **Teleplot** (extensão do VS Code) para o gráfico ao vivo — configure-o
também para **921600**.

Cada etapa traz um segundo `platformio.ini.*` para a outra placa: renomeie-o para
`platformio.ini` (guardando o original com outro nome). **O código em `src/` não muda**: a flag
`PLACA_S3` troca os pinos dentro do `INMP441.hpp`.

## Documentos

| | |
|---|---|
| [PLANO.md](PLANO.md) | o plano completo, com as decisões e o porquê de cada uma |
| [CLAUDE.md](CLAUDE.md) | convenções técnicas — leia antes de mexer no código |
| [docs/coleta-edge-impulse.md](docs/coleta-edge-impulse.md) | Edge Impulse: gravar e exportar |
| [docs/slides-aula-25.1.md](docs/slides-aula-25.1.md) | especificação dos slides da aula do ESP32 |
| [docs/ideias-voz-depois-da-wakeword.md](docs/ideias-voz-depois-da-wakeword.md) | brainstorm: fala → texto depois da wake word, em Edge / Fog / Cloud |
| [app36-5-WakeWord-Android/docs/](app36-5-WakeWord-Android/docs/) | plano, auditoria, coleta e aula do trilho Android |
