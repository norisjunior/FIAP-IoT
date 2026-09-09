# Instruções para o PowerPoint — IoT Aula 17 v2 (IMU multiclasse)

Arquivo alvo: `IoT - Aula 17 v2 - Machine Leaning com dados IoT parte 4 - IMU Multiclasse.pptx`

## Como usar

1. Abra o PowerPoint com o assistente.
2. Cole o **Bloco fixo** uma vez, no início da conversa.
3. Depois cole **uma seção por vez, na ordem**. Confira antes de seguir.

> Cada seção usa como âncora o **título da divisória**, não o número do slide.
> A **Seção 0** existe para dar nomes únicos às divisórias — hoje três delas se
> chamam "ML: Classificação multiclasse", e sem isso as âncoras seguintes ficam
> ambíguas. **Aplique a Seção 0 primeiro.**

---

## Estado atual do arquivo

**14 slides**, o esqueleto mínimo. Estrutura hoje:

| # | O que é | Situação |
|---|---|---|
| 1–3 | capa, capa da disciplina, agenda (*Sinais*) | manter |
| 4 | divisória **Aplicação IoT apoiada por ML** | manter |
| 5 | divisória **ML: Classificação multiclasse** | manter |
| 6 | Estudo de caso — as 4 classes do equipamento | **já em IMU**, manter |
| 7 | Problema de negócio + features do MPU | **já em IMU**, só o título diz "AQI" |
| 8 | divisória **ML: Classificação multiclasse** (2ª) | Seção 0 renomeia |
| 9 | Arquitetura — resíduo do AQI ("INFO: Aceitável / CRÍTICO: Perigoso") | **Seção 2 substitui** |
| 10 | divisória **ML: Classificação multiclasse** (3ª) | Seção 0 renomeia |
| 11 | As peças — ainda AQI (12 sensores, Flask, Telegram) | **Seção 4 substitui** |
| 12 | divisória **HANDS ON!** | manter |
| 13 | divisória **Recap** | Seção 8 reescreve |
| 14 | encerramento / copyright | manter |

Os slides 6 e 7 **já falam de IMU** — o conteúdo está certo. Nenhuma seção os
reescreve; a Seção 0 só corrige o título do 7.

Resultado final previsto: **37 slides**.

---

## Bloco fixo

> Cole isto primeiro, sempre.

```
Você vai completar os slides do arquivo
"IoT - Aula 17 v2 - Machine Leaning com dados IoT parte 4 - IMU Multiclasse.pptx".

O arquivo hoje tem 14 slides: é um esqueleto. As divisórias e os dois
slides do estudo de caso já existem. Você vai preencher o miolo.

CONTEXTO
- Disciplina de IoT, graduação FIAP, 2º ano.
- Os alunos JÁ cursaram uma disciplina de Machine Learning.
- A aula tem duas metades:
  (1) recapitular COMO o modelo foi treinado - o pipeline que a turma já
      executou nas aulas anteriores;
  (2) mostrar o que muda para transformar aquilo em INFERÊNCIA em tempo
      real, com o dispositivo recebendo a resposta de volta.

O CASO - motor com acelerômetro
- Um mini motor 3 V com hélice, num gabarito 3D, com um MPU6500 acoplado.
- Quatro classes, TODAS com o motor girando:
  operando · inclinado_frente · inclinado_tras · anomalia
  (anomalia = massa/fita colada na hélice, desbalanceando).
- O ESP32 amostra o acelerômetro a 100 Hz e fecha uma janela de 1 s
  (100 amostras). De cada janela ele calcula 8 features:
  mean_ax, mean_ay, mean_az, std_ax, std_ay, std_az, std_mag, p2p_mag.
- mean_* carregam ORIENTAÇÃO (a gravidade projetada nos eixos): separam
  inclinado_frente de inclinado_tras.
- std_* e p2p_mag carregam VIBRAÇÃO: separam operando de anomalia.
- Nenhuma das duas famílias sozinha resolve as 4 classes.

OS DOIS PIPELINES - é a espinha dorsal da aula inteira
Treinamento (já feito, app17-7):
  ESP32 -> MQTT Broker -> Node-RED -> InfluxDB -> Colab -> MLPClassifier (.pkl)
Inferência (o que vamos construir, app25):
  ESP32 -> MQTT Broker -> n8n -> FastAPI (.pkl) -> n8n -> MQTT Broker -> ESP32

ONDE ESTA AULA FICA NA TRILHA
- Aula 14: dataset com SINAIS - acelerômetro, janela, features. É a aula
  em que o app17-7 nasceu.
- Aula 17 v2 (esta): treinar o multiclasse e colocá-lo para inferir.
- Aulas 18 e 19 (próximas): o mesmo n8n ganha PostgreSQL e um agente LLM.
  Nesta aula o n8n aparece SÓ como orquestrador - nada de LLM ainda.

O QUE JÁ FOI VISTO SOBRE n8n - não repita do zero
- Na aula do app28 (AQI) a turma já montou um fluxo n8n com MQTT Trigger,
  nó Code, nó IF e HTTP Request. O n8n em si já é conhecido.
- O que é NOVO aqui: o fluxo VOLTA para o dispositivo por MQTT, em vez de
  terminar num alerta de Telegram. O ESP32 é agora consumidor da predição.

NUNCA CITE
- o caso AQI / qualidade do ar, os 12 poluentes, o Flask
Tudo isso é da versão anterior deste arquivo e está sendo substituído.
Onde o AQI aparecer, é para trocar - nunca para manter ao lado.
EXCEÇÃO: o Telegram continua no fluxo, mas com outro papel - no AQI ele
era o único destino; aqui é o segundo, e só dispara em anomalia.

NÃO REPITA (já viram em ML, é perda de tempo)
- o que é classificação, o que é rede neural, o que é MLP
- métricas de avaliação, matriz de confusão como conceito
- normalização, encoding, pré-processamento genérico
- o que é janelamento e feature estatística (foi a aula 14)

O FOCO (é o que só a disciplina de IoT ensina)
- o rótulo NÃO vem dos dados: veio de um humano apertando um botão
- o mesmo firmware que gerou o dataset NÃO serve para inferir - e a
  diferença é pequena e específica
- treino e inferência têm que produzir os números do MESMO jeito: mesma
  escala do sensor, mesmo filtro, mesma janela, mesma conta
- o contrato entre as duas pontas são os NOMES das 8 features
- a orquestração é uma peça própria, separada do modelo

ESTILO
- Português do Brasil.
- No máximo 5 marcadores por slide. Frases curtas, sem parágrafo.
- O slide apoia a fala, não a substitui.
- Mantenha o template visual e a identidade do arquivo atual.
- Em cada slide, escreva as NOTAS DO APRESENTADOR com o que falar.

RECURSOS VISUAIS - vale para todas as seções
- TODO slide precisa de um elemento visual: diagrama, tabela, esquema,
  linha do tempo ou destaque gráfico. Slide só com marcadores é exceção.
- Construa os diagramas com FORMAS do PowerPoint (retângulos, setas,
  conectores). Não use clipart nem imagem de banco.
- Ao comparar duas coisas, coloque as duas lado a lado no mesmo slide.
- Havendo sequência, use setas e numere as etapas.
- Use animação por etapas nos diagramas com sequência.
- Prefira mostrar um exemplo concreto (um JSON, um bloco de código, uma
  tela) a descrever o conceito em texto.

CÓDIGO DE CORES - o mesmo na aula inteira
- AZUL: medição vinda de um sensor (accelX, accelY, accelZ)
- LARANJA: valor derivado, calculado pelo dispositivo (as 8 features)
- ROXO: rótulo dado por um humano (o botão de classe)
- CINZA: identificação e metadado (device, rodada, janela, ts_epoch_ms)
- VERDE: o que o modelo devolve
- VERMELHO: o que foi REMOVIDO na passagem para a inferência

COMO VAMOS TRABALHAR
- Vou enviar uma seção por vez.
- Cada seção diz onde inserir, usando o título da divisória como âncora.
- Crie exatamente os slides pedidos, na ordem pedida.
- Não invente slides além dos solicitados.
```

---

## Mapa das seções

| Seção | Título | Onde | Slides |
|---:|---|---|---:|
| **0** | **Preparar as âncoras** | 3 divisórias + 1 título | 0 (só renomeia) |
| 1 | Duas famílias de feature | depois do slide "Problema de negócio" | 2 |
| 2 | Como o dataset foi feito | divisória **Treinamento** (substitui o slide de arquitetura AQI) | 5 |
| 3 | O modelo que saiu do Colab | após a Seção 2 | 2 |
| 4 | Do treinamento à inferência | divisória **Inferência** (substitui o slide das peças AQI) | 3 |
| 5 | O firmware muda de papel | após a Seção 4 | 3 |
| 6 | O serviço de ML em FastAPI | após a Seção 5 | 3 |
| 7 | O fluxo n8n | após a Seção 6, antes de **HANDS ON!** | 6 |
| 8 | Roteiro e Recap | após **HANDS ON!** e na divisória **Recap** | 3 |
| **9** | **Correções** | deck já montado, 40 slides | −1 (apaga um) |

---

## Seção 0 — Preparar as âncoras

```
SEÇÃO 0. Quatro renomeações. Nenhum slide entra ou sai.

1. Existem TRÊS divisórias com o título "ML: Classificação multiclasse".
   Renomeie a SEGUNDA e a TERCEIRA (a primeira fica como está):

   - a que vem logo DEPOIS do slide "Problema de negócio" passa a se
     chamar:  "Treinamento: do dispositivo ao modelo"
   - a seguinte passa a se chamar:
     "Inferência: do modelo ao dispositivo"

   Mantenha o layout e o estilo das divisórias existentes - troque só o
   texto do título.

2. O slide de "Problema de negócio" tem o título "Estudo de caso -
   Aplicação IoT: AQI", mas o conteúdo já é do acelerômetro. Troque o
   título para: "Estudo de caso - Aplicação IoT: Sinais/IMU".

3. Confira que não sobrou nenhuma menção a AQI nos títulos. O conteúdo
   dos slides com resíduo de AQI será substituído nas Seções 2 e 4.
```

---

## Seção 1 — Duas famílias de feature

```
SEÇÃO 1. Insira 2 slides NOVOS logo depois do slide "Estudo de caso -
Aplicação IoT: Sinais/IMU" (o do problema de negócio), antes da divisória
"Treinamento: do dispositivo ao modelo". Não altere os slides existentes.

--------------------------------------------------------------------
SLIDE 1.1 - Título: "O que o ESP32 mede e o que ele calcula"

VISUAL PRINCIPAL - DIAGRAMA EM DOIS BLOCOS, esquerda para direita:

BLOCO ESQUERDO - AZUL, título "o que o sensor mede":
    accelX, accelY, accelZ
    100 vezes por segundo
    (desenhe uma senoide curta dentro do bloco)

SETA GRANDE no meio, rotulada "janela de 1 s = 100 amostras"

BLOCO DIREITO - LARANJA, título "o que o ESP32 calcula":
    mean_ax   mean_ay   mean_az
    std_ax    std_ay    std_az
    std_mag   p2p_mag

DESTAQUE no rodapé:
"300 números por segundo entram. 8 saem. É o dispositivo que decide o que
o modelo vai poder aprender."

MARCADORES (2):
- A conta é feita a bordo, não no servidor.
- O que trafega na rede é a feature, não o sinal.

NOTAS: perguntar antes de mostrar o bloco direito: "quantos bytes por
segundo dá para mandar 100 amostras de 3 eixos por MQTT, e por quanto
tempo isso se sustenta?" A redução de 300 para 8 é a decisão de
engenharia central desta trilha - e é ela que torna a inferência viável,
porque a API recebe uma linha só, não um sinal.

--------------------------------------------------------------------
SLIDE 1.2 - Título: "Duas famílias de feature, dois trabalhos"

VISUAL PRINCIPAL - TABELA 2x4, com as quatro classes nas colunas e as
duas famílias nas linhas. Marque cada célula com VERDE (separa) ou
VERMELHO (não separa):

|                        | operando | incl_frente | incl_tras | anomalia |
| mean_* (orientação)    | VERMELHO | VERDE       | VERDE     | VERMELHO |
| std_* + p2p (vibração) | VERDE    | VERMELHO    | VERMELHO  | VERDE    |

VISUAL 2 - abaixo da tabela, duas mini-ilustrações lado a lado:
- esquerda: um vetor de gravidade inclinado sobre eixos x/z, rotulado
  "sen 25° = 0,42 migra de mean_az para mean_ax"
- direita: duas senoides sobrepostas, uma fina e uma grossa, rotuladas
  "operando" e "anomalia"

DESTAQUE no rodapé:
"Nenhuma das duas famílias resolve as quatro classes sozinha. As oito
juntas, sim."

NOTAS: este slide é o coração técnico da primeira metade. Nas aulas
anteriores, no problema binário, mean_* foi DESCARTADO - era um atalho
que codificava postura em vez de vibração. Aqui o veredicto se inverte,
porque agora a postura É a informação. Mesma feature, contexto diferente,
decisão oposta: é isso que se aprende projetando dataset.
```

---

## Seção 2 — Como o dataset foi feito

```
SEÇÃO 2. APAGUE o slide que vem logo depois da divisória "Treinamento: do
dispositivo ao modelo" - é o slide de arquitetura com resíduo do AQI
("Mensagens e Alertas", "INFO: Aceitável", "CRÍTICO: Perigoso").
No lugar dele, crie 5 slides, nesta ordem.

--------------------------------------------------------------------
SLIDE 2.1 - Título: "O pipeline de treinamento"

VISUAL PRINCIPAL - FLUXO HORIZONTAL de 6 caixas com setas, ocupando a
largura do slide. Numere as etapas de 1 a 6 e use ANIMAÇÃO por etapas:

  1 ESP32          2 MQTT Broker     3 Node-RED
  app17-7          Mosquitto         monta o ponto
  8 features       tópico            e grava
  + label          .../multiclasse
        |                |                 |
        v                v                 v
  4 InfluxDB       5 Colab           6 MLPClassifier
  measurement      lê via Flux       StandardScaler + MLP
  vibracao_        e treina          modelo_motor_
  multiclasse                        multiclasse.pkl

Pinte a caixa 1 de LARANJA (features) com um detalhe ROXO (o label),
as caixas 2-4 de CINZA (transporte e armazenamento) e as caixas 5-6 de
VERDE (o modelo).

DESTAQUE no rodapé:
"Cada peça faz uma coisa só. Nenhuma delas sabe o que a outra faz por
dentro."

NOTAS: este é o slide-mapa da primeira metade. Vale dizer que os alunos já
executaram cada uma dessas caixas em aulas separadas - aqui elas aparecem
juntas pela primeira vez. Anunciar que o primeiro slide da segunda metade
vai mostrar este mesmo desenho ao lado do pipeline de inferência.

--------------------------------------------------------------------
SLIDE 2.2 - Título: "O rótulo vem do botão, não do dado"

VISUAL PRINCIPAL - DIAGRAMA do protoboard em formas, com dois botões
destacados e uma legenda em cada:

  [Botão 21] -> "COLETA: inicia e para. Para sozinho em 30 janelas."
  [Botão 18] -> "CLASSE: avança na sequência. Só com a coleta parada."
  [LED]      -> "pisca N vezes = qual classe está selecionada"

Ao lado, a SEQUÊNCIA CÍCLICA desenhada como um círculo com 4 posições e
setas entre elas:
  operando -> inclinado_frente -> inclinado_tras -> anomalia -> (volta)
Marque a volta completa com a etiqueta "rodada++".

MARCADORES (3):
- Um humano posiciona o gabarito e diz qual classe é aquela.
- 30 janelas por classe, depois para sozinho.
- Cada volta completa incrementa a rodada.

NOTAS: insistir: o dataset não foi "coletado", foi PROJETADO. Alguém
decidiu 30 janelas, alguém decidiu 4 classes, alguém teve que lembrar de
colar a fita na hélice antes de gravar a classe anomalia e de tirar antes
de voltar para operando. Se o rótulo estiver errado, nada mais no
pipeline percebe.

--------------------------------------------------------------------
SLIDE 2.3 - Título: "A mensagem que sai do dispositivo"

VISUAL PRINCIPAL - JSON em fonte monoespaçada e grande, com cada chave
pintada pelo seu tipo:

    {
      "device": "IoTDevMultiClasse001",   <- CINZA
      "label": "inclinado_frente",        <- ROXO
      "rodada": 1,                        <- CINZA
      "janela": 7,                        <- CINZA
      "ts_epoch_ms": 1749760205123,       <- CINZA
      "mean_ax": 0.328,                   <- LARANJA
      "mean_ay": -0.008,                  <- LARANJA
      "mean_az": 0.929,                   <- LARANJA
      "std_ax": 0.036,                    <- LARANJA
      "std_ay": 0.030,                    <- LARANJA
      "std_az": 0.042,                    <- LARANJA
      "std_mag": 0.046,                   <- LARANJA
      "p2p_mag": 0.215                    <- LARANJA
    }

VISUAL 2 - LEGENDA em três caixas ao lado:
| LARANJA - Feature   | o ESP32 calculou        | 8 campos |
| ROXO - Rótulo       | um humano apertou botão | 1 campo  |
| CINZA - Metadado    | identifica a coleta     | 4 campos |

DESTAQUE no rodapé:
"Guarde a cor ROXA. Na segunda metade da aula ela desaparece - e é essa a
diferença entre gerar dataset e inferir."

MARCADORES (2):
- Tópico: FIAPIoT/motor/multiclasse
- Uma mensagem por segundo, uma janela por mensagem.

NOTAS: percorrer o JSON campo a campo. Chamar atenção para "label": ele
não foi medido nem calculado - veio do botão 18, apertado por quem sabia
em que posição o gabarito estava. Este é o slide que a Seção 5 vai
retomar para APAGAR a linha roxa.

--------------------------------------------------------------------
SLIDE 2.4 - Título: "Do InfluxDB para o DataFrame"

VISUAL PRINCIPAL - bloco de código em fonte monoespaçada, com os campos
a preencher destacados em VERMELHO:

    INFLUX_URL    = "https://us-east-1-1.aws.cloud2.influxdata.com"
    INFLUX_TOKEN  = "SEU_TOKEN_INFLUX_CLOUD"
    INFLUX_ORG    = "SUA_ORG"
    INFLUX_BUCKET = "IoTSensores"
    MEASUREMENT   = "vibracao_multiclasse"

    flux = f'''
    from(bucket: "{INFLUX_BUCKET}")
      |> range(start: -30d)
      |> filter(fn: (r) => r._measurement == "{MEASUREMENT}")
      |> pivot(rowKey: ["_time"], columnKey: ["_field"],
               valueColumn: "_value")
    '''
    df = client.query_api().query_data_frame(flux)

CALLOUT em LARANJA apontando para a linha do pivot:
"sem o pivot, cada feature vira uma LINHA. Com ele, vira uma COLUNA."

VISUAL 2 - abaixo, uma barra dividida em 3 blocos rotulados
"rodada 1 | rodada 2 | rodada 3", com os dois primeiros VERDES
("treino") e o último VERMELHO ("teste").

MARCADORES (3):
- Uma linha do DataFrame = uma janela de 1 s.
- A última rodada inteira fica fora do treino.
- A partir daqui é a aula de ML.

NOTAS: o pivot é o detalhe que trava todo mundo na primeira vez - o
InfluxDB devolve formato longo (uma linha por field) e o scikit-learn quer
formato largo (uma coluna por feature). O split por rodada é o outro
ponto: se fosse aleatório, janelas vizinhas da MESMA rodada cairiam metade
no treino e metade no teste - e como janelas vizinhas são quase idênticas,
a acurácia subiria sem o modelo ter aprendido nada. Lembrar que token não
vai para o GitHub.

--------------------------------------------------------------------
SLIDE 2.5 - Título: "O treino cabe em cinco linhas"

VISUAL PRINCIPAL - bloco de código grande, em fonte monoespaçada:

    modelo = make_pipeline(
        StandardScaler(),
        MLPClassifier(hidden_layer_sizes=(16,),
                      max_iter=2000, random_state=42),
    )

    modelo.fit(treino[FEATURES], treino["label"])

CALLOUT 1 em VERDE apontando para make_pipeline:
"o scaler viaja DENTRO do modelo - quem carregar o .pkl não precisa
lembrar de normalizar"

CALLOUT 2 em ROXO apontando para treino["label"]:
"y em TEXTO. O predict vai devolver 'operando', não 0."

MARCADORES (3):
- Uma camada escondida de 16 neurônios. O problema é pequeno.
- 8 entradas, 4 saídas.
- classes_ = ['anomalia', 'inclinado_frente', 'inclinado_tras', 'operando']

NOTAS: os dois callouts são o que esta aula acrescenta ao que já sabem de
ML. O Pipeline não é elegância: é o que impede o erro clássico de treinar
normalizado e inferir cru. E treinar com y em texto elimina o dicionário
de tradução que teria que ser mantido em sincronia entre o Colab, a API e
o firmware. Chamar atenção para a ordem ALFABÉTICA de classes_ - ela não é
a ordem da sequência de coleta, e isso volta no slide sobre o firmware.
```

---

## Seção 3 — O modelo que saiu do Colab

```
SEÇÃO 3. Insira 2 slides NOVOS logo depois do slide "O treino cabe em
cinco linhas" (o último criado pela Seção 2).

--------------------------------------------------------------------
SLIDE 3.1 - Título: "O que tem dentro do .pkl"

VISUAL PRINCIPAL - uma CAIXA GRANDE rotulada "modelo_motor_multiclasse.pkl
(24 KB)", contendo três caixas empilhadas:

  [ StandardScaler ]  média e desvio de cada uma das 8 features
  [ MLPClassifier  ]  pesos: 8 -> 16 -> 4
  [ metadados      ]  feature_names_in_ (as 8, na ordem)
                      classes_ (as 4, em texto)

Ao lado da caixa, uma seta de entrada LARANJA ("8 números") e uma seta de
saída VERDE ("1 nome + 4 probabilidades").

DESTAQUE no rodapé:
"O arquivo carrega o modelo E o contrato de como usá-lo."

MARCADORES (3):
- joblib.dump(modelo, "modelo_motor_multiclasse.pkl")
- feature_names_in_ garante que a ordem das colunas não importa.
- classes_ em texto: sem tabela de tradução em lugar nenhum.

NOTAS: abrir o arquivo ao vivo no Colab, se der tempo, e imprimir
modelo.classes_ e modelo.feature_names_in_. É o momento de mostrar que um
.pkl não é uma caixa-preta opaca: dá para perguntar a ele o que ele espera
receber e o que ele sabe responder.

--------------------------------------------------------------------
SLIDE 3.2 - Título: "As versões precisam casar"

VISUAL PRINCIPAL - COMPARAÇÃO lado a lado, duas colunas, com os mesmos
quatro pacotes nas duas e um sinal de igual grande no meio:

| Colab (1ª célula do notebook) | API (api/requirements.txt) |
| numpy==2.1.3                  | numpy==2.1.3               |
| pandas==2.2.3                 | pandas==2.2.3              |
| scikit-learn==1.6.1           | scikit-learn==1.6.1        |
| joblib==1.5.3                 | joblib==1.5.3              |

Abaixo, uma faixa VERMELHA:
"Versões diferentes = joblib.load() quebra, ou pior: carrega e prediz
errado, sem avisar."

MARCADORES (2):
- numpy e scikit-learn são as que realmente quebram.
- O .pkl não é um formato portável entre versões.

NOTAS: este é o erro número um quando o modelo sai do Colab e chega no
servidor. O notebook imprime as versões do runtime na última célula
justamente para conferir contra o requirements.txt antes de copiar o
arquivo. É um slide curto, mas evita meia aula de suporte no HANDS ON.
```

---

## Seção 4 — Do treinamento à inferência

```
SEÇÃO 4. APAGUE o slide que vem logo depois da divisória "Inferência: do
modelo ao dispositivo" - é o slide das peças com resíduo do AQI (ESP32
com 12 sensores, ML Service em Flask, Telegram). No lugar dele, crie 3
slides, nesta ordem.

--------------------------------------------------------------------
SLIDE 4.1 - Título: "Dois pipelines, a mesma origem"

VISUAL PRINCIPAL - DOIS FLUXOS HORIZONTAIS empilhados, um sobre o outro,
alinhados pelas duas primeiras caixas:

  TREINAMENTO (app17-7) - feito:
  ESP32 -> MQTT -> Node-RED -> InfluxDB -> Colab -> .pkl

  INFERÊNCIA (app25) - agora:
  ESP32 -> MQTT -> n8n -> FastAPI (.pkl) -> n8n -> MQTT -> ESP32

Desenhe um RETÂNGULO TRACEJADO em volta das duas primeiras caixas de cada
fluxo (ESP32 e MQTT), com a etiqueta: "igual nos dois".

Pinte de VERDE a caixa ".pkl" do primeiro fluxo e ligue-a com uma SETA
CURVA descendente até a caixa "FastAPI (.pkl)" do segundo, rotulada:
"o mesmo arquivo".

Note a diferença de FORMA: o primeiro fluxo termina; o segundo VOLTA.
Desenhe a seta final do segundo fluxo em arco, retornando ao ESP32.

MARCADORES (3):
- O treinamento termina num arquivo. A inferência é um ciclo.
- O dispositivo deixa de ser só produtor: agora ele consome.
- Node-RED gravava. n8n responde.

NOTAS: é o slide-charneira da aula. Deixar o desenho no ar e perguntar à
turma: "o que precisa mudar no ESP32 para ele participar do segundo
fluxo?" Deixar a pergunta aberta - a Seção 5 responde.

--------------------------------------------------------------------
SLIDE 4.2 - Título: "Os cinco passos da inferência"

VISUAL PRINCIPAL - CIRCUITO FECHADO, cinco caixas numeradas dispostas em
círculo (ou em U), com setas e ANIMAÇÃO por etapas:

  01  ESP32 fecha a janela de 1 s e publica as 8 features
      tópico: FIAPIoT/motor/multiclasse
  02  n8n (MQTT Trigger) recebe, extrai as 8 features do payload
  03  n8n (HTTP Request) faz POST /predict na API FastAPI
  04  FastAPI carrega o .pkl, prediz e devolve o NOME da classe
      {"class": "inclinado_tras", "probabilities": {...}}
  05  n8n (MQTT) publica o nome de volta
      tópico: FIAPIoT/motor/multiclasse/cmd -> ESP32 pisca

Marque no centro do circuito: "1 volta por segundo".

MARCADORES (2):
- Dois tópicos MQTT: um de ida, um de volta.
- O n8n aparece duas vezes: na entrada e na saída.

NOTAS: contar a volta inteira apontando o dedo no desenho. O detalhe que
merece pausa é o passo 05: o tópico de volta termina em /cmd, e é aí que o
dispositivo está inscrito. Sem esse tópico de retorno, tudo o que
existiria seria um dashboard - o dispositivo continuaria cego.

--------------------------------------------------------------------
SLIDE 4.3 - Título: "Quem faz o quê"

VISUAL PRINCIPAL - TABELA de 4 linhas, com uma coluna de cor à esquerda:

| Peça          | Responsabilidade      | O que ela NÃO faz            |
| ESP32         | medir e calcular      | não decide a classe          |
| n8n           | orquestrar            | não faz ML                   |
| FastAPI       | carregar o .pkl e     | não conhece MQTT, não sabe   |
|               | predizer              | de onde vieram os números    |
| MQTT Broker   | transportar           | não interpreta o conteúdo    |

FAIXA embaixo, atravessando o slide:
"Trocar o modelo não mexe no fluxo. Trocar o fluxo não mexe no modelo."

MARCADORES (2):
- A API recebe uma linha de 8 números e devolve um nome. Só isso.
- Cada peça pode ser testada sozinha.

NOTAS: a coluna "o que ela NÃO faz" é mais importante que a do meio. Falar
do teste isolado: dá para chamar a API com curl sem nenhum ESP32 ligado, e
dá para injetar uma mensagem no n8n sem a API no ar. Quando algo quebra no
HANDS ON, é assim que se descobre onde.
```

---

## Seção 5 — O firmware muda de papel

```
SEÇÃO 5. Insira 3 slides NOVOS logo depois do slide "Quem faz o quê".

--------------------------------------------------------------------
SLIDE 5.1 - Título: "O mesmo JSON, sem o roxo"

VISUAL PRINCIPAL - o JSON do slide "A mensagem que sai do dispositivo"
REPETIDO, mantendo as cores, mas com as QUATRO linhas removidas RISCADAS EM
VERMELHO e marcadas com um X:

    {
      "device": "IoTDevInferenciaMultiClasse001",   <- CINZA
      RISCADO  "label": "inclinado_frente",         <- some
      RISCADO  "rodada": 1,                         <- some
      RISCADO  "janela": 7,                         <- some
      RISCADO  "ts_epoch_ms": 1749760205123,        <- some
      "mean_ax": 0.328,  ... as 8 features ...      <- LARANJA
    }

VISUAL 2 - ao lado, TABELA com o motivo de cada saída, uma linha por campo:

| label       | era o gabarito humano  | é o que estamos perguntando |
| rodada      | agrupava as sessões    | não há treino aqui          |
| janela      | contava até as 30      | a coleta não termina        |
| ts_epoch_ms | o tempo é o eixo do banco | a janela vale AGORA      |

Destaque a linha do label com a seta grande e o texto em VERMELHO:
"o label saiu da MENSAGEM porque saiu do PROBLEMA. Agora ele é a resposta,
não a pergunta."

MARCADORES (3):
- 13 campos viram 9: sobra device e as 8 features.
- As 8 features não mudaram nem de nome nem de conta.
- Sem timestamp no payload, o NTP some do firmware inteiro.

NOTAS: voltar ao slide do JSON original (ou reexibi-lo) antes de mostrar
este. A mudança visual é pequena de propósito: o aluno tende a achar que
"inferência" exige reescrever tudo. Não exige. E as 8 features NÃO PODEM
mudar - elas são o contrato com o .pkl.

O ts_epoch_ms merece uma pausa, porque é o menos óbvio dos quatro. No
app17-7 as janelas iam para um BANCO DE SÉRIES TEMPORAIS, onde o tempo
literalmente é o eixo: sem carimbo não dá para ordenar, nem para saber
quando cada rodada aconteceu. Aqui a janela é medida, classificada e
respondida em menos de um segundo, e depois não serve para mais nada -
guardar QUANDO ela foi medida não muda o que o LED faz. Perguntar à turma
antes de revelar: "por que o mesmo campo era essencial lá e é inútil
aqui?" A resposta é o destino do dado, não o dado.

--------------------------------------------------------------------
SLIDE 5.2 - Título: "Sete coisas que não podem mudar"

VISUAL PRINCIPAL - TABELA de 7 linhas com uma coluna de "balança" à
esquerda (use o mesmo ícone de formas em todas), sob o título
"paridade treino x inferência":

| 1 | Chip do IMU        | o mesmo da coleta            |
| 2 | Fundo de escala    | setAccelRange(8)             |
| 3 | Filtro             | setAccelLPF(41) + setGyroLPF(42) |
| 4 | Calibração         | na posição de uso, motor parado |
| 5 | Amostragem         | 100 Hz, janela de 100        |
| 6 | Funções de feature | copiadas literalmente        |
| 7 | Nomes e unidade    | os 8 nomes exatos, em g      |

FAIXA VERMELHA embaixo:
"Se a inferência produzir os números de outro jeito, o modelo recebe dados
fora da distribuição em que foi treinado - e erra sem avisar."

MARCADORES (2):
- O ±8 g não é preferência: em ±2 g a vibração satura.
- calcStd divide por n, não por (n-1).

NOTAS: este slide não existia na versão AQI da aula, e é o que mais
diferencia sinal de escalar. Com temperatura, um sensor trocado ainda dá
um número comparável. Com vibração, mudar o fundo de escala ou o filtro
anti-aliasing muda TODAS as features de uma vez, e o modelo não tem como
perceber. Mencionar o oitavo item, que não está no código: a montagem
física. Girar o sensor 90° no gabarito troca mean_ax por mean_ay - se as
inclinações saírem trocadas, suspeitar da montagem antes do modelo.

--------------------------------------------------------------------
SLIDE 5.3 - Título: "A resposta chega e vira luz"

VISUAL PRINCIPAL - bloco de código em fonte monoespaçada, com o if/else
inteiro à mostra (é curto de propósito):

    void receberComando(char* topico, byte* conteudo,
                        unsigned int tamanho) {
      String classe(conteudo, tamanho);
      classe.trim();

      apagarTodasAsSaidas();          // passo 1: apaga tudo

      if (classe == "operando") {              // passo 2: acende uma
        digitalWrite(LED_AZUL, HIGH);
      } else if (classe == "inclinado_frente") {
        digitalWrite(LED_AMARELO, HIGH);
      } else if (classe == "inclinado_tras") {
        digitalWrite(LED_VERMELHO, HIGH);
      } else if (classe == "anomalia") {
        digitalWrite(BUZZER, HIGH);
      } else {
        // nome desconhecido: tudo fica apagado
      }
    }

CALLOUT 1 em VERDE apontando para o apagarTodasAsSaidas():
"apaga tudo, depois acende uma. Nunca ficam duas ligadas."

CALLOUT 2 em LARANJA apontando para o primeiro if:
"compara o NOME direto. Não existe índice de classe em lugar nenhum do
firmware."

VISUAL 2 - a SAÍDA REAL do Monitor Serial, em fonte monoespaçada e fundo
escuro, ao lado do código:

    --- MQTT recebido ---
      topico:  FIAPIoT/motor/multiclasse/cmd
      tamanho: 8 bytes
      payload: "anomalia"
      MODELO:  anomalia         -> BUZZER ligado
    ---------------------

VISUAL 3 - TABELA pequena embaixo, com o componente desenhado em cada linha
na cor certa:
| GPIO  4 | LED azul     | operando         |
| GPIO 21 | LED amarelo  | inclinado_frente |
| GPIO 18 | LED vermelho | inclinado_tras   |
| GPIO 19 | buzzer       | anomalia         |
| —       | tudo apagado | a nuvem ainda não respondeu |

Ao lado da tabela, uma NOTA em destaque:
"21 e 18 eram os dois BOTÕES do app17-7. Os pinos com que um humano rotulava
agora mostram o rótulo que o modelo escolheu."

DESTAQUE no rodapé:
"Acendeu, FICA aceso até chegar a próxima mensagem - e ela chega a cada
segundo. A saída é a memória do dispositivo."

MARCADORES (3):
- É a única função que mexe nas saídas. O loop() não cuida de LED.
- Não há delay() nem millis() aqui: nada a temporizar.
- O payload é impresso CRU, antes de qualquer comparação.

NOTAS: percorrer o if/else linha a linha - é o slide mais fácil da aula, e
isso é proposital depois de dois slides densos. Três coisas para apontar:
(1) apagar tudo antes de acender evita o bug clássico de duas saídas ligadas
quando a classe muda; (2) no ramo else as saídas ficam apagadas de propósito
- apagado avisa que algo está errado, e é melhor do que manter a classe
anterior e parecer que o sistema continua funcionando; (3) o firmware compara
NOMES. Aqui se paga o que foi prometido no slide do treino: como o modelo foi
treinado com y em texto, o firmware nunca precisou de uma tabela traduzindo
número em classe - tabela que teria de ser mantida em sincronia no Colab, na
API e aqui, e que um dia alguém esqueceria de atualizar.
```

---

## Seção 6 — O serviço de ML em FastAPI

```
SEÇÃO 6. Insira 3 slides NOVOS logo depois do slide "A resposta chega e
vira luz".

--------------------------------------------------------------------
SLIDE 6.1 - Título: "O .pkl vira serviço"

VISUAL PRINCIPAL - bloco de código em fonte monoespaçada:

    modelo = joblib.load("modelo_motor_multiclasse.pkl")

    FEATURES = ["mean_ax", "mean_ay", "mean_az",
                "std_ax", "std_ay", "std_az",
                "std_mag", "p2p_mag"]

    @app.post("/predict")
    def predict(janela: JanelaMotor):
        x_df = pd.DataFrame([janela.model_dump()])[FEATURES]
        predicao = str(modelo.predict(x_df)[0])
        ...

CALLOUT 1 em VERDE apontando para joblib.load, FORA da função:
"carrega uma vez, no boot. Não a cada requisição."

CALLOUT 2 em LARANJA apontando para [FEATURES]:
"DataFrame com os NOMES: a ordem em que o JSON chega não importa."

MARCADORES (2):
- Uvicorn na porta 8000.
- Documentação automática em /docs.

NOTAS: os dois callouts são erros que a turma comete. Carregar o .pkl
dentro da função funciona e é lento; ninguém percebe até rodar em
produção. E montar o DataFrame por posição em vez de por nome funciona até
alguém reordenar o JSON - aí prediz errado silenciosamente.

--------------------------------------------------------------------
SLIDE 6.2 - Título: "O contrato: entra e sai"

VISUAL PRINCIPAL - DIAGRAMA de duas caixas com uma seta grossa entre elas,
tudo em fonte monoespaçada:

CAIXA ESQUERDA, LARANJA, "POST /predict":
    {"mean_ax": -0.413, "mean_ay": 0.811, "mean_az": 0.965,
     "std_ax": 0.017, "std_ay": 0.011, "std_az": 0.005,
     "std_mag": 0.01, "p2p_mag": 0.04}

CAIXA DIREITA, VERDE, "resposta":
    {"class": "inclinado_frente",
     "probabilities": {"anomalia": 0.00,
                       "inclinado_frente": 0.99,
                       "inclinado_tras": 0.01,
                       "operando": 0.00}}

Abaixo, um comando curl em uma linha só, com fundo escuro:
    curl -X POST http://localhost:8000/predict \
      -H "Content-Type: application/json" -d '{...}'

MARCADORES (3):
- A API devolve o NOME, não o índice.
- As probabilidades vêm junto: dá para exigir confiança mínima.
- Dá para testar sem nenhum ESP32 ligado.

NOTAS: o último marcador é o mais prático da aula. Antes de ligar qualquer
coisa no HANDS ON, testar a API com curl - se ela não responde, não
adianta olhar o n8n nem o firmware. Mencionar as probabilidades como
gancho: um exercício natural é só acender o LED quando a maior
probabilidade passar de 0,8.

--------------------------------------------------------------------
SLIDE 6.3 - Título: "O que a API aceita e o que ela recusa"

VISUAL - TABELA de 4 linhas, cada uma com um código HTTP colorido:

| o que chega                        | resposta |
| as 8 features                      | VERDE  200 + a classe |
| as 8 + campos extras (device, ts)  | VERDE  200, extras ignorados |
| faltando uma feature               | VERMELHO 422 Field required |
| uma feature como texto             | VERMELHO 500 |

DESTAQUE no rodapé:
"Campo a mais, a API ignora. Campo a menos, ela recusa."

MARCADORES (2):
- O Pydantic valida a entrada antes de o modelo ver qualquer coisa.
- Por isso o nó IF do n8n existe: barrar o NaN antes de gastar a chamada.

NOTAS: mostrar ao vivo no /docs, se der tempo. A assimetria é o ponto: a
API é tolerante com o que sobra e rígida com o que falta. Isso muda como o
fluxo do n8n é montado - e explica por que o nó Code filtra os campos
mesmo a API não exigindo: o filtro documenta o contrato e faz um erro de
nome aparecer no n8n, e não silenciosamente na predição.
```

---

## Seção 7 — O fluxo n8n

```
SEÇÃO 7. Insira 6 slides NOVOS logo depois do slide "O que a API aceita e
o que ela recusa", ainda ANTES da divisória "HANDS ON!".

RESSALVA ao bloco fixo: ele manda não citar o Telegram, porque no caso AQI
o Telegram era o fim da linha. Nesta seção ele VOLTA, com outro papel - é
o segundo destino do fluxo, e só dispara em anomalia. Pode citá-lo aqui.

--------------------------------------------------------------------
SLIDE 7.1 - Título: "n8n: sete nós, dois destinos"

VISUAL PRINCIPAL - CANVAS DO n8n desenhado com formas. Quatro nós em linha
e depois uma BIFURCAÇÃO em dois ramos:

  [MQTT Trigger] -> [Code] -> [IF] -> [HTTP Request] --+
  FIAPIoT/motor/    extrai    valida  POST :8000/      |
  multiclasse       as 8              predict          |
                    features                           |
                                                       |
    ramo de cima (sempre, 1x/s) ------------------------+
    [MQTT] FIAPIoT/motor/multiclasse/cmd  -> volta ao ESP32
                                                       |
    ramo de baixo (só na virada p/ anomalia) ----------+
    [Code] filtra  ->  [Telegram] alerta   -> vai a uma PESSOA

Pinte o ramo de cima de VERDE (o ciclo de controle) e o de baixo de
VERMELHO (o alerta). Desenhe a seta do ramo verde voltando por baixo até um
ESP32 na margem esquerda, rotulada "o ciclo fecha".

DESTAQUE no rodapé, atravessando o slide:
"O mesmo resultado vai para duas plateias com pressas diferentes: a máquina
quer a cada segundo, a pessoa quer quando muda."

MARCADORES (3):
- n8n em http://localhost:5678
- Os quatro primeiros nós são os mesmos do fluxo do AQI.
- Os ramos são PARALELOS: o alerta nunca atrasa o LED.

NOTAS: começar reconhecendo o que já é conhecido - MQTT Trigger, Code, IF e
HTTP Request são exatamente os quatro primeiros nós do fluxo do AQI. O que
mudou é o fim: no AQI o Telegram era o único destino, e o sistema só
INFORMAVA. Aqui o destino principal é o dispositivo, e o sistema ATUA - o
Telegram virou o ramo secundário. Insistir na palavra PARALELO: se o alerta
estivesse em série antes do MQTT, uma falha do Telegram seguraria o LED.

SLIDE 7.2 - Título: "Nós 1 e 2: receber e limpar"

VISUAL 1 - painel do MQTT Trigger, desenhado como formulário:
    Credential:  MQTT Local - Mosquitto
    Topics:      FIAPIoT/motor/multiclasse

VISUAL 2 - bloco de código do nó Code, em fonte monoespaçada
(Mode: Run Once for Each Item):

    // O MQTT Trigger entrega o payload cru em $json.message (string).
    let data = $json.message;
    if (typeof data === "string") data = JSON.parse(data);

    // Só as 8 features, com os MESMOS nomes do treino.
    return {
      json: {
        mean_ax: data.mean_ax,
        mean_ay: data.mean_ay,
        mean_az: data.mean_az,
        std_ax:  data.std_ax,
        std_ay:  data.std_ay,
        std_az:  data.std_az,
        std_mag: data.std_mag,
        p2p_mag: data.p2p_mag,
      }
    };

CALLOUT em VERMELHO apontando para $json.message:
"o payload chega como STRING dentro de .message - não como objeto.
Sem este nó, a API receberia {message, topic} e responderia 422."

NOTAS: o callout é a pegadinha número um do n8n com MQTT, e é idêntica à
que a turma viu no fluxo do AQI. Vale antecipar a pergunta: "se a API
ignora campos extras, por que filtrar?" Duas razões: sem o nó Code o corpo
nem teria as features (só message e topic), e o filtro explícito faz um
erro de nome estourar aqui, no n8n, em vez de virar uma predição errada
silenciosa lá na frente.

--------------------------------------------------------------------
SLIDE 7.3 - Título: "Nós 3 e 4: validar e chamar o modelo"

VISUAL 1 - a condição do nó IF, em fonte monoespaçada:

    {{ ["mean_ax","mean_ay","mean_az","std_ax","std_ay",
        "std_az","std_mag","p2p_mag"]
       .every(k => typeof $json[k] === "number"
                   && !isNaN($json[k])) }}

VISUAL 2 - painel do HTTP Request como formulário, com a URL em destaque:

    Method:        POST
    URL:           http://host.docker.internal:8000/predict
    Send Body:     ON
    Body Content:  JSON
    JSON:          {{ $json }}

CAIXA DE ATENÇÃO em VERMELHO ao lado:
"host.docker.internal só funciona com o n8n em contêiner.
n8n nativo -> http://localhost:8000/predict
API em outra máquina -> o IP dela"

MARCADORES (2):
- Uma janela truncada vira NaN e derruba a API com 500.
- {{ $json }} manda o item inteiro, que o nó Code já deixou limpo.

NOTAS: a caixa vermelha é responsável pela metade dos travamentos no HANDS
ON. Explicar em uma frase: dentro de um contêiner, "localhost" é o próprio
contêiner, não a máquina - host.docker.internal é o apelido que o Docker dá
para o host. O nó IF é o mesmo do fluxo do AQI, com outra lista de campos.

--------------------------------------------------------------------
SLIDE 7.4 - Título: "Nó 5: devolver para o dispositivo"

VISUAL - painel do nó MQTT (Send a message) desenhado como formulário:

    Credential:       MQTT Local - Mosquitto
    Topic:            FIAPIoT/motor/multiclasse/cmd
    Send Input Data:  OFF        <- destacar em VERMELHO
    Message:          {{ $json.class }}

CALLOUT GRANDE em VERMELHO apontando para Send Input Data:
"se ficar ligado, o n8n publica o JSON INTEIRO - e o ESP32 responde
'classe desconhecida'"

VISUAL 2 - abaixo, o antes e depois COMO O ESP32 VÊ, em fonte monoespaçada,
o de cima com fundo VERMELHO e o de baixo VERDE:

    Send Input Data ON:
      payload: "{"class":"inclinado_tras","probabilities":{...}}"
      MODELO:  classe DESCONHECIDA - o LED nao muda

    Send Input Data OFF:
      payload: "inclinado_tras"
      MODELO:  inclinado_tras -> LED vermelho

MARCADORES (2):
- O ESP32 espera uma string pura, não um JSON.
- É o nó que não existia no fluxo do AQI.

NOTAS: este é o slide que salva o HANDS ON. O firmware compara a string
recebida com os quatro nomes - se vier o JSON inteiro, nenhum if casa, cai no
else e TUDO FICA APAGADO. O Monitor Serial mostra o payload cru e diz quais
nomes eram esperados, então o erro se diagnostica sozinho. Vale ligar o Send
Input Data de propósito e mostrar as duas saídas lado a lado: é um erro que
ensina a debugar.

--------------------------------------------------------------------
SLIDE 7.5 - Título: "O segundo destino: avisar uma pessoa"

VISUAL PRINCIPAL - LINHA DO TEMPO horizontal de 60 segundos, com uma faixa
por segundo colorida pela classe prevista:

  s0 ................. s20 ................... s45 ....... s55 ..... s60
  [ VERDE: operando  ][ VERMELHO: anomalia (25 s) ][ VERDE ][ VERMELHO ]

Abaixo da faixa, DUAS trilhas de setas:
  MQTT  -> uma seta para CADA segundo (60 setas, bem finas)  "a máquina"
  TELEGRAM -> apenas DUAS setas, em s20 e em s55              "a pessoa"

DESTAQUE em VERMELHO ao lado das duas setas do Telegram:
"2 mensagens, não 30. Só a VIRADA para anomalia dispara o alerta."

VISUAL 2 - o nó Code do ramo, em fonte monoespaçada:

    const estado   = $getWorkflowStaticData('global');
    const atual    = $input.first().json.class;
    const anterior = estado.ultimaClasse;
    estado.ultimaClasse = atual;

    if (atual === 'anomalia' && anterior !== 'anomalia') {
      return [{ json: { anterior: anterior || 'nenhuma' } }];
    }
    return [];   // nada a enviar: o ramo para aqui

MARCADORES (2):
- $getWorkflowStaticData guarda um valor entre execuções do fluxo.
- return [] encerra o ramo sem erro nenhum.

NOTAS: este é o slide que justifica não copiar o fluxo do AQI tal e qual. Lá
as medições chegavam esparsas e uma mensagem por medição fazia sentido. Aqui
chega uma janela por SEGUNDO: sem filtro, o Telegram receberia 60 mensagens
por minuto e o bot seria limitado pelo próprio Telegram. A pergunta para a
turma: "quem precisa saber a cada segundo, e quem precisa saber quando
muda?" Cadência de controle e cadência de alerta são coisas diferentes, e é
a orquestração - não o modelo, nem o firmware - que separa as duas.

--------------------------------------------------------------------
SLIDE 7.6 - Título: "Testar por partes"

VISUAL PRINCIPAL - ESCADA DE 4 DEGRAUS, de baixo para cima, cada degrau
com um comando em fonte monoespaçada e fundo escuro:

  1. a API sozinha
     curl -X POST http://localhost:8000/predict -d '{...}'

  2. o firmware publicando
     mosquitto_sub -h localhost -t "FIAPIoT/motor/multiclasse" -v

  3. o n8n respondendo
     mosquitto_sub -h localhost -t "FIAPIoT/motor/multiclasse/cmd" -v

  4. o firmware obedecendo
     mosquitto_pub -h localhost -t "FIAPIoT/motor/multiclasse/cmd" \
       -m "anomalia"

Ao lado da escada, uma seta vertical rotulada:
"pare no primeiro que falhar - o erro está ali"

MARCADORES (3):
- Cada degrau testa UMA peça.
- O degrau 4 testa o ESP32 sem n8n e sem API.
- O alerta do Telegram não entra na escada: só dispara na virada.

NOTAS: este é o slide que a turma vai olhar durante o HANDS ON inteiro.
Insistir na ordem: a tentação é ligar tudo e ver se funciona, e aí não há
como saber qual das quatro peças quebrou. O degrau 4 é o mais
contra-intuitivo e o mais útil - dá para validar o firmware inteiro com um
mosquitto_pub na mão.
```

---

## Seção 8 — Roteiro e Recap

```
SEÇÃO 8. Um slide depois da divisória "HANDS ON!", e dois depois da
divisória "Recap".

--------------------------------------------------------------------
SLIDE 8.1 - depois da divisória "HANDS ON!"
Título: "Roteiro da prática"

VISUAL PRINCIPAL - LISTA NUMERADA em 5 caixas na vertical, cada uma com
um quadradinho de checkbox à esquerda:

  1  Copiar o modelo_motor_multiclasse.pkl do Colab para api/
  2  Subir a API:  uvicorn service_app:app --host 0.0.0.0 --port 8000
     e testar em http://localhost:8000/docs
  3  Importar n8n/Fluxo-n8n-predict.json e selecionar as credenciais:
     MQTT nos DOIS nós de MQTT, e Telegram no nó de alerta
  4  Gravar o firmware (ajustar Wi-Fi e MQTT_SERVER antes)
  5  Ativar o fluxo no n8n e acompanhar o Monitor Serial

CAIXA lateral, VERDE, "sinal de que funcionou":
    MODELO:  operando -> LED azul
    MODELO:  anomalia -> BUZZER

MARCADORES (2):
- Repositório: app25-Inferencia-AI-API_Sinais-MultiClass_IMU
- Erro? Volte ao slide "Testar por partes".

NOTAS: reservar tempo para o passo 3 - são TRÊS credenciais a selecionar
(MQTT no trigger, MQTT na saída, Telegram no alerta) e é onde a turma
esquece uma. Sem a do Telegram o ciclo do LED funciona normalmente e só o
alerta falha - o que é o comportamento certo dos ramos paralelos, mas
confunde quem não percebeu. Lembrar que o fluxo importado vem
INATIVO: precisa ativar no canto superior direito, senão o MQTT Trigger
não escuta nada e não há erro nenhum na tela.

--------------------------------------------------------------------
SLIDE 8.2 - depois da divisória "Recap"
Título: "O que mudou de ponta a ponta"

VISUAL PRINCIPAL - TABELA de 6 linhas, três colunas:

|                  | Treinamento (app17-7)   | Inferência (app25)      |
| Quem rotula      | um humano, botão 18     | o modelo                |
| Orquestração     | Node-RED                | n8n                     |
| Destino do dado  | InfluxDB                | FastAPI                 |
| Tópicos MQTT     | 1 (só ida)              | 2 (ida e volta)         |
| O ESP32 é        | produtor                | produtor e consumidor   |
| O que sobra      | um .pkl                 | um ciclo rodando        |

Pinte a coluna do meio de CINZA e a da direita de VERDE.

MARCADORES (2):
- As 8 features não mudaram em nenhuma das duas pontas.
- O que mudou foi o que se faz com elas.

NOTAS: percorrer linha a linha, rápido. Parar na última: "o que sobra". No
treinamento o produto é um arquivo; na inferência o produto é um sistema
em operação, que não termina.

--------------------------------------------------------------------
SLIDE 8.3 - Título: "O que levar desta aula"

VISUAL PRINCIPAL - QUATRO CARDS numerados, cada um com um ícone feito de
formas:

  1  O rótulo não está no dado
     Alguém teve que saber a resposta antes. O dataset é projetado.

  2  Treino e inferência têm que medir igual
     Mesmo chip, mesma escala, mesmo filtro, mesma janela, mesma conta.
     E o contrato são os NOMES - das features e das classes.

  3  Cada peça faz uma coisa
     n8n não faz ML. A API não conhece MQTT. O ESP32 não decide.

  4  Virar inferência é, na maior parte, REMOVER
     Saíram dois botões, uma máquina de estados e o campo label.

DESTAQUE no rodapé:
"Nas aulas 18 e 19 este mesmo fluxo n8n ganha um banco e um agente LLM."

NOTAS: fechar amarrando com o que vem. O card 4 é o mais contra-intuitivo
e vale repetir: os alunos esperam que "colocar ML em produção" signifique
acrescentar complexidade ao dispositivo, e foi o contrário - o firmware
ficou menor, porque a decisão saiu dele.
```

---

## Seção 9 — Correções (deck de 40 slides)

Aplicar depois das Seções 0 a 8. Cinco correções, todas ancoradas pelo
**título** do slide — a numeração muda assim que o primeiro item apagar um
slide.

> **As notas do apresentador estão certas.** Uma versão anterior desta seção
> mandava movê-las, por um erro de leitura do arquivo: dentro do `.pptx`, os
> arquivos `notesSlideN.xml` são numerados na ordem em que as notas foram
> criadas, não pelo número do slide. Só duas notas precisam de retoque, e
> estão no item 9.2. **Não mova nota nenhuma.**

```
SEÇÃO 9. Cinco correções, na ordem abaixo. Nenhum slide novo é criado;
um é apagado. NÃO MOVA as notas do apresentador - elas estão corretas.

--------------------------------------------------------------------
9.1 - APAGAR O SLIDE DO CASO ANTIGO

Existe um slide sobrando do caso AQI: título "Estudo de caso -
Aplicação IoT: Sinais/IMU", com os textos "Mensagens e Alertas",
"INFO: Aceitável" e "CRÍTICO: Perigoso". Ele fica logo ANTES da
divisória "Inferência: do modelo ao dispositivo".

Cuidado: há outro slide com o mesmo título, o do "Problema de
negócio", que tem as features do MPU. Esse FICA.

APAGUE o slide que tem "INFO: Aceitável".

--------------------------------------------------------------------
9.2 - REFAZER O SLIDE "A resposta chega e vira luz"

O código nele é de uma versão antiga do firmware. Refaça o slide com
DOIS elementos apenas.

VISUAL 1 - o código, em fonte monoespaçada e GRANDE:

    apagarTodasAsSaidas();

    if      (classe == "operando")         digitalWrite(LED_AZUL,     HIGH);
    else if (classe == "inclinado_frente") digitalWrite(LED_AMARELO,  HIGH);
    else if (classe == "inclinado_tras")   digitalWrite(LED_VERMELHO, HIGH);
    else if (classe == "anomalia")         digitalWrite(BUZZER,       HIGH);

VISUAL 2 - TABELA ao lado, com o componente desenhado na cor certa:
| GPIO  4 | LED azul     | operando         |
| GPIO 21 | LED amarelo  | inclinado_frente |
| GPIO 18 | LED vermelho | inclinado_tras   |
| GPIO 19 | buzzer       | anomalia         |

DESTAQUE no rodapé, único texto solto do slide:
"Acendeu, FICA aceso até chegar a próxima mensagem. A saída é a
memória do dispositivo."

REMOVA: o bloco do Monitor Serial, os dois callouts, a nota sobre os
pinos 21 e 18, o rodapé sobre SEQUENCIA[] e os marcadores.

CORRIJA A NOTA deste slide. Ela descreve o código antigo: "varre
SEQUENCIA[] comparando por NOME até achar o índice". Substitua essa
frase por: "compara a string com os quatro nomes, num if/else. Não
existe índice de classe em lugar nenhum do firmware."

--------------------------------------------------------------------
9.3 - CORRIGIR A NOTA DO SLIDE "Nó 5: devolver para o dispositivo"

Só a nota; o slide fica como está. Ela diz "o firmware compara a
string recebida com SEQUENCIA[], item por item (...) e classePrevista
nunca muda". Substitua por: "se vier o JSON inteiro, nenhum if casa,
cai no else e TUDO FICA APAGADO."

--------------------------------------------------------------------
9.4 - SLIDE "Dois pipelines, a mesma origem": SETAS DE VOLTA

No fluxo de baixo (INFERÊNCIA), as caixas ESP32, MQTT e n8n aparecem
DUAS vezes cada. Parecem instâncias diferentes, e não são: é a mesma
peça na ida e na volta.

Refaça esse fluxo com QUATRO caixas apenas e duas fileiras de setas:

    ESP32  -->  MQTT  -->  n8n  -->  FastAPI (.pkl)
      ^          ^          ^             |
      |          |          |             |
      +----------+----------+-------------+

Uma fileira de setas por CIMA (ida), outra por BAIXO (volta), ligando
as mesmas quatro caixas. Rotule a de cima "as 8 features" e a de baixo
"a classe prevista".

O fluxo de cima (TREINAMENTO) não muda: ele termina mesmo, não volta.
É esse contraste que o slide precisa mostrar.

--------------------------------------------------------------------
9.5 - CORTES: menos texto nos slides mais carregados

Em cada um, remova o indicado. Não acrescente nada.

"O mesmo JSON, sem o roxo"
  Remova os 3 marcadores do rodapé. A tabela "por que cada campo saiu"
  já diz o mesmo.

"Do InfluxDB para o DataFrame"
  Remova as 5 linhas de configuração (INFLUX_URL, INFLUX_TOKEN,
  INFLUX_ORG, INFLUX_BUCKET, MEASUREMENT). Deixe só a query Flux.

"A mensagem que sai do dispositivo"
  No JSON, junte as 8 features em 2 linhas, como o slide "O mesmo
  JSON, sem o roxo" já faz. Cai de 13 linhas para 7.

"Nós 1 e 2: receber e limpar"
  No bloco de código, deixe só as 3 primeiras features e "...".

"O segundo destino: avisar uma pessoa"
  No bloco de código, deixe só:
      if (atual === 'anomalia' && anterior !== 'anomalia') {
        return [{ json: { anterior } }];
      }
      return [];
  As linhas de $getWorkflowStaticData saem do slide e vão para a nota.

"O contrato: entra e sai"
  Remova o comando curl do rodapé. Repete o JSON de entrada que já
  está no slide.
```

## Anexo — o que existe no repositório

O `app25` já está pronto e bate com estes slides.

```text
app25-Inferencia-AI-API_Sinais-MultiClass_IMU/
├── api/          service_app.py · modelo_motor_multiclasse.pkl · requirements.txt
├── device/       firmware + CONSTRUIR-O-FIRMWARE.md (guia em 7 etapas)
└── n8n/          Fluxo-n8n-predict.json  (5 nós, pronto para importar)
```

O `device/CONSTRUIR-O-FIRMWARE.md` monta o firmware do zero em sete etapas,
com a tabela de paridade do slide 5.2 marcada ao longo do código — serve de
apoio ao HANDS ON.

Parâmetros do fluxo n8n, caso precise montar à mão:

| # | Nó | Parâmetro | Valor |
|---|---|---|---|
| 1 | MQTT Trigger | Topics | `FIAPIoT/motor/multiclasse` |
| 2 | Code | Mode | Run Once for Each Item |
| 3 | IF | condição | ver slide 7.3 |
| 4 | HTTP Request | Method / URL | `POST` · `http://host.docker.internal:8000/predict` |
| | | Send Body / Content / JSON | ON · JSON · `={{ $json }}` |
| 5 | MQTT | Topic | `FIAPIoT/motor/multiclasse/cmd` |
| | | Send Input Data | **OFF** |
| | | Message | `={{ $json.class }}` |
| 6 | Code | filtro de virada | ver slide 7.5 — 2º ramo, sai do nó 4 |
| 7 | Telegram | Chat ID / Text | o chat do bot · mensagem de anomalia |

Os nós 5 e 6 saem **os dois** do nó 4, em paralelo: o ciclo do dispositivo
não espera o alerta.

## Fontes

- `app17-IMU/app17-7-MultiClassAccFeaturesInfluxDevkitv1MOTOR/` — firmware,
  README e o fluxo Node-RED do treinamento
- `app17-GerarDatasetSinais_IMU/app17-7-MultiClassAccFeaturesInflux/colab/treinamento_multiclasse.ipynb`
  — o notebook do Colab (MLPClassifier, InfluxDB via Flux)
- `app25-Inferencia-AI-API_Sinais-MultiClass_IMU/` — API, firmware, fluxo n8n
- `app28-Inferencia-AI-Cloud_Escalares_AQI/App-Fluxo_n8n/` — o padrão de fluxo
  n8n que a turma já conhece
- `IoT - Aula 18 e 19 - Automação LLM IoT ML.pptx` — a plataforma e o n8n
