# Desk buddy com toque e LED

O [app36-6](../app36-6-DeskBuddy-ESP32/) crescendo. A wake word é a mesma, e os olhos também.
O que muda é que agora ele tem um **botão touch** e usa o **LED RGB** que já vem na placa.

O LED mostra o estado, como a luz da Alexa. Nesta versão, o verde quer dizer "acordado". Na
versão com comando de voz, vai querer dizer "pode falar".

## Estados

| Estado | Como entra | Olhos | LED |
|---|---|---|---|
| **Dormindo** | ao ligar, ou 6 s sem ouvir a wake word | fechados, com "Zz" | amarelo |
| **Acordado** | **"Ei Fioti"** ou **toque curto** | abertos, olham em volta e piscam | verde |
| **Feliz** | toque curto quando já está acordado | "^ ^" em magenta por 1,5 s | verde |
| **Mudo** | **toque longo** (2 s) | fechados, vermelhos e parados | vermelho |

Outro toque longo tira do mudo, e ele volta a dormir. No mudo, nem a wake word nem o toque
curto fazem nada.

O mudo desta versão **ignora** a wake word, mas o microfone continua ligado e o modelo
continua rodando. Na versão com comando de voz, mudo vai querer dizer que nada é gravado nem
enviado.

## Hardware

O mesmo do app36-6 (INMP441 nos GPIOs 5–7, tela nos GPIOs 9–13), mais o botão:

| TTP223B (botão touch) | ESP32-S3 Super Mini |
|---|---|
| VCC | 3V3 (nunca 5V: o sinal sai com a tensão do VCC) |
| GND | GND |
| SIG (em alguns, I/O) | GPIO 4 |

O LED RGB já vem soldado na placa, ligado no GPIO 48: não tem nada para ligar.

O TTP223B se calibra ao ser ligado. **Não encoste no botão nesse momento**, senão ele passa a
considerar o dedo como "sem toque".

## O que mudou em relação ao app36-6

| Onde | O quê |
|---|---|
| `src/toque.hpp` | **novo**: separa o toque curto do toque longo |
| `src/led.hpp` | **novo**: uma cor para cada estado |
| `src/olhos.hpp` | trechos `// TOQUE` e `// LED`: o modo mudo, os olhos felizes, e a tarefa da tela lendo o toque e acendendo o LED |
| `src/deskbuddy.ino` | um trecho `// TOQUE`: no mudo, a wake word é ignorada |
| `platformio.ini` | só o comentário do topo. Nenhuma biblioteca nova: o LED usa o `neopixelWrite()` do core |

## Por que o toque é lido na tarefa da tela

O `loop()` só dá uma volta a cada 375 ms: ele espera a fatia de áudio e roda o modelo. Um
toque rápido, de 200 ms, poderia começar e terminar sem o `loop()` ver.

A tarefa da tela, no núcleo 0, dá uma volta a cada quadro (uns 35 ms). Por isso é ela quem lê
o botão, decide o estado e acende o LED. O `loop()` só consulta o `olhos::mudo`.

## Curto ou longo: uma regra, não um modelo

O TTP223B faz a parte difícil, a capacitiva, e entrega um sinal pronto: `HIGH` enquanto o dedo
está no botão. O `toque.hpp` só mede quanto tempo o dedo ficou:

- soltou antes de 2 s: **curto**;
- completou 2 s ainda segurando: **longo**. Ele dispara na hora, com o dedo no botão, para a
  pessoa saber que já pode soltar.

Vale comparar em aula: o toque é reconhecido por uma **regra** escrita à mão; a wake word,
por um **modelo** que aprendeu com exemplos. A regra basta quando dá para escrevê-la. Para
reconhecer "Ei Fioti" no meio de outras falas, não daria.

## Usar

```bash
pio run -t upload
pio device monitor
```

A primeira compilação leva uns 3 minutos, por causa da biblioteca do Edge Impulse.

## Para mexer

| Arquivo | O quê |
|---|---|
| `olhos.hpp` | no topo: `COR`, `COR_MUDO`, `COR_FELIZ`, `ACORDADO_MS`, `FELIZ_MS` |
| `toque.hpp` | `LONGO_MS`, quanto tempo segurar para o toque longo |
| `led.hpp` | as três cores e o brilho (40 de 255; no máximo o LED ofusca) |

## Próximos passos

- **Dormir de verdade**: o toque longo põe o ESP32 em deep sleep, no lugar do mudo. Está no
  [app36-8](../app36-8-DeskBuddy-DeepSleep-ESP32/).
- **Botão de emergência**: um gesto do toque publica um alerta por MQTT (a etapa
  `EMERGÊNCIA` do [PLANO.md](../PLANO.md)).
- **Apertar para falar**: com o comando de voz, gravar só enquanto o dedo está no botão. Ver
  [docs/ideias-voz-depois-da-wakeword.md](../docs/ideias-voz-depois-da-wakeword.md).

## Se não funcionar

| Sintoma | Causa provável |
|---|---|
| entra no mudo sozinho uns 2 s depois de ligar | o módulo entrega `LOW` com toque (saída invertida): troque `HIGH` por `LOW` no `toque.hpp` |
| o toque não faz nada | confira SIG no GPIO 4 e VCC no 3V3; não encoste no botão ao ligar |
| LED vermelho quando deveria estar verde | o LED desta placa usa outra ordem de cores: troque os dois primeiros números no `led.hpp` |
| tela toda preta ou branca | fio da tela trocado: confira SCL 9, SDA 10, DC 11, CS 12, RST 13 |
| reinicia sozinho logo ao ligar | faltou o `-DUSE_HSPI_PORT=1` no `platformio.ini` |
| olhos amarelos em vez de ciano | a tela usa outra ordem de cores: acrescente `-DTFT_RGB_ORDER=TFT_BGR` |
| olhos certos, mas nunca acordam com a voz | é a wake word: teste o app36-4 e veja o ganho (`>> 16`) |
