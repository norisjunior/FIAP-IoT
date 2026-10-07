# app-2-mic-VOZ_FEATURES — a janela vira quatro números

Etapa 2, e a última antes do Edge Impulse. Junta 4096 amostras (256 ms) e descreve a janela
inteira com quatro **features**: 16 000 números por segundo viram 4.

**Observação, não acionamento.** Nada acende, nada dispara — o aluno olha o Teleplot.

## Pinos

INMP441 igual à etapa 1. Nenhum atuador.

## O código

`src/mic-features.cpp` — uma função por feature, do mesmo jeito que o **app17** faz com o
acelerômetro. `calcMean`, `calcStd` e `calcPtP` têm o mesmo nome e a mesma forma lá e aqui: são
as mesmas features, outro sinal. O `loop()` vira a lista, legível de cima a baixo:

```cpp
  float media  = calcMean(som, N);
  float desvio = calcStd(som, N, media);
  float p2p    = calcPtP(som, N);
  float zcr    = calcZCR(som, N, media);
```

`calcZCR` é a única que não existe no app17 — a 100 Hz do acelerômetro não há conteúdo de alta
frequência para ela enxergar. Em áudio, é a mais interessante das quatro.

| Feature | O que captura |
|---|---|
| `media` | o deslocamento fixo do microfone — perto de zero |
| `desvio` | o quanto o sinal varia: o volume |
| `p2p` | pico a pico, maior menos menor |
| `zcr` | cruzamentos por zero, por segundo |

Todas no **domínio do tempo**. Nenhuma transformada — restrição didática, não limitação.

### `desvio` é o RMS

A onda oscila em torno de zero: a média de uma janela é ~0 no silêncio **e** no grito — é o
`[-3,+3,-3,+3,-3]` da Aula 13, agora de verdade. `calcStd` eleva ao quadrado antes de somar
justamente para positivo e negativo não se cancelarem; depois de subtrair a média, desvio-padrão
e RMS são a mesma conta. Como *valor*, a média não diz nada sobre áudio: ela existe no código
para ser tirada das outras três.

## Uso

```bash
pio run -t upload
```

Abra o **Teleplot** e configure **921600** (não 115200 — é fácil esquecer e achar que travou).

**1. Olhando só o `desvio`:** silêncio fica baixo e plano · **palma** dá um pico estreito e
altíssimo, que cai rápido · voz normal ondula com as sílabas · **voz alta sobe tanto quanto a
palma**. Um número diz *quão alto*, nunca *o quê*.

**2. `sssss` contra `ahhhh`, no mesmo volume:** o `desvio` fica parecido, o `zcr` não — alto no
agudo, baixo no grave. Contar trocas de sinal é o único jeito de falar de frequência sem
transformada, e é o teto do que dá para fazer aqui.

**3. `Ei Fioti` contra qualquer outra palavra, no mesmo volume:** as quatro features empatam.

## Onde isso trava — e é o ponto da trilha

As features dizem *quanta* energia e *quão* agudo. Não dizem **quais frequências, em qual
ordem**. Palavra é sequência de sons, e nenhuma estatística de janela captura ordem: tocar o
áudio de "Ei Fioti" de trás para frente dá exatamente as mesmas quatro features.

É por isso que a etapa 3 usa o Edge Impulse: o MFCC transforma a janela numa **imagem
tempo × frequência**, e aí a ordem aparece.

## Parâmetros e placa

`N` = 4096 → 4096 / 16000 Hz = 256 ms.

Para o **ESP32-S3 Super Mini**: renomeie `platformio.ini.esp32s3` para `platformio.ini` (guarde
o original como `platformio.ini.devkit1`). O código não muda; a flag `PLACA_S3` troca os pinos
para SCK 4 · WS 5 · SD 6.

Sem Wokwi: não existe peça INMP441 no simulador.
