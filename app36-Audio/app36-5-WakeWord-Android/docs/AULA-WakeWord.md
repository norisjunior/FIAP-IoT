# Wake Word no Celular
### Do microfone ao acionamento, com Edge Impulse e Flutter

---

## 1. O problema

- Um assistente precisa ouvir **o tempo todo** sem mandar áudio para a nuvem
- Não é reconhecimento de fala: é detectar **uma** palavra
- Restrições reais:
  - Rodar no dispositivo, sem internet
  - Consumir pouca CPU e bateria
  - Errar pouco — falso positivo irrita mais que falso negativo

> **Nosso caso:** detectar "Ei Fioti" e acionar a leitura do acelerômetro

---

## 2. Som vira número

- Microfone → tensão contínua → **amostragem**
- **16.000 amostras por segundo** (16 kHz)
  - Nyquist: captura até 8 kHz, suficiente para voz
  - Telefonia usa 8 kHz; música usa 44,1 kHz
- Cada amostra: inteiro de **16 bits** com sinal (−32768 a +32767)
- 1 segundo de áudio = 32 KB de números

```
[0, 152, -89, 1204, 980, -3011, ...]
```

---

## 3. Por que não jogar a onda crua na rede

- 1,5 s a 16 kHz = **24.000 números** de entrada
- A mesma palavra nunca gera a mesma onda: volume, tom, distância mudam tudo
- A informação que importa não está na onda, está no **espectro ao longo do tempo**

**MFCC** — Mel-Frequency Cepstral Coefficients
- Fatia o áudio em quadros curtos (~20 ms)
- Calcula o espectro de cada quadro
- Comprime em escala **Mel**, que imita a audição humana

> **24.000 amostras → 975 características.** A rede fica 25x menor.

---

## 4. Janela deslizante

- O usuário fala quando quer — não existe "botão de começar"
- Solução: uma janela que anda no tempo

| Parâmetro | Valor | Por quê |
|---|---|---|
| Janela | 1500 ms | "Ei Fioti" inteiro cabe |
| Passo | 500 ms | 2 avaliações por segundo |
| Taxa | 16 kHz | = 24.000 amostras por janela |

- Cada 500 ms: pega os últimos 1,5 s, extrai MFCC, classifica
- Uma fala é avaliada **3 vezes** — dá margem para errar uma

---

# [ SEUS SLIDES 6 A 19 — EDGE IMPULSE ]

Coleta · Impulse · MFCC · Treino · Model testing · Deployment
*(já prontos no arquivo, não mexer)*

---

## 5. O caminho do áudio

```
  microfone
      |  blocos de PCM 16 bits, 16 kHz
      v
  _onChunk()                      detector.dart:103
      |
      v
  buffer circular (24.000)        detector.dart:111
      |  a cada 8.000 amostras = 500 ms
      v
  _fillWindow()                   detector.dart:133
      |  janela linear de 1,5 s
      v
  _ffi.classify(_window)          detector.dart:144   <-- AQUI
      |  Dart -> memória nativa
      v
  ww_classify()                   wakeword_ffi.cpp:41
      |
      +--> MFCC        24.000 amostras -> 975 features
      +--> rede        975 features -> 3 probabilidades
      |
      v
  [WakeWord, desconhecido, ruido]     2 a 4 ms
      |
      v
  limiar + cooldown               detector.dart:147
      |
      v
  ação: capturar acelerômetro     main.dart
```

> Tudo antes da linha 144 é **preparar o buffer**.
> Tudo depois é **interpretar o resultado**.

---

## 6. A ponte nativa

O Edge Impulse entrega C++. O Flutter fala Dart. Precisamos de uma ponte.

```cpp
// wakeword_ffi.cpp
static const int16_t *g_samples = nullptr;

// ATENÇÃO: o MFCC espera a escala int16 crua, NÃO [-1,1]
static int ww_get_data(size_t offset, size_t length, float *out) {
    for (size_t i = 0; i < length; i++)
        out[i] = static_cast<float>(g_samples[offset + i]);
    return 0;
}

WW_EXPORT int ww_classify(const int16_t *samples, int count,
                          float *out_scores, int out_len) {
    g_samples = samples;
    signal_t signal;
    signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
    signal.get_data = &ww_get_data;

    ei_impulse_result_t result = {0};
    run_classifier(&signal, &result, false);

    for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++)
        out_scores[i] = result.classification[i].value;
    return 0;
}
```

> Normalizar o áudio aqui faria o modelo receber lixo — **sem erro, sem aviso**

---

## 7. O lado Dart: FFI

```dart
// wakeword_ffi.dart
final lib = ffi.DynamicLibrary.open('libwakeword.so');

final _classify = lib.lookupFunction<
    ffi.Int32 Function(ffi.Pointer<ffi.Int16>, ffi.Int32,
                       ffi.Pointer<ffi.Float>, ffi.Int32),
    int Function(ffi.Pointer<ffi.Int16>, int,
                 ffi.Pointer<ffi.Float>, int)>('ww_classify');

List<double> classify(Int16List window) {
  _inBuf.asTypedList(windowSamples).setAll(0, window);
  final rc = _classify(_inBuf, windowSamples, _outBuf, labels.length);
  if (rc != 0) throw StateError('falhou: $rc');
  return List<double>.from(_outBuf.asTypedList(labels.length));
}
```

- Buffers alocados **uma vez**, reusados a cada inferência
- O app lê janela, taxa e labels **do próprio modelo** — nada fixo no código

---

## 8. Buffer circular

O microfone entrega blocos; precisamos sempre dos **últimos 1,5 s**.

```dart
// detector.dart — O(1) por amostra
for (final s in samples) {
  _ring[_writeIdx] = s;
  _writeIdx = _writeIdx + 1 == len ? 0 : _writeIdx + 1;
  if (_filled < len) _filled++;
  _sinceLastRun++;

  if (_filled == len && _sinceLastRun >= _strideSamples) {
    _sinceLastRun = 0;
    _run();                       // a cada 8000 amostras = 500 ms
  }
}

// Desenrola o círculo: mais antiga primeiro
void _fillWindow() {
  final tail = len - _writeIdx;
  _window.setRange(0, tail, _ring, _writeIdx);
  _window.setRange(tail, len, _ring, 0);
}
```

> Deslocar o array inteiro a cada amostra seriam **384 milhões de operações por segundo**

---

## 9. Decidir e acionar

A rede devolve probabilidades. Virar "sim ou não" é decisão de projeto.

```dart
void _run() {
  _fillWindow();
  final scores = _ffi.classify(_window);       // 2 a 4 ms

  if (scores[wakeIndex] >= threshold) _hits++; else _hits = 0;

  if (_hits >= hitsNeeded &&
      DateTime.now().difference(_lastTrigger) > cooldown) {
    _lastTrigger = DateTime.now();
    _hits = 0;
    _triggers.add(_lastTrigger);               // dispara a ação
  }
}
```

```dart
// main.dart — o shell reage
void _onWakeWord() {
  setState(() => _index = 1);      // vai para o gráfico
  _captureRequests.add(null);      // captura 5 s do acelerômetro
}
```

- **cooldown** evita disparo repetido na mesma fala
- **hitsNeeded** exige consistência no tempo, não um pico sortudo

---

## 10. Números e armadilhas

| Métrica | Valor |
|---|---|
| Acurácia (validação) | 93,1% |
| Acurácia (teste) | 86,7% |
| F1 da wake word | 0,92 |
| Inferência no Galaxy A52 | **2 a 4 ms** |
| Tamanho da lib nativa | 1,5 MB |

**O limiar depende do ambiente:**

| Ambiente | Limiar que funcionou |
|---|---|
| Sala fechada | 0,90 |
| Bar, multidão | 0,79 |

> O conserto é **dado**, não parâmetro — gravar a wake word com ruído real.

### O que não funciona de primeira

- **Normalizar o áudio** — o MFCC do Edge Impulse espera escala int16 crua
- **Buffer linear** — deslocar 24.000 amostras a cada amostra trava o app
- **Só a classe positiva** — sem uma classe `desconhecido` rica, tudo vira wake word
  - Gravamos "fiot", "fiap", "ei" sozinho e o vocabulário da aula **de propósito**
- **Confiar na validação** — o número honesto é o do *test set*
- **Um limiar universal** — não existe; deixe ajustável

### Próximo passo
Mesmo modelo, mesmo MFCC, outro alvo: **ESP32 + INMP441**


---

# NÃO É SLIDE — INSTRUÇÕES DE MONTAGEM

Cole o bloco abaixo no Copilot do PowerPoint, com o arquivo
`IoT - Aula 27 - TinyML - EI - Audio.pptx` aberto.

---

```
Estou editando esta apresentação e quero ADICIONAR slides novos sem alterar
nada do que já existe.

ESTADO ATUAL DO ARQUIVO (21 slides):
- 1 a 2: capa e abertura institucional
- 3: Agenda
- 4: "Audio" (divisória, layout "1_Slide de título")
- 5: "Audio – Wake Word" — materiais necessários
- 6 a 19: "Audio – Wake Word" — prints do Edge Impulse, um por slide
- 20: "HANDS ON!" (divisória)
- 21: Copyright

REGRAS OBRIGATÓRIAS:
1. NÃO altere os slides 1, 2, 3, 4, 5, 20 e 21.
2. NÃO altere os slides 6 a 19 — são prints do Edge Impulse já posicionados.
3. NÃO altere o slide mestre, os layouts nem o tema.
4. Todo slide novo deve usar o layout "Título e conteúdo", o mesmo dos
   slides 6 a 19.
5. FONTES — não substitua por nenhuma outra:
   - Títulos: Fira Sans ExtraBold, 40 pt (vem do slide mestre)
   - Corpo: Fira Sans (vem do layout "Título e conteúdo")
   O tema declara Calibri como fallback, mas ele NÃO é usado. Se você
   aplicar Calibri em qualquer lugar, estará errado.
   Não defina tamanhos manualmente no corpo — herde do layout.
6. Mantenha o título "Audio – Wake Word" em todos os slides novos, igual
   aos existentes. O assunto específico vai na primeira linha do corpo,
   em negrito.
7. Use as cores de destaque do tema: #29AF8C (principal), #3D9CCC
   (secundária), #97BE49 e #7C60C6. Não introduza cores novas.

ONDE INSERIR:
- Bloco TEORIA (4 slides): inserir APÓS o slide 5, antes do slide 6.
- Bloco CÓDIGO (6 slides): inserir APÓS o slide 19, antes do "HANDS ON!".

FORMATAÇÃO DO CONTEÚDO NOVO:
- Blocos de código e o diagrama de fluxo: Consolas, 11 a 13 pt, fundo
  cinza-claro, SEM tradução e SEM quebra automática de linha. Esta é a
  ÚNICA exceção ao Fira Sans.
- Tabelas: usar o estilo de tabela padrão do tema, cabeçalho em #29AF8C.
- Citações marcadas com ">" no texto de origem viram uma caixa de destaque
  com barra lateral em #29AF8C.
- No slide "O caminho do áudio", destaque a linha
  "_ffi.classify(_window)  detector.dart:144" em #29AF8C e negrito.
- Não gere imagens nem ícones decorativos.

O conteúdo dos 10 slides novos está no arquivo Markdown que vou colar em
seguida. Cada "---" separa um slide; cada "##" é o assunto daquele slide.
```

---

## Mapa depois da inserção

| Slides | Conteúdo |
|---|---|
| 1 – 5 | Já existentes |
| **6 – 9** | **Novo — teoria de áudio** |
| 10 – 23 | Edge Impulse (eram 6 – 19) |
| **24 – 29** | **Novo — caminho do áudio e código** |
| 30 – 31 | HANDS ON! e Copyright |

## Se o Copilot não obedecer

O Copilot tende a reescrever texto e trocar fontes. Se acontecer:

- Insira os slides novos em branco manualmente, com **Novo Slide → Título e conteúdo**
- Cole o texto com **Ctrl+Shift+V** (mantém a formatação de destino)
- Confira que o título ficou Fira Sans ExtraBold e o corpo Fira Sans
- Aplique Consolas só nos blocos de código
- Leva ~15 min e garante que os slides 6–19 não sejam tocados
