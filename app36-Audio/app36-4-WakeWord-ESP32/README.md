# Wake word no ESP32-S3 com Edge Impulse

A sua wake word, treinada no Edge Impulse, roda **dentro do ESP32-S3**: o INMP441 ouve
o tempo todo, o modelo dá a nota a cada fatia de áudio e, quando uma classe passa do
limiar, o Monitor Serial mostra o nome dela: `>>> WakeWord`.

Você monta o código, mas **não do zero**: ele é o exemplo `esp32_microphone_continuous`
que vem dentro da biblioteca que o Edge Impulse gera para o seu projeto. Você copia o
exemplo e faz **quatro ajustes**:

1. o formato do INMP441 (32 bits, canal esquerdo) e os pinos;
2. a leitura em 32 bits, convertida para 16;
3. mostrar a classe reconhecida;
4. a alocação de memória do ESP32-S3.

O `src/wakeword.ino` desta pasta é o exemplo já ajustado, com o modelo `WakeWord-EiFIOT`
do professor. Cada linha alterada está marcada com `// AJUSTE`.

Por que cada ajuste existe, com um simulador do ganho: abra
[ei-fioti-no-esp32.html](ei-fioti-no-esp32.html) no navegador.

## Hardware

| INMP441 | ESP32-S3 Super Mini |
|---|---|
| VDD | 3V3 (nunca 5V) |
| GND | GND |
| L/R | GND (canal esquerdo) |
| SCK | GPIO 5 |
| WS | GPIO 6 |
| SD | GPIO 7 |

A tela redonda da protoboard não é usada nesta etapa; ela entra no [app36-6](../app36-6-DeskBuddy-ESP32/).

## Roteiro

### 1. Baixar a biblioteca

No Edge Impulse: **Deployment** → **Arduino library** → **Build**. Será baixado um
`.zip`, por exemplo `ei-wakeword-eifiot-arduino-1.0.5-impulse-#1.zip`.

### 2. Descompactar dentro de `lib/`

Apague o `.zip` do professor, coloque o seu em `lib/` e descompacte-o ali mesmo. O
PlatformIO não lê o `.zip`: tem que descompactar. A estrutura deve ficar assim:

```
app36-4-WakeWord-ESP32/
├── lib/
│   ├── ei-<seu-projeto>-arduino-<versão>.zip
│   └── <SEU_PROJETO>_inferencing/
│       ├── examples/
│       ├── src/
│       └── library.properties
├── src/
└── platformio.ini
```

**ATENÇÃO:** o "Extrair tudo" do Windows sugere criar uma pasta com o nome do `.zip`, e
a biblioteca fica um nível abaixo (`lib/ei-...-#1/<SEU_PROJETO>_inferencing/`). Assim o
PlatformIO **não acha** a biblioteca. Apague o final do caminho sugerido para extrair
direto em `lib\`, ou, no terminal, dentro de `lib/`:

```powershell
Expand-Archive .\ei-<seu-projeto>-arduino-<versão>.zip -DestinationPath .
```

A pasta descompactada **não vai para o Git** (está no `.gitignore`); o `.zip` vai.

### 3. Copiar o exemplo para `src/`

Copie

```
lib/<SEU_PROJETO>_inferencing/examples/esp32/esp32_microphone_continuous/esp32_microphone_continuous.ino
```

para `src/wakeword.ino` (substituindo o do professor). O exemplo já vem com o
`#include` certo, com o nome do seu projeto. No do professor:

```cpp
#include <WakeWord-EiFIOT_inferencing.h>
```

### 4. Ajuste 1: formato e pinos do INMP441

O exemplo foi escrito para outro microfone. Na função `i2s_init()`, perto do fim:

| Linha | Como vem | Como fica |
|---|---|---|
| `.bits_per_sample` | `(i2s_bits_per_sample_t)16` | `(i2s_bits_per_sample_t)32` |
| `.channel_format` | `I2S_CHANNEL_FMT_ONLY_RIGHT` | `I2S_CHANNEL_FMT_ONLY_LEFT` |
| `.bck_io_num` | `26` | `5` |
| `.ws_io_num` | `32` | `6` |
| `.data_in_num` | `33` | `7` |

O INMP441 entrega 24 bits dentro de um slot de 32. Lido em 16 bits, o sinal vem lixo.

### 5. Ajuste 2: ler em 32 bits e converter para 16

O modelo espera amostras `int16`, na escala crua (não normalize para `[-1, 1]`). Logo
abaixo de `sampleBuffer`, crie o buffer de 32 bits:

```cpp
static int32_t rawBuffer[sample_buffer_size / 2];
```

Em `capture_samples()`, leia nele (o dobro de bytes) e troque o `* 8` pelo deslocamento:

```cpp
i2s_read((i2s_port_t)1, (void*)rawBuffer, i2s_bytes_to_read * 2, &bytes_read, 100);
...
if (bytes_read < i2s_bytes_to_read * 2) {
...
sampleBuffer[x] = (int16_t)(rawBuffer[x] >> 16);
```

`>> 16` pega os 16 bits de cima, **sem** o `* 8` do exemplo. Aquele ganho era para o
microfone da ESP-EYE; o INMP441 é mais sensível, e com ele a voz passa de 32 767 e dá a volta
para o negativo. O modelo recebe uma onda picotada e responde `ruido`, sem erro nenhum.
Para conferir, rode o `app36-1-mic-RAW` com o mesmo `GANHO = 16` e fale a uns 30 cm: os picos
devem ficar longe de ±32 000.

### 6. Ajuste 3: mostrar a classe reconhecida

O exemplo só imprime as notas de todas as classes. Copie do `wakeword.ino` do professor o
`LIMIAR` do topo e o bloco `// AJUSTE: a cada fatia, mostra a classe` do `loop()`.

Ele pega a classe de **maior nota** e, se passar do `LIMIAR`, mostra o nome dela, como
na detecção de imagem. Todas aparecem, inclusive `ruido` e `desconhecido`: também são
classes que o modelo reconhece. A mesma classe só se repete depois de 1,5 s; uma classe
diferente aparece na hora.

Os nomes vêm da biblioteca, não do código: cada classe que você criar no Edge Impulse
aparece com o próprio nome, com o mesmo firmware.

### 7. Ajuste 4: alocação de memória do ESP32-S3

Sem este ajuste, o código compila, mas o Monitor Serial só mostra:

```
ERR: MFCC failed (-1002)
ERR: Failed to run DSP process (-1002)
ERR: Failed to run classifier (-5)
```

`-1002` quer dizer "sem memória", mas memória sobra (mais de 230 KB livres). Em algumas
fatias de áudio o MFCC pede um bloco de **0 bytes**; no ESP32-S3 com o core 2.0.x esse
pedido devolve `NULL`, e a biblioteca entende como falta de memória.

A biblioteca deixa o sketch trocar as funções de alocação dela (`ei_malloc` e
`ei_calloc`). Copie do `wakeword.ino` do professor o bloco `// AJUSTE: no ESP32-S3 com o
core 2.0.x`, logo abaixo do `LIMIAR`: ele pede sempre pelo menos 1 byte.

### 8. Compilar, gravar e testar

A primeira compilação leva cerca de **3 minutos** (a biblioteca inteira do Edge Impulse).
Dois avisos são esperados: `"EI_PORTING_ARDUINO" redefined` (da biblioteca) e
`'I2S_COMM_FORMAT_I2S' is deprecated` (do exemplo). O que importa é o `[SUCCESS]`.

Grave e abra o Monitor Serial (`pio device monitor`). O programa só começa quando o
monitor abre. Saída esperada:

```
Predictions (DSP: 29 ms., Classification: 23 ms., Anomaly: 0 ms.):
    WakeWord: 0.01172
    desconhecido: 0.04297
    ruido: 0.94531

>>> ruido (0.94531)

>>> WakeWord (0.91406)

>>> ruido (0.89844)
```

## Problemas comuns

| Sintoma | Causa |
|---|---|
| `WakeWord-EiFIOT_inferencing.h: No such file` | biblioteca não descompactada, ou um nível abaixo (passo 2) |
| Notas paradas em `ruido`, mesmo falando | troque `ONLY_LEFT` por `ONLY_RIGHT`, ou confira L/R no GND |
| `MFCC failed (-1002)` | falta o ajuste 4 (passo 7) |
| `Error sample buffer overrun` | o modelo não acompanha o áudio; avise o professor |
| Reconhece mal | o treino foi com o microfone do celular; o INMP441 soa diferente |
