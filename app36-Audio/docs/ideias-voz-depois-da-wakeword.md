# Ideias: fala → texto depois da wake word

> **Status:** brainstorm de 04/11/2026, nenhum código ainda. Ponto de partida: o
> [app36-6-DeskBuddy-ESP32](../app36-6-DeskBuddy-ESP32/) já reconhece "Ei Fioti" (0,99) e
> abre os olhos por 6 s. A pergunta é o que fazer nessa janela acordada, e serve de demo
> para a aula de **Cloud / Fog / Edge**.

## O limite do ESP32

O Whisper **não cabe** no ESP32. O menor modelo, o *tiny*, tem uns 39 M de parâmetros
(≈ 75 MB); a Super Mini tem 4 MB de flash e 2 MB de PSRAM (hoje desligada no `.ini`).

O ESP32 fica com o que faz bem: **ouvir a wake word, gravar o comando e agir**. Fala livre →
texto acontece na Fog ou na Cloud.

## Um caminho, três camadas

```text
EDGE (ESP32)                         FOG ou CLOUD                       EDGE
"Ei Fioti" → olhos abrem → grava ─▶  fala → texto → entender → decidir ─▶ agir / responder
             (já existe)    3–4 s                                         MQTT, olhos, tela, voz
```

A ideia da aula: as etapas são sempre as mesmas, e o que muda é **em que camada cada uma
roda**.

| Etapa | Edge (só o ESP32) | Fog (notebook da sala) | Cloud |
|---|---|---|---|
| Fala → texto | **comandos fixos** treinados no Edge Impulse ("liga", "desliga"...), uns 5 a 10 | **faster-whisper** num container (mesma biblioteca do `C:\Projects\WhisperAudio`) | API de STT (Whisper, Google, Azure) |
| Entender | a classe já é o comando | Node-RED ou n8n por palavra-chave; Ollama para pergunta livre | LLM na nuvem |
| Agir | GPIO local, expressão dos olhos | MQTT pelo Mosquitto → app07/app08 (LED, buzzer, relé) | APIs externas (previsão do tempo, agenda) |
| Responder | olhos felizes ou confusos | texto na tela redonda; voz com Piper (TTS local, tem pt-BR) | TTS na nuvem |
| Sem internet | funciona | funciona | não funciona |
| Vocabulário | fechado | aberto | aberto |
| Demora (ordem de grandeza) | ~50 ms | 1–3 s com o *small* em CPU | 1–3 s + rede |
| Chave de API | — | fica no notebook | **fica no notebook, nunca no ESP32** |

## O que já existe no repositório

| Peça | Onde | Serve para |
|---|---|---|
| Wake word + olhos | `app36-Audio/app36-6-DeskBuddy-ESP32` | o lado Edge inteiro |
| Mosquitto, Node-RED, n8n, InfluxDB, Grafana | `FIAP-IoT-eval/IoT-platform` (Compose; há variante para RPi) | a Fog: basta somar um container de STT |
| ESP32 → MQTT → n8n → Ollama | `FIAP-IoT-eval/LLM-IoT-platform` | LLM local na Fog |
| Chatbot Cloud × Edge | `FIAP-IoT-eval/app19-AI-LLM-Cloud_e_Edge` | a comparação LLM Cloud × local |
| faster-whisper | `C:\Projects\WhisperAudio` (`transcript.py`) | o STT, já testado em português |
| Atuadores MQTT | `app07_mqtt_subscriber_atuador`, `app08_..._LED_e_BUZ` | quem obedece ao comando |
| Segurança MQTT | `app37`, `app38`, `app39` | onde ficam as credenciais |

## Demo para a aula

Três frases, cada uma resolvida numa camada:

1. **"Liga a luz"**: Edge (comando treinado) ou Fog (palavra-chave). Vira MQTT para o
   app08. Não precisa de internet.
2. **"Como está o tempo?"**: a Fog reconhece a intenção e só ela vai à Cloud, numa API de
   previsão. A resposta aparece na tela redonda. Não precisa de LLM.
3. **"Me explica o que é MQTT"**: vai para o LLM. Compara Ollama na Fog × LLM na Cloud, em
   qualidade e em demora.

Regra do desenho: **resolver o mais perto possível e subir só o que precisa**.

Para a demora ficar visível: enquanto espera, os olhos fazem cara de "pensando" e a tela
mostra quantos ms cada camada levou. No Node-RED, trocar o STT da Fog pelo da Cloud é
religar um fio: dá para comparar ao vivo.

## Pontos de engenharia

- **Quando parar de gravar.** Simples: tempo fixo de 3–4 s com os olhos abertos. Melhor:
  parar no silêncio, usando o RMS do app36-2 como VAD (a etapa 2 ganha um uso real).
  Ou **apertar para falar**: gravar só enquanto o dedo está no botão touch do app36-7.
- **Memória.** 4 s × 16 kHz × 16 bits = 128 KB. Com o quadro da tela (57 KB) e o Edge
  Impulse, aperta a RAM interna. Opções: ligar a PSRAM ou mandar o áudio em pedaços
  enquanto grava.
- **Como enviar o áudio.**
  - HTTP POST com o `.wav` para a Fog: é por onde começar (Node-RED recebe com `http in`).
  - WebSocket em streaming: menos demora, mais complexo.
  - MQTT funciona, mas não foi feito para mensagens de 128 KB.
- **Ganho.** O Whisper tolera volume baixo, não tolera estouro. O `>> 16` já resolve.
- **Inferência durante a gravação.** Pausar a wake word enquanto grava o comando.
- **Português.** Whisper *tiny* e *base* erram bastante; *small* é o mínimo razoável.
- **Uma frase ou duas.** "Ei Fioti" → olhos abrem → comando (duas etapas, mais simples) ou
  "Ei Fioti, liga a luz" numa frase só (mais natural, exige gravar logo após a wake word).
- **Falar de volta.** Hoje não há alto-falante. Por ordem de esforço:
  1. texto na tela redonda (sem hardware novo);
  2. a Fog toca o áudio no notebook;
  3. MAX98357A + alto-falante no segundo I2S do S3. O microfone usa a porta 1; a porta 0
     está livre.
- **Segurança e privacidade.** Chave de API nunca no dispositivo (liga com app37–39).
  Mandar áudio da sala para a nuvem é argumento de LGPD a favor da Fog.

## Alternativas descartadas (por enquanto)

- **ESP-SR MultiNet (Espressif):** até 200 comandos escritos em texto, direto no S3. Mas
  não tem português, exige o core 3.x (o repositório está no 2.0.17) e mais memória.
- **xiaozhi-esp32 (projeto aberto):** é exatamente este desenho (wake word no S3, áudio por
  WebSocket para servidor de STT + LLM + TTS). Não para usar, mas para mostrar que é
  arquitetura de mercado.

## Por onde começar: Fog primeiro

1. Depois da wake word, o ESP32 grava 4 s e manda o `.wav` por HTTP POST.
2. Um container com faster-whisper entra no Compose do `IoT-platform`, ao lado do Mosquitto e
   do Node-RED.
3. O Node-RED procura palavras-chave e publica MQTT para os atuadores. O que não reconhece
   vai para o Ollama.
4. O texto da resposta volta e aparece na tela redonda, entre os olhos.

Depois:
- **Cloud:** trocar dois nós no Node-RED.
- **Edge:** comandos fixos, que é o `app-5-mic-ASSISTENTE` do [PLANO.md](../PLANO.md).

## Decisões em aberto

- [ ] Nome e número do app novo (um `app36-8-...` derivado do app36-7, que já tem toque e LED?).
- [ ] Janela fixa ou VAD pelo RMS.
- [ ] HTTP POST ou WebSocket.
- [ ] Ligar a PSRAM da Super Mini ou mandar em pedaços.
- [ ] Como responder: só tela, notebook, ou comprar o MAX98357A.
- [ ] Modelo do faster-whisper na Fog (*small* × *medium*) e se roda no notebook ou no RPi.
- [ ] Qual STT/LLM de Cloud usar na comparação.
- [ ] API de previsão do tempo para a frase 2.
- [ ] Botão touch como **botão de emergência** (gesto → alerta MQTT; a etapa EMERGÊNCIA do PLANO).
- [ ] O LED do app36-7 já tem verde = "esperando o comando": falta um estado "pensando" (azul?) enquanto a Fog responde.
