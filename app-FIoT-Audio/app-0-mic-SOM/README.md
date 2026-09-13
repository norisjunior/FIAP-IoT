# app-0-mic-SOM — material de apoio, fora do roteiro

> **Não entra na aula.** A trilha começa no [app-1-mic-RAW](../app-1-mic-RAW/).
>
> Este projeto existe para responder a **uma** pergunta, se algum aluno fizer:
> *"por que o app-1 precisa daquele `INMP441.hpp`?"* — rode este aqui e a resposta aparece
> sozinha no gráfico.

A versão mais curta possível de ler o INMP441: a biblioteca `<I2S.h>`, que já vem no core do
ESP32, sem cabeçalho próprio.

```cpp
#include <Arduino.h>
#include <I2S.h>

void setup() {
  Serial.begin(115200);
  I2S.setAllPins(26, 25, 33, -1, 33);      // SCK, WS, SD
  I2S.begin(I2S_PHILIPS_MODE, 16000, 32);  // 16000 amostras/s, 32 bits
}

void loop() {
  int som = I2S.read();
  Serial.printf(">som:%d\n", som);
}
```

## Os dois defeitos — que são o motivo de ele existir

**1. O sinal sai picotado.** `I2S.read()` entrega uma amostra por chamada, e cada `printf`
custa muito mais que os 62 µs entre duas amostras a 16 kHz. O microfone continua produzindo
enquanto o ESP32 imprime, e o que não é lido a tempo se perde.

**2. Metade das amostras é lixo.** A `<I2S.h>` não tem modo mono: entrega `L, R, L, R…`. Com um
microfone só, um dos canais chega sempre `0` (ou `1`). É aquele zero alternado na saída.

O `INMP441.hpp` do app-1 conserta os dois: lê em bloco com DMA e pede
`I2S_CHANNEL_FMT_ONLY_LEFT`.

## Placas

| | DevKit v1 | S3 Super Mini |
|---|---|---|
| SCK | 26 | 4 |
| WS | 25 | 5 |
| SD | 33 | 6 |

Para o S3: renomeie `platformio.ini.esp32s3` para `platformio.ini`. O código não muda — a flag
`PLACA_S3` troca os pinos.

Ligação comum: VDD → 3V3 (**nunca** 5V) · GND → GND · L/R → GND.

## Problemas

| Sintoma | Causa |
|---|---|
| Monitor Serial mudo no S3 | falta `-DARDUINO_USB_CDC_ON_BOOT=1` — a Super Mini usa USB nativo |
| só zeros ou lixo constante | canal errado — o L/R deste módulo pode cair no canal oposto |
| placa não dá boot (S3) | pino de I2S em 26–32, que é a flash interna |

Sem Wokwi: não existe peça INMP441 no simulador.
