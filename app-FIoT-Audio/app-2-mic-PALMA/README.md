# app-2-mic-PALMA — a janela vira um número

Etapa 2. Depois de ver a onda inteira na etapa 1, junta 512 amostras (32 ms) e calcula o
**RMS** dessa janela. 16 000 números por segundo viram 31.

**Observação, não acionamento.** Nada acende, nada dispara.

## Pinos

INMP441 igual às etapas 0 e 1. Nenhum atuador.

## O código

`src/mic-janela.cpp` — 34 linhas. Duas contas, uma depois da outra:

```cpp
  // o microfone tem um deslocamento fixo: tira a media primeiro
  float media = 0;
  for (int i = 0; i < N; i++) media += som[i];
  media = media / N;

  // RMS: eleva ao quadrado, tira a media, tira a raiz
  float soma = 0;
  for (int i = 0; i < N; i++) {
    float v = som[i] - media;
    soma += v * v;
  }
  int rms = sqrt(soma / N);
```

Tirar a média é a mesma ideia do `calibrarCentro()` da versão MAX9814
(`iot-app28-mic-voz-max9814-ANALOGICO.txt`), agora recalculada a cada janela.

## Uso

```bash
pio run -t upload
```

Teleplot a 115200, uma série: `rms`.

| Faça | O que aparece |
|---|---|
| silêncio | baixo e quase plano |
| **palma** | pico estreito e altíssimo, que cai rápido |
| voz normal | ondula, sobe e desce com as sílabas |
| voz alta | sobe **tanto quanto** a palma |

## Por que RMS e não a média

A onda sonora oscila em torno de zero: a média de uma janela é sempre ~0, no silêncio **e** no
grito. É o exemplo `[-3,+3,-3,+3,-3]` da Aula 13, agora de verdade. Elevar ao quadrado antes de
somar impede que positivo e negativo se cancelem.

É por isso que a média entra no código só para ser **subtraída** — como valor, ela não diz nada
sobre áudio.

## Onde isso trava

**Um número diz quão alto — nunca diz o quê.** Palma, voz alta, batida na mesa e cadeira
arrastando sobem todos igual. Daí a etapa 3.

## Parâmetros

| | Padrão | |
|---|---:|---|
| `N` | 512 | 512 / 16000 Hz = 32 ms |

Sem Wokwi: não existe peça INMP441 no simulador.
