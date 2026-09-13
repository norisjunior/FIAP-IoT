# Instruções para o PowerPoint — IoT Aula 25.1

Arquivo alvo: `IoT - Aula 25.1 - EI - Áudio.pptx`
(em `C:\Users\noris\OneDrive\Aulas\FIAP\FIAP-Slides-CPs\IoT\`)

## Como usar

1. Abra o PowerPoint com o assistente.
2. Cole o **Bloco fixo** uma vez, no início da conversa.
3. Depois cole **uma seção por vez, na ordem**. Confira antes de seguir.

> As seções usam como âncora o **título da divisória**, não o número do slide.

## O princípio desta aula

Turma de **IA**, não de processamento de sinais. O bloco de áudio existe só para o aluno
**bater numa parede** — e a parede é o slide 2.4. Tudo antes dele é rampa; tudo depois é
Edge Impulse.

Proporção alvo: **6 slides de sinal, 7 de Edge Impulse.**

Ganho, 32 bits, Nyquist e "por que não tem biblioteca" **não são slides**. Estão no
apêndice *Se perguntarem*, no fim deste arquivo, para você ter a resposta pronta — não para
ocupar tela.

---

## Estado atual do arquivo

**8 slides**, esqueleto herdado do deck do Magic Wand. Nada aplicado ainda.

| Slide | O que é | Situação |
|---|---|---|
| 1–2 | capa e capa da disciplina | manter |
| 3 | Agenda — ainda diz "Aplicação: **Varinha mágica**" | **Seção 0 corrige** |
| 4 | divisória **Áudio** | âncora |
| 5 | (Far/Extreme) Edge AI — reconhecimento de voz | manter |
| 6 | Materiais necessários | **Seção 0 detalha** |
| 7 | divisória **HANDS ON!** | âncora |
| 8 | encerramento | manter |

Slides novos entram **entre o slide 6 e a divisória HANDS ON!**.
Resultado previsto: **21 slides**.

---

## Bloco fixo

> Cole isto primeiro, sempre.

```
Você vai construir os slides do arquivo
"IoT - Aula 25.1 - EI - Áudio.pptx".

CONTEXTO
- Disciplina de IoT, graduação FIAP, 2º ano.
- Os alunos JÁ cursaram uma disciplina de Machine Learning.
- Esta aula fecha a trilha de TinyML: depois de gestos com acelerômetro,
  agora reconhecimento de PALAVRA com microfone.
- A aplicação: a palavra FIOT acorda o dispositivo (LED azul) e a palavra
  EMERGÊNCIA toca um buzzer.

ESTA É UMA TURMA DE IA, NÃO DE PROCESSAMENTO DE SINAIS
- A parte de áudio é RAMPA, não conteúdo. Ela existe para chegar rápido
  na conclusão de que features estatísticas não distinguem palavras.
- NÃO faça slides sobre: bits por amostra, ganho, teorema de Nyquist,
  formato do barramento I2S, bibliotecas de sensor. Se o assunto
  aparecer, resolva em UMA FRASE dentro de um slide existente.
- Dois terços da aula são Edge Impulse.

ONDE ESTA AULA FICA NA TRILHA
- Aula 12: sinais temporais, frequência de amostragem, Nyquist.
- Aula 13: janela e features (média, desvio-padrão, RMS, pico a pico).
- Aula 14: dataset, rótulo, protocolo de coleta, efeito de borda, FFT
  apresentada como "próximo nível".
- Aula 24: o que é o Edge Impulse e o Studio.
- Aula 25: Magic Wand - gestos, exportar biblioteca, rodar no ESP32.
- Aula 25.1 (ESTA): o mesmo pipeline com ÁUDIO.

NÃO REPITA (já viram, é perda de tempo)
- o que é o Edge Impulse e como navegar no Studio (Aula 24)
- como exportar a biblioteca Arduino e incluir no PlatformIO (Aula 25)
- o que é janela, o que é feature, como se calcula média e desvio
  (Aula 13) - aqui só se APLICA ao áudio
- rede neural, treino, acurácia, matriz de confusão (vieram de ML)

O FOCO
- estatística de janela descreve INTENSIDADE, nunca ORDEM - e palavra é
  ordem. É por isso que o MFCC existe.
- arquitetura vale tanto quanto acurácia: a máquina de estados derruba o
  falso positivo sem tocar no modelo.

ESTILO
- Português do Brasil.
- No máximo 5 marcadores por slide. Frases curtas, sem parágrafo.
- O slide apoia a fala, não a substitui.
- Mantenha o template visual e a identidade do arquivo atual.
- Em cada slide, escreva as NOTAS DO APRESENTADOR com o que falar.

RECURSOS VISUAIS
- TODO slide precisa de um elemento visual: diagrama, tabela, esquema ou
  destaque gráfico. Slide só com marcadores é exceção.
- Diagramas com FORMAS do PowerPoint. Sem clipart, sem banco de imagem.
- Ao comparar duas coisas, coloque lado a lado no mesmo slide.
- Havendo sequência, use setas, numere as etapas e anime por etapas.
- Prefira um exemplo concreto (um número real, uma curva) a texto.

CÓDIGO DE CORES - o mesmo da trilha, com duas convenções de áudio
- AZUL: medição do sensor -> aqui, a ONDA CRUA
- LARANJA: valor calculado -> aqui, QUALQUER feature (RMS, ZCR, MFCC)
- CINZA: metadado, coisa descartada
- VERDE: detectado / dentro da faixa
- VERMELHO: falhou, saturou, não distingue

COMO VAMOS TRABALHAR
- Vou enviar uma seção por vez.
- Cada seção diz onde inserir, usando o título da divisória como âncora.
- Crie exatamente os slides pedidos. Não invente slides a mais.
```

---

## Mapa das seções

| Seção | Título | Slides |
|---:|---|---:|
| 0 | Ajustes no que já existe | edita 2 |
| 1 | O microfone | 2 |
| 2 | Da janela à parede | 4 |
| 3 | Edge Impulse: coletar e treinar | 4 |
| 4 | FIOT: o dispositivo acorda | 3 |

---

## Seção 0 — Ajustes no que já existe

```
SEÇÃO 0. Não crie slides novos. Edite dois slides existentes.

--------------------------------------------------------------------
SLIDE 3 (Agenda)

Troque "Aplicação: Varinha mágica" por:
  "Aplicação: assistente de voz de emergência"

ACRESCENTE abaixo, 3 itens:
  - Por que áudio quebra o que funcionava com o acelerômetro
  - Treinar a palavra FIOT no Edge Impulse
  - Acordar o dispositivo e disparar o alarme

--------------------------------------------------------------------
SLIDE 6 (Materiais necessários)

Troque "Sensor para captação de áudio" por uma tabela:

| Item | Detalhe |
| ESP32 | DevKit v1 |
| Microfone INMP441 | MEMS digital, I2S |
| LED azul | indica o dispositivo ACORDADO |
| Buzzer | dispara na palavra EMERGÊNCIA |

ACRESCENTE ao lado uma FIGURA DE LIGAÇÃO com formas: retângulo "INMP441"
com 6 pinos rotulados ligado a um retângulo "ESP32".
  VDD -> 3V3   (rótulo "nunca 5V" em VERMELHO)
  GND -> GND
  L/R -> GND
  SCK -> 26
  WS  -> 25
  SD  -> 33

NOTAS: mostrar o módulo na mão. Custa o mesmo que o sensor de som
analógico do 1º ano e é outro mundo. Não entrar em I2S.
```

---

## Seção 1 — O microfone

```
SEÇÃO 1. Insira 2 slides depois do slide "Materiais necessários".

--------------------------------------------------------------------
SLIDE 1.1 - Título: "O microfone não entende palavras"

VISUAL PRINCIPAL - duas caixas com seta, espelhando o slide "O sensor
não entende vibração" da Aula 14:

  [ SENSOR ]                 ->    [ SISTEMA DE ENGENHARIA ]
  mede pressão do ar               interpreta o que foi dito
  16 000 números por segundo       janela + feature + modelo

Embaixo, faixa CINZA com o que a MESMA leitura pode ser:
  fala | palma | cadeira arrastando | ventilador | silêncio

DESTAQUE: "Quem transforma medição em informação é o sistema de
engenharia."

NOTAS: é o mesmo slide da Aula 14 trocando vibração por som - dizer isso
em voz alta, a repetição amarra a trilha. Uma frase sobre a escala: o
acelerômetro era 100 medições por segundo, o microfone é 16 000. Não
explicar de onde vem o 16 000 (está no apêndice se alguém perguntar).

--------------------------------------------------------------------
SLIDE 1.2 - Título: "O sensor já entrega o número pronto"

VISUAL PRINCIPAL - duas colunas lado a lado.

ESQUERDA (CINZA) - "Sensor de som analógico - 1º ano"
    int som = analogRead(34);
  - a tensão está sempre no pino
  - o ESP32 mede e converte

DIREITA (AZUL) - "INMP441 - agora"
    INMP441::ler(som, 256);
  - o microfone digitaliza lá dentro
  - o ESP32 só recebe números

ACRESCENTE embaixo uma linha de código com o gráfico ao lado:
    Serial.printf(">som:%d\n", som[i]);

DESTAQUE: "Não existe analogRead para o INMP441."

NOTAS: aqui é HANDS ON de verdade - rodar o app-1 e falar no microfone.
O aluno vê a onda reagir à voz dele; é o momento lúdico da aula e vale
mais que qualquer slide. Se perguntarem sobre o INMP441.hpp: é uma
biblioteca, como a do DHT22 - ninguém escreve o driver do sensor.
Prosseguir.
```

---

## Seção 2 — Da janela à parede

```
SEÇÃO 2. Insira 4 slides na sequência.

--------------------------------------------------------------------
SLIDE 2.1 - Título: "A onda sonora oscila em torno de zero"

VISUAL PRINCIPAL - duas colunas.

ESQUERDA: onda de voz real (AZUL), simétrica em torno do eixo, com a
linha da MÉDIA desenhada em cima do eixo zero.  Rótulo: "média ≈ 0"

DIREITA: o exemplo da Aula 13, em caixa:
  Sinal: [-3, +3, -3, +3, -3]
  Média = 0     (os valores se cancelam)
  RMS   = 3,0   (captura a intensidade real)

DESTAQUE: "Em áudio a média é sempre ~0 - no silêncio E no grito. Ela só
serve para ser SUBTRAÍDA."

NOTAS: é o slide 14 da Aula 13 com um sinal de verdade no lugar do
exemplo sintético. Lá era curiosidade; em áudio é a regra.

--------------------------------------------------------------------
SLIDE 2.2 - Título: "A janela vira um número"

VISUAL PRINCIPAL - funil da esquerda para a direita, numerado e animado:

  (1) 16 000 números/s -> (2) janela de 512 (32 ms) -> (3) 1 RMS
      AZUL                    CINZA                      LARANJA

Embaixo: "31 números por segundo, em vez de 16 000."

ACRESCENTE mini-tabela do que se vê no gráfico:
  | silêncio | baixo e plano |
  | palma | pico estreito e altíssimo |
  | voz normal | ondula com as sílabas |
  | voz alta | sobe TANTO QUANTO a palma |   <- pinte de VERMELHO

DESTAQUE: "Um número diz QUÃO ALTO. Nunca diz O QUÊ."

NOTAS: rodar o app-2 ao vivo - bater palma, depois falar alto. A última
linha da tabela é o ponto: são indistinguíveis por energia. Emendar: e
se em vez de um número forem quatro?

--------------------------------------------------------------------
SLIDE 2.3 - Título: "Quatro números descrevem a janela"

VISUAL PRINCIPAL - tabela das features:

| Feature | Função | O que captura |
| média | calcMean | o deslocamento do microfone |
| desvio-padrão | calcStd | o volume |
| pico a pico | calcPtP | o maior salto |
| cruzamentos por zero | calcZCR | grave ou agudo |

Pinte a coluna "Função" de LARANJA.

ACRESCENTE faixa embaixo:
"calcMean, calcStd e calcPtP são as MESMAS funções do app17
(acelerômetro). Mesmo nome, mesmo código, outro sinal."

DESTAQUE: "4096 amostras -> 4 números."

NOTAS: reforçar a continuidade - eles já viram essas funções no
acelerômetro. A calcZCR é a única nova: conta quantas vezes o sinal passa
pelo zero, o que separa agudo de grave. Se sobrar tempo, teste ao vivo:
"sssss" e "ahhhh" no mesmo volume, e olhe o zcr. Se não sobrar, siga.

--------------------------------------------------------------------
SLIDE 2.4 - Título: "FIOT e EMERGÊNCIA empatam"   [SLIDE-CHAVE DA AULA]

VISUAL PRINCIPAL - tabela com as duas palavras e valores quase iguais:

| Feature | "FIOT" | "EMERGÊNCIA" |
| média | ~0 | ~0 |
| desvio | 412 | 431 |
| pico a pico | 2764 | 2810 |
| zcr | 1875 | 1902 |

Pinte a tabela de LARANJA e ponha um X VERMELHO grande sobre ela.

ACRESCENTE embaixo, em destaque grande:
"As features dizem QUANTA energia e QUÃO agudo.
 Não dizem QUAIS frequências, em QUAL ordem."

E uma linha menor:
"Tocar o áudio de FIOT de trás para frente dá exatamente os mesmos
quatro números."

NOTAS: ESTE É O SLIDE DA AULA. Tudo antes é rampa para ele; tudo depois
é a solução. Palavra é SEQUÊNCIA de sons, e nenhuma estatística de
janela captura ordem - a janela inteira vira um número só. O exemplo do
áudio invertido é o argumento mais curto e mais forte: use ele e pare.
Emendar: para capturar ordem, é preciso olhar a frequência AO LONGO DO
TEMPO - e é exatamente isso que o Edge Impulse faz.
```

---

## Seção 3 — Edge Impulse: coletar e treinar

```
SEÇÃO 3. Insira 4 slides na sequência.

--------------------------------------------------------------------
SLIDE 3.1 - Título: "O que muda é a frequência ao longo do tempo"

VISUAL PRINCIPAL - dois ESPECTROGRAMAS lado a lado, capturados do
próprio Studio: "FIOT" e "EMERGÊNCIA". Eixo horizontal = tempo,
vertical = frequência, cor = energia.

Embaixo de cada um, repita a linha de features do slide anterior em
CINZA claro, com a legenda "iguais".

DESTAQUE: "As mesmas quatro features. Imagens completamente diferentes."

NOTAS: a virada da aula. A informação que distingue as palavras sempre
esteve no sinal - a estatística de janela é que jogava fora.

--------------------------------------------------------------------
SLIDE 3.2 - Título: "MFCC: a rede não escuta, ela olha"

VISUAL PRINCIPAL - sequência numerada com setas, animada por etapas:

  (1) a janela de 1,5 s  ->  (2) fatiada em pedaços de ~25 ms  ->
  (3) energia por faixa de frequência em cada fatia  ->
  (4) cada fatia vira uma COLUNA; o conjunto vira uma IMAGEM

AZUL no passo 1, LARANJA nos passos 3 e 4.

ACRESCENTE em caixa: "No Edge Impulse isso é o bloco Audio (MFCC).
Parâmetros padrão servem."

DESTAQUE: "O tempo continua no eixo horizontal - é por isso que a ORDEM
dos sons sobrevive."

NOTAS: NÃO deduzir a transformada - eles não precisam disso para usar, e
turma de IA já tem repertório de rede que olha imagem. O único ponto que
precisa entrar é o destaque: o eixo do tempo sobrevive, e era isso que
faltava no slide anterior. Mencionar de passagem que a janela desliza de
500 em 500 ms para a palavra não cair na fronteira - é o efeito de borda
da Aula 14, agora virando parâmetro.

--------------------------------------------------------------------
SLIDE 3.3 - Título: "O celular é o sensor"

VISUAL PRINCIPAL - fluxo de 4 caixas com setas:
  [QR code no Studio] -> [gravar 60 s] -> [Split sample] -> [25 amostras]

Ao lado, antes/depois: uma faixa longa de áudio com ~25 picos, e embaixo
a mesma faixa recortada em janelas centradas nos picos.

ACRESCENTE caixa VERMELHA de atenção:
"Segmento = 1500 ms. O padrão do Studio é 1000 ms e EMERGÊNCIA tem 5
sílabas - não cabe. O segmento e a janela do impulse têm que ser o MESMO
número."

DESTAQUE: "Protocolo da Aula 14: descarte os primeiros segundos e
mantenha a condição estável durante toda a gravação."

NOTAS: não precisa de firmware para coletar - o celular vira o sensor,
sem instalar nada. Gravar com pelo menos 3 pessoas diferentes: um modelo
treinado só com a voz do professor reconhece só a voz do professor.

--------------------------------------------------------------------
SLIDE 3.4 - Título: "Quatro classes, não duas"

VISUAL PRINCIPAL - quatro caixas, as duas primeiras AZUIS e as duas
últimas CINZAS:

  fiot          a palavra que acorda
  emergencia    o comando
  ruido         silêncio, ventilador, cadeira, conversa ao fundo
  desconhecido  outras palavras: "fiapo", "emergente", "alô", números

DESTAQUE em caixa VERMELHA:
"Um modelo treinado só com as palavras-alvo SEMPRE responde uma delas.
Ele não tem alternativa para escolher."

NOTAS: as duas classes de baixo não são enchimento - são o que permite ao
modelo dizer NÃO. Retomar a Aula 14 (slide 28): o rótulo não vem do
sensor, vem de quem sabia o que estava acontecendo. Aqui, de quem gravou.
```

---

## Seção 4 — FIOT: o dispositivo acorda

```
SEÇÃO 4. Insira 3 slides. O último fica IMEDIATAMENTE ANTES da divisória
"HANDS ON!".

--------------------------------------------------------------------
SLIDE 4.1 - Título: "A máquina de estados"   [SLIDE-CHAVE]

VISUAL PRINCIPAL - diagrama de estados com formas e setas:

        +---------------------------+
        |         DORMINDO          |   CINZA
        |  só reage à palavra FIOT  |
        +-------------+-------------+
                      | "FIOT"
                      v
        +---------------------------+
   <----|    ATIVO - LED AZUL       |   AZUL
 timeout|                           |
   5 s  +-------------+-------------+
                      |
              "EMERGÊNCIA" -> buzzer

DESTAQUE em caixa:
"O modelo é o MESMO nos dois estados. O que muda é o que o firmware
aceita ouvir."

NOTAS: o slide de engenharia da aula. Um assistente sempre armado dispara
sozinho no meio da conversa da sala - e o conserto não foi treinar
melhor, foi mudar a ARQUITETURA. A palavra de comando só vale com o
dispositivo acordado, e isso derruba o falso positivo sem tocar numa
linha do modelo. Frase de fechamento: arquitetura vale tanto quanto
acurácia.

--------------------------------------------------------------------
SLIDE 4.2 - Título: "Limiar de confiança"

VISUAL PRINCIPAL - barra horizontal de 0 a 1 com a marca do limiar em
0,7 e três saídas do modelo plotadas nela:

  fiot 0,94    VERDE, aceita
  fiot 0,62    CINZA, ignora
  ruido 0,88   não é comando

Ao lado, tabela de 2 linhas:
  | limiar alto | menos falso positivo, mas precisa falar mais claro |
  | limiar baixo | reage fácil, mas dispara sozinho |

DESTAQUE: "Não existe limiar certo. Existe a escolha entre incomodar e
não responder."

NOTAS: eles já viram precision/recall em ML - não reensine, só aponte que
esse parâmetro é a escolha entre os dois, feita numa linha de código.
Pergunta para a turma: num assistente de EMERGÊNCIA, é pior disparar
sozinho ou deixar de responder? Não há resposta certa, e é isso que faz
a pergunta valer.

--------------------------------------------------------------------
SLIDE 4.3 - Título: "Treinou no celular, roda no INMP441"

VISUAL PRINCIPAL - duas caixas com uma seta torta e um "?" entre elas:
  [ microfone do celular ]  -- treino -->  [ modelo ]
  [ INMP441 no ESP32 ]      -- uso -->     [ modelo ]

Liste o que difere: resposta em frequência, ganho, distância, ruído.

ACRESCENTE caixa VERDE com as mitigações numeradas:
  1. gravar à mesma distância de uso (~30 cm)
  2. classe "ruido" gravada NA SALA onde a demo vai acontecer
  3. ligar o data augmentation no Studio
  4. se ainda falhar: gravar algumas amostras com o próprio INMP441

DESTAQUE: "O dado de treino precisa parecer o dado de operação."

NOTAS: fechar com honestidade de engenharia. Isso se chama descasamento
de domínio e é aposta consciente, não detalhe escondido - é a lição da
Aula 14 dita de outro jeito. Se a demo falhar ao vivo, ESTE slide é a
explicação: falhar na frente da turma com a explicação pronta vale mais
que acertar por sorte.
```

---

## Apêndice — Se perguntarem

Respostas prontas para o professor. **Nenhuma delas é slide.** Se a pergunta vier, responda em
30 segundos e siga — são assuntos de disciplina de sinais, não desta aula.

### "Por que 16 000 amostras por segundo?"

O essencial da fala vai até ~8 kHz — vogais no grave, "s"/"f"/"ch" no agudo. Nyquist (Aula 12,
slide 25) pede o dobro: 16 kHz. O acelerômetro do app17 usava 100 Hz porque movimento humano
não passa de ~50 Hz. **Mesmo teorema, fenômeno diferente.**

### "Por que o código pede 32 bits se o microfone é de 24?"

O I2S não transmite números, transmite **fatias de tempo**. Cada amostra ganha uma fatia de 32
pulsos de clock; o microfone despeja seus 24 bits no começo e o resto fica zero. **32 é o
tamanho do balde, 24 é a água.**

Dá para provar na saída crua: todo valor é múltiplo exato de 256, que são os 8 bits vazios
embaixo. `48550400 ÷ 256 = 189650`.

### "O que é esse GANHO no arquivo do microfone?"

Não é volume. O sensor entrega 24 bits (±8,4 milhões) e o programa guarda em 16 bits (±32.767).
Não cabe — tem que descartar 8 bits, e o ganho escolhe **quais**: é uma janela de 16 bits
deslizando sobre um número de 24.

- shift grande → nunca satura, mas voz normal vira gráfico quase reto
- shift pequeno → sussurro aparece, som forte **achata** no teto

É o mesmo dilema do 2G/4G/8G/16G da Aula 12, com uma diferença: no acelerômetro você escolhe a
régua **antes** de medir e a informação nunca é capturada; no microfone você escolhe **depois**,
e a informação chegou e foi descartada.

Medido na bancada: pico de `73.897.472` → `>>11` dá 36.082 (estoura o int16 e achata),
`>>12` dá 18.041 (cabe). Por isso o `GANHO = 12`.

### "Não tem uma biblioteca do INMP441, como a do DHT22?"

Não, e por um bom motivo: **o INMP441 não tem registradores.** Não tem endereço, não aceita
comando, você não escreve nada nele. A única configuração do chip é o pino L/R — e se faz com
um fio.

Biblioteca existe para encapsular protocolo próprio (DHT22, DS18B20, MPU6050 têm). Uma
"biblioteca do INMP441" seria uma biblioteca do periférico I2S do ESP32 — e essa já vem no core.

> A existência de biblioteca diz algo sobre o sensor: protocolo próprio → tem lib; barramento
> padrão sem registradores → não tem, e não faz falta.

### "Por que o `INMP441.hpp` tem todo aquele código?"

Porque `analogRead()` **também** é um monte de código — só já está escondido no framework.
Ninguém escreve o driver do ADC; também não se escreve o do I2S. O `.hpp` é uma biblioteca como
qualquer outra.

Se alguém insistir, o `app-0-mic-SOM` (material de apoio, fora do roteiro) mostra o que
acontece **sem** ele: o sinal sai picotado e metade das amostras é canal vazio.

---

## Apêndice — dados reais da bancada

Use estes números nos slides em vez de inventar. Saíram do `app-0-mic-SOM` num ESP32-S3 Super
Mini com INMP441.

```
>som:0            <- canal vazio (a lib <I2S.h> nao tem modo mono)
>som:48550400     <- 48550400 / 256 = 189650
>som:0
>som:50812928
...
>som:-35549696
```

| | |
|---|---|
| pico observado | `73.897.472` (= 288.662 no valor de 24 bits) |
| mínimo observado | `−35.549.696` |
| ganho resultante | `GANHO = 12` |

Pinagem: DevKit v1 = SCK 26 · WS 25 · SD 33 · S3 Super Mini = SCK 4 · WS 5 · SD 6.

---

## Falta capturar

Só existe depois que você gravar e treinar:

- [ ] print dos dois **espectrogramas** (FIOT e EMERGÊNCIA) → slide 3.1
- [ ] print do **MFCC** gerado → slide 3.2
- [ ] print do **Split sample** antes/depois → slide 3.3
- [ ] os **valores reais das 4 features** para FIOT e EMERGÊNCIA → slide 2.4
      (os do spec são plausíveis, mas **inventados** — troque pelos medidos)

---

## Depois deste deck

**Aula 13 — 3 slides de áudio depois do slide 16.** A agenda da própria Aula 13 (slide 2) já
promete *"Observação com dados de acelerômetro **e áudio**"*, e os 28 slides não têm uma linha
de áudio. Reaproveitar os slides 2.1, 2.3 e 2.4 deste deck.

**Aula 14 — 1 slide.** O slide 26 (FFT como "desafio extra") ganha uma emenda: em áudio ela
deixa de ser opcional.
