# Edge Impulse — do dataset ao firmware do ESP32

Este é o **bloco 2** da trilha: entre observar o sinal (etapas 1 e 2) e inferir na borda
(etapas 3 a 5).

> **A boa notícia: o modelo já existe.** O projeto `WakeWord-EiFIOT` foi gravado, treinado e
> validado no trilho Android — 93,1% de validação, F1 0,92 na wake word. As etapas 3 a 5 do
> ESP32 usam **esse mesmo projeto**, só com outro Deployment. Ninguém precisa gravar nada para
> a aula acontecer.

| | |
|---|---|
| Wake word | **"Ei Fioti"** |
| Labels | `WakeWord` · `desconhecido` · `ruido` |
| Janela | **1500 ms**, stride **500 ms**, 16 kHz |
| Processing / Learning | Audio (MFCC) → Classification |

---

## 1. Se você só quer rodar no ESP32

Pule a coleta. No Studio, projeto `WakeWord-EiFIOT`:

**Deployment → Arduino library → Quantized (int8) → Build.** Baixa um `.zip`.

> **Int8 no ESP32, float32 no Android.** O Android roda `Unoptimized (float32)` porque tem RAM
> de sobra; no ESP32 a RAM é o recurso escasso. Se mesmo em int8 não couber, baixe a janela de
> 1500 para 1000 ms e retreine — "Ei Fioti" tem 3 sílabas e cabe em 1 s, ao contrário de
> EMERGÊNCIA (ver [app-6](../app-6-mic-EMERGENCIA/)).

Descompacte dentro da pasta da etapa:

```text
app-3-mic-FIOT/
└── lib/
    └── WakeWord-EiFIOT_inferencing/     <- o conteúdo do .zip
        ├── src/
        └── library.properties
```

É a mesma convenção do `app33-MagicWand` no repositório principal. A biblioteca é grande e
**é versionada mesmo assim**.

O `platformio.ini` da etapa traz `board_build.partitions = huge_app.csv` — sem isso o binário
estoura a partição padrão de 1,3 MB.

## 2. Se você vai gravar um dataset novo

O protocolo de gravação está em
[android-wakeword/ETAPA1-COLETA.md](../android-wakeword/ETAPA1-COLETA.md) — quantas amostras
por classe, como variar a voz, como usar o **Split sample** para 60 s virarem 25 amostras. Foi
esse roteiro que produziu o modelo atual; não há um segundo procedimento.

O que vale repetir aqui, porque é onde todo mundo erra:

| | |
|---|---|
| `ruido` e `desconhecido` não são enchimento | um modelo treinado só com a palavra-alvo **sempre** responde ela: não tem alternativa para escolher |
| Sample length 2000 ms, janela 1500 ms | a amostra **nunca** pode ser menor que a janela |
| Pelo menos 3 pessoas | um modelo treinado só com a sua voz reconhece só a sua voz |
| Segmento do split = janela do impulse | precisam ser o mesmo número |

## 3. O risco que precisa ser dito em aula

**O dataset foi gravado com o microfone do celular, e a etapa 3 infere com o INMP441.**
Resposta em frequência, ganho e distância são diferentes. Isso se chama **descasamento de
domínio**, e é exatamente a lição da Aula 14: *o dado de treino precisa parecer o dado de
operação*.

Na prática, para uma wake word de sala, costuma funcionar — o MFCC é razoavelmente robusto e o
augmentation (ligado no treino atual) ajuda. Mas é uma aposta consciente, não um detalhe.

O trilho Android já mediu o preço disso, e o número é honesto: o limiar que serve numa sala
fechada (**0,90**) não é o que serve num ambiente barulhento (**0,79**), porque a classe
`WakeWord` foi gravada quase toda em ambiente controlado. O padrão fixado no app é **0,85**.

Se a taxa de acerto no ESP32 decepcionar, em ordem de custo:

1. ajustar o `GANHO` do `INMP441.hpp` para o sinal ficar na mesma faixa do treino;
2. mais amostras de `ruido` gravadas **na sala onde a demo vai acontecer**;
3. ~30 amostras de "Ei Fioti" com ruído de fundo real, e retreinar (é a melhoria já pendente na
   [AUDITORIA.md](../android-wakeword/AUDITORIA.md));
4. só então: gravar amostras com o próprio INMP441 e subir no Studio.

## 4. A armadilha que já custou tempo no Android

O MFCC do Edge Impulse espera a escala **int16 crua**, não `[-1, 1]`. O `int16_to_float` do SDK
só faz cast. Normalizar antes **quebra o modelo em silêncio**, sem erro nenhum — a inferência
roda, os números saem, e a classificação é lixo.

Vale igual no ESP32: o `INMP441::ler()` já entrega int16, então é só passar adiante.
