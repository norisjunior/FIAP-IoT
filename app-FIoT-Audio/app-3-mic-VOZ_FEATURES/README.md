# app-3-mic-VOZ_FEATURES — a janela vira quatro números

Etapa 3. Janela de 4096 amostras (256 ms). Cada janela é descrita por quatro **features**.

## Pinos

INMP441 igual às etapas 0 e 1. Nenhum atuador.

## O código

`src/mic-features.cpp` — uma função por feature, do mesmo jeito que o **app17** faz com o
acelerômetro. `calcMean`, `calcStd` e `calcPtP` têm exatamente o mesmo nome e a mesma forma lá
e aqui: são as mesmas features, outro sinal.

```cpp
float calcStd(int16_t arr[], int n, float media) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += (arr[i] - media) * (arr[i] - media);
  return sqrt(soma / n);
}
```

E o `loop()` vira a lista de features, legível de cima a baixo:

```cpp
  float media  = calcMean(som, N);
  float desvio = calcStd(som, N, media);
  float p2p    = calcPtP(som, N);
  float zcr    = calcZCR(som, N, media);
```

`calcZCR` é a única que não existe no app17 — a 100 Hz do acelerômetro não há conteúdo de alta
frequência para ela enxergar. Em áudio, é a feature mais interessante das quatro.

## As features

| Feature | O que captura |
|---|---|
| `media` | o deslocamento fixo do microfone — perto de zero |
| `desvio` | o quanto o sinal varia: o volume |
| `p2p` | pico a pico, maior menos menor |
| `zcr` | cruzamentos por zero, por segundo |

Todas no **domínio do tempo**. Nenhuma transformada — é restrição didática, não limitação.

## Uso

```bash
pio run -t upload
```

Teleplot a 115200, quatro séries. Dois testes valem a aula:

**1. `sssss` contra `ahhhh`, no mesmo volume:**

| | `desvio` | `zcr` |
|---|---|---|
| `sssss` (agudo) | parecido | **alto** |
| `ahhhh` (grave) | parecido | **baixo** |

Contar trocas de sinal é o único jeito de falar de frequência sem transformada — e é o teto do
que dá para fazer aqui.

**2. `FIOT` contra `EMERGÊNCIA`, no mesmo volume:** as quatro features ficam praticamente
iguais.

## Onde isso trava — e é o ponto da trilha

As features dizem *quanta* energia e *quão* agudo. Não dizem **quais frequências, em qual
ordem**.

Palavra é sequência de sons, e nenhuma estatística de janela captura ordem: a janela inteira
vira um número só. Inverter o áudio de "FIOT" dá exatamente as mesmas quatro features.

É por isso que a etapa 4 usa o Edge Impulse: o MFCC transforma a janela numa **imagem
tempo × frequência**, e aí a ordem aparece.

## Parâmetros

| | Padrão | |
|---|---:|---|
| `N` | 4096 | 4096 / 16000 Hz = 256 ms |

Sem Wokwi: não existe peça INMP441 no simulador.
