# Desk buddy que dorme de verdade

O [app36-7](../app36-7-DeskBuddy-Toque-ESP32/) crescendo. No app36-7, o toque longo ligava o
**mudo**: o microfone continuava gravando e o código ignorava a wake word. Aqui, o toque longo
põe o ESP32 em **deep sleep**: a tela apaga, o LED fica vermelho no mínimo e ninguém escuta.
Só um toque acorda.

## Estados

| Estado | Como entra | Tela | LED |
|---|---|---|---|
| **Esperando** | ao ligar, ou 6 s sem ouvir a wake word | olhos fechados, com "Zz" | amarelo |
| **Acordado** | **"Ei Fioti"**, **toque curto**, ou acordar do deep sleep | olhos abertos, olham em volta e piscam | verde |
| **Feliz** | toque curto quando já está acordado | "^ ^" em magenta por 1,5 s | verde |
| **Deep sleep** | **toque longo** (2 s) | olhos fechados em vermelho por 1 s; depois, apagada | vermelho, no mínimo |

## Privacidade: quem garante que ele não está ouvindo?

No deep sleep a CPU está desligada e o INMP441 fica sem clock. Sem clock, o microfone entra no
modo de sono dele e não produz áudio nenhum. Não é o código ignorando a fala: não existe fala
para ignorar.

| Nível | Como | Onde |
|---|---|---|
| 1. o código ignora | o microfone grava, o programa descarta | o mudo do app36-7 |
| 2. ninguém escuta | CPU desligada, microfone sem clock | **este app** |
| 3. corte físico | uma chave ou um transistor desliga a alimentação do microfone | o botão de mudo da Alexa: corta o microfone no circuito, e o LED vermelho acende pelo mesmo circuito, sem software no meio |

O nível 3 responde à pergunta que sobra no nível 2: "e quem garante que o firmware dorme
mesmo?".

Na LGPD (art. 6º), dois princípios aparecem aqui:
- **transparência**: o LED avisa quando o dispositivo pode escutar;
- **necessidade**: ele só trata áudio quando precisa.

## O toque longo, passo a passo

1. Os olhos fecham em **vermelho**: o aviso de que ele vai parar de escutar.
2. Espera o dedo sair do botão. Se dormisse com o dedo no botão, acordaria na mesma hora.
3. Depois de 1 s, **apaga a tela**: manda à tela os comandos *display off* e *sleep in*.
4. Deixa o **LED vermelho no brilho mínimo** (1 de 255) e trava o fio de dados dele.
5. Configura o GPIO 4 para acordar o ESP32 e entra em **deep sleep**.

### Por que o LED continua aceso com o ESP32 dormindo

O WS2812 guarda a última cor sozinho, enquanto tiver energia. Só o fio de dados (GPIO 48)
precisa ficar parado em LOW: solto, um ruído nele poderia trocar a cor. O `gpio_hold_en` trava
o pino, e o `gpio_deep_sleep_hold_en` mantém a trava durante o deep sleep.

Vermelho puro acende só um dos três LEDs de dentro do WS2812. Cinza ou violeta acenderiam dois
ou três, e gastariam mais.

### Por que a tela não apaga por inteiro

Esta tela tem 7 pinos e **nenhum para a luz de fundo**: ela está ligada direto no 3V3. Os
comandos apagam a imagem e põem o controlador da tela para dormir, mas a luz de fundo
continua acesa. Num lugar escuro dá para ver um leve brilho.

## Acordar é reiniciar

Do deep sleep, o ESP32 não continua de onde parou: ele **reinicia**, e o `setup()` roda de novo.

- `esp_sleep_get_wakeup_cause()` diz que foi o toque que acordou. Então ele já começa acordado,
  com os olhos abertos e o LED verde.
- O toque que acordou não conta como toque curto nem longo.
- Ele volta a ouvir a wake word uns 2 a 3 s depois, o tempo de carregar o modelo.
- O LED continua travado ao acordar: o `led::iniciar()` solta a trava antes da primeira cor.

## Consumo

Estimativas, para medir em aula com um multímetro em série com a bateria:

| Estado | Consumo estimado | Bateria de 1000 mAh dura |
|---|---|---|
| acordado ou esperando a wake word | ~120–150 mA | ~7 h |
| deep sleep com esta tela | ~20–35 mA, quase tudo luz de fundo | ~1,5 dia |
| deep sleep sem luz de fundo (outra tela, ou um transistor) | ~3–5 mA | uma semana ou mais |

O deep sleep aqui economiza de 4 a 5 vezes, mas a luz de fundo é quem decide o resto.

## O que mudou em relação ao app36-7

| Onde | O quê |
|---|---|
| `src/olhos.hpp` | `dormir()` **novo**, chamado pelo toque longo; o modo mudo saiu |
| `src/led.hpp` | `iniciar()` e `dormir()` **novos**, para o LED atravessar o deep sleep; o estado `MUDO` saiu |
| `src/toque.hpp` | no `iniciar()`, o toque que acordou o ESP32 é ignorado |
| `src/deskbuddy.ino` | começa acordado quando acorda pelo toque; o `loop()` não consulta mais o mudo |
| `platformio.ini` | só o comentário do topo |

Tudo o que é novo está marcado `// SONO`.

## Hardware

O mesmo do app36-7: INMP441 nos GPIOs 5–7, tela nos GPIOs 9–13, TTP223B no GPIO 4 e o LED RGB
da placa no GPIO 48.

O botão touch precisa ficar num dos GPIOs 0 a 21. Só esses pinos (os pinos RTC) conseguem
acordar o ESP32-S3 do deep sleep.

## Usar

```bash
pio run -t upload
pio device monitor
```

**Em deep sleep, a porta COM some.** O USB desliga junto com o chip, e o Monitor Serial cai.
Para gravar de novo, acorde o ESP32 com um toque antes do upload. Ou segure o BOOT, aperte e
solte o RST, e solte o BOOT.

## Para ir além

- **Apagar a luz de fundo também:** um transistor (por exemplo, um MOSFET canal P) entre o 3V3
  e o VCC da tela, comandado por um GPIO. Ao acordar, a tela precisa do `tft.init()` de novo,
  e ele já está no `setup()`.
- **Nível 3 de privacidade:** o mesmo transistor, no VDD do INMP441.

## Se não funcionar

| Sintoma | Causa provável |
|---|---|
| dorme e acorda na mesma hora | o TTP223 está com a saída invertida (`LOW` com toque): troque `HIGH` por `LOW` no `toque.hpp` e o `1` por `0` no `esp_sleep_enable_ext0_wakeup` |
| não acorda com o toque | SIG fora do GPIO 4, ou num pino acima do 21 |
| o LED fica vermelho depois de acordar | faltou o `led::iniciar()` antes da primeira cor |
| o upload falha: porta não encontrada | o ESP32 está em deep sleep: toque para acordar, ou use o BOOT |
| LED verde quando deveria estar vermelho | o LED desta placa usa outra ordem de cores: troque os dois primeiros números no `led.hpp` |
| tela toda preta ou branca | fio da tela trocado: confira SCL 9, SDA 10, DC 11, CS 12, RST 13 |
| reinicia sozinho logo ao ligar | faltou o `-DUSE_HSPI_PORT=1` no `platformio.ini` |
| olhos certos, mas nunca acordam com a voz | é a wake word: teste o app36-4 e veja o ganho (`>> 16`) |
