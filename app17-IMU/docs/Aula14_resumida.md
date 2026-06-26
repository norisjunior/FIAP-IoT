# AULA 14 — Da série temporal ao Edge Analytics (versão resumida — 26 slides)

---

```markdown
# Slide 1 — Da série temporal ao Edge Analytics

## Ideia central
Apresentar o tema: transformar vibração em informação útil.

## Texto visível no slide
Da série temporal ao Edge Analytics

Como transformar vibração em informação útil

Pipeline da aula:

Fenômeno físico → Sensor → Sinal raw → Janela → Features → Classificação → Ação

## Elemento visual recomendado
Pipeline horizontal com as 7 etapas. Destacar "Decisão depende da qualidade da medição".

## Notas de fala
Conecte com as aulas 12 e 13: já vimos dados raw, sinais temporais e o problema da transmissão. Agora vamos da medição até a decisão.
```

```markdown
# Slide 2 — O sensor mede, o sistema interpreta

## Ideia central
O sensor não entende vibração; quem interpreta é o sistema de engenharia.

## Texto visível no slide
O MPU6050 mede aceleração (m/s²).

Ele não sabe se o movimento é:
- motor normal
- vibração excessiva
- batida na mesa
- ruído ou falha mecânica

Quem transforma medição em informação é o sistema de engenharia.

Nosso cenário: MPU6050 sobre a mesa + impactos = simulação de vibração/anomalia.
O ativo monitorado é a mesa.

## Elemento visual recomendado
Dois cards: Sensor "mede aceleração" | Sistema "interpreta o comportamento". Abaixo, esquema: MPU6050 na mesa → impacto → sinal de vibração.

## Notas de fala
Explique que o experimento é uma simplificação didática para entender coleta, janela, feature e anomalia.
```

```markdown
# Slide 3 — Um ponto não forma movimento

## Ideia central
Uma leitura isolada de aceleração é ambígua; movimento é sequência.

## Texto visível no slide
x = 0.15   y = -0.02   z = 9.76

Pode significar: parado, inclinado, início ou fim de movimento, ruído.

Um ponto não forma movimento. Uma sequência forma movimento.

Série temporal:

tempo, ax, ay, az
0 ms,   0.10, 0.02, 9.78
10 ms,  0.12, 0.01, 9.80
20 ms,  0.30, 0.05, 9.65
30 ms,  1.40, 0.80, 8.90

## Elemento visual recomendado
Ponto isolado no gráfico vs linha temporal ao lado. Legenda: "ponto isolado" vs "movimento ao longo do tempo".

## Notas de fala
Retome rapidamente a aula 12: vibração só faz sentido em função do tempo. Um tapa na mesa muda o sinal por alguns instantes.
```

```markdown
# Slide 4 — Visualizar antes de programar

## Ideia central
Raw é a fonte da verdade; o gráfico revela o que a linha não revela.

## Texto visível no slide
RAW = fonte da verdade

Permite: gráficos, testar janelas, novas features, auditoria, comparação de métodos.

O gráfico mostra: estabilidade, picos, início/fim de eventos, comportamento normal vs anômalo.

Antes de criar algoritmo, engenheiros observam o sinal.

## Elemento visual recomendado
Comparação: CSV bruto → difícil de interpretar | Gráfico temporal → comportamento visível.

## Notas de fala
Guardar raw dá flexibilidade: se amanhã quisermos outra janela ou feature, o dado original está lá. Mas raw tem custo (aula 13): 100 Hz = 100 mensagens/s.
```

```markdown
# Slide 5 — A pergunta de engenharia: raw ou features?

## Ideia central
Decisão central de projeto: enviar tudo ou resumir no dispositivo.

## Texto visível no slide
Opção A — Enviar raw
ESP32 → ax, ay, az continuamente → servidor analisa
Vantagem: flexibilidade | Desvantagem: muito dado

Opção B — Extrair features no ESP32
ESP32 → calcula resumo da janela → envia poucas features
Vantagem: eficiência | Desvantagem: menos liberdade depois

Não existe certo absoluto. Depende da fase do projeto.

## Elemento visual recomendado
Duas colunas comparativas: Raw no servidor | Features no ESP32.

## Notas de fala
Na fase de estudo, raw é valioso. Na solução final, features no edge são mais eficientes.
```

```markdown
# Slide 6 — Edge Analytics

## Ideia central
Edge Analytics é processar perto do sensor.

## Texto visível no slide
Processar perto do sensor.

MPU6050 → ESP32 coleta janela → ESP32 calcula features → envia resumo → servidor classifica/armazena

Edge não exige IA complexa no dispositivo.
Pode ser algo simples: média, RMS, energia antes de transmitir.

## Elemento visual recomendado
Pipeline horizontal destacando o ESP32 como "edge".

## Notas de fala
Isso é mais próximo de uma solução real de IoT e resolve o problema de sobrecarga visto na aula 13.
```

```markdown
# Slide 7 — Feature: de série temporal para tabela

## Ideia central
Feature é um resumo calculado da janela; transforma sinal em linha tabular.

## Texto visível no slide
Antes (raw): várias linhas
tempo, ax, ay, az
0,  0.10, 0.02, 9.78
10, 0.12, 0.01, 9.80
20, 0.30, 0.05, 9.65

Depois (features): uma linha
mean_ax,std_ax,rms_ax,min_ax,max_ax,p2p_ax,rms_mag,label
0.14,0.03,0.15,0.08,0.20,0.12,9.81,normal

100 leituras → 1 linha. Série temporal → tabela.
Modelos de ML tradicionais gostam de tabela.

## Elemento visual recomendado
Duas caixas: Raw (várias linhas) → seta "resumo da janela" → Features (uma linha).

## Notas de fala
A feature é a ponte entre sinal temporal e ML tradicional. O modelo não recebe 100 leituras, recebe resumos úteis.
```

```markdown
# Slide 8 — Janela temporal

## Ideia central
Janela é um pedaço do sinal; escolhê-la é decisão de engenharia.

## Texto visível no slide
taxa = 100 Hz
janela = 1 segundo → 100 amostras
janela = 2 segundos → 200 amostras

A janela define:
- quanto tempo do fenômeno será observado
- se eventos curtos serão capturados
- se o sistema responde rápido ou devagar
- se o processamento cabe no dispositivo

Escolher janela é escolher como o sistema enxerga o mundo.

## Elemento visual recomendado
Linha temporal com região destacada representando a janela. Card central: "Janela = decisão de engenharia".

## Notas de fala
Parâmetro de código representa decisão física e operacional. Mudar a janela muda as features e a decisão do sistema.
```

```markdown
# Slide 9 — Eventos curtos e o efeito de borda

## Ideia central
Eventos curtos podem ser diluídos em janelas grandes ou divididos entre duas janelas.

## Texto visível no slide
Problema 1 — Diluição:
janela = 2 s, tapa = 0,2 s → evento se dilui na média.

Problema 2 — Borda:
Janela 1: 0s ───── 2s
Janela 2: 2s ───── 4s
Tapa entre 1,8s e 2,2s → nenhuma janela vê o evento completo.

Para vibração contínua de motor, a preocupação é menor: o padrão se repete.
Para impactos isolados, é problema real.

## Elemento visual recomendado
Linha temporal com pico curto atravessando a fronteira entre duas janelas. Alerta: "efeito de borda".

## Notas de fala
O fenômeno define a estratégia: motor contínuo tolera janela fixa; impacto isolado pede outras abordagens (próximos slides).
```

```markdown
# Slide 10 — Estratégias de janela

## Ideia central
Quatro estratégias práticas de segmentação temporal.

## Texto visível no slide
| Estratégia | Como funciona | Quando usar |
|---|---|---|
| Janela fixa | blocos sequenciais sem sobreposição | começo, fenômeno contínuo |
| Sobreposição 50% | janela 2s, passo 1s | reduzir efeito de borda |
| Gatilho | coleta só quando magnitude passa de limite | evento raro e curto |
| Buffer circular | guarda 0,5s antes + 1,5s depois do evento | capturar contexto do impacto |

Para a disciplina: janela de 1 segundo é um bom ponto de partida.

## Elemento visual recomendado
Quatro mini-diagramas lado a lado: blocos sequenciais | janelas sobrepostas | limiar+evento | antes/evento/depois.

## Notas de fala
Janela fixa é a primeira implementação natural. Sobreposição aumenta custo computacional, mas reduz perda na borda. Gatilho e buffer economizam rede e energia.
```

```markdown
# Slide 11 — Features principais

## Ideia central
Conjunto básico de features para vibração; cada uma captura uma propriedade.

## Texto visível no slide
Para cada janela, podemos calcular:

| Grupo | Features |
|---|---|
| Tendência | mean |
| Variabilidade | std |
| Intensidade | rms, energia |
| Extremos | min, max, pico a pico |
| Eixos combinados | magnitude, rms_mag |
| Impacto | crest factor, jerk |

Nem sempre precisamos de todas.
Boa engenharia escolhe features coerentes com o fenômeno.

## Elemento visual recomendado
Cards agrupados por categoria: Tendência | Variabilidade | Intensidade | Impacto.

## Notas de fala
Features não são escolhidas aleatoriamente. Os próximos slides explicam as principais.
```

```markdown
# Slide 12 — Média e desvio padrão

## Ideia central
Média mostra o valor central; desvio padrão mede a instabilidade.

## Texto visível no slide
Média: útil para orientação do sensor, gravidade, inclinação.
Limitação: vibração positiva e negativa se cancela. Média sozinha não detecta vibração.

Desvio padrão: mede o quanto o sinal varia.

| Situação | std esperado |
|---|---|
| Sensor parado | Baixo |
| Mesa vibrando | Médio |
| Impacto forte | Alto |

## Elemento visual recomendado
Gráfico oscilando em torno de zero (média ≈ 0, mas houve movimento) + três mini-gráficos: parado, vibrando, impacto.

## Notas de fala
Std não mostra a posição média, mas o quanto o sinal oscila em torno dela. Para vibração, é muito útil.
```

```markdown
# Slide 13 — RMS

## Ideia central
RMS é a intensidade efetiva do sinal; resolve o cancelamento de sinais.

## Texto visível no slide
RMS = Root Mean Square (raiz da média dos quadrados)

sinal = [-3, +3, -3, +3]
média = 0
RMS = 3

Para vibração, RMS é uma das features mais importantes.

## Elemento visual recomendado
Dois blocos: Média → valores se cancelam | RMS → captura intensidade.

## Notas de fala
RMS representa a intensidade efetiva mesmo com oscilação positiva e negativa. É a métrica padrão de severidade de vibração na indústria.
```

```markdown
# Slide 14 — Pico, pico a pico e energia

## Ideia central
Features de extremos e intensidade acumulada detectam impactos e separam classes.

## Texto visível no slide
Pico: maior valor absoluto da janela.

Pico a pico: max - min

Energia: soma(x²)

Quanto maior a vibração, maior a energia.
Um único tapa altera muito o pico e o pico a pico.

Boas para separar: normal vs anomalia.

## Elemento visual recomendado
Gráfico de sinal com marcações de mínimo, máximo e pico a pico. Ao lado: janela normal (baixa energia) vs janela vibrando (alta energia).

## Notas de fala
Pico e p2p são sensíveis a eventos extremos. Energia acumula contribuições sempre positivas, pelos quadrados.
```

```markdown
# Slide 15 — Magnitude e RMS da magnitude

## Ideia central
Magnitude combina os três eixos; RMS da magnitude resume a intensidade geral.

## Texto visível no slide
mag = sqrt(ax² + ay² + az²)

Reduz dependência da orientação do sensor.
Sensor parado → magnitude próxima da gravidade (≈ 9,8 m/s²).

ax, ay, az → magnitude por amostra → RMS da magnitude

Resume a intensidade geral do movimento na janela.
Para a Sprint, é uma feature excelente.

## Elemento visual recomendado
Diagrama com três setas (ax, ay, az) convergindo para "magnitude" → "RMS da magnitude".

## Notas de fala
Combina duas ideias: juntar eixos e medir intensidade efetiva, sem depender de qual eixo está alinhado com a gravidade.
```

```markdown
# Slide 16 — Crest factor e jerk

## Ideia central
Features que detectam impactos e mudanças bruscas.

## Texto visível no slide
Crest factor = pico / RMS

Diferencia vibração contínua (crest menor) de impacto isolado (crest maior).

Jerk = aceleração_atual - aceleração_anterior

Detecta: trancos, batidas, início repentino de vibração.

aceleração mede movimento | jerk mede mudança brusca no movimento

## Elemento visual recomendado
Dois mini-gráficos: vibração contínua vs impacto isolado. Card: "jerk alto = mudança rápida".

## Notas de fala
Crest factor cresce quando há pico alto em janela de RMS moderado. Jerk é intuitivo: quanto a aceleração mudou entre amostras.
```

```markdown
# Slide 17 — Frequência: o próximo nível

## Ideia central
Na indústria, muitas falhas aparecem no domínio da frequência (FFT).

## Texto visível no slide
Falhas que aparecem como frequência:
- desbalanceamento
- desalinhamento
- falha em rolamento
- ressonância

Ferramenta: FFT (análise espectral).

Nesta disciplina: desafio extra, não obrigatório.

## Elemento visual recomendado
Comparação: domínio do tempo (sinal) vs domínio da frequência (picos).

## Notas de fala
Estamos focando features simples no tempo. Na indústria, frequência é essencial para diagnóstico de máquinas.
```

```markdown
# Slide 18 — Como a indústria trata vibração

## Ideia central
Vibração industrial tem norma, grandezas e critérios — não é "achismo".

## Texto visível no slide
ISO 20816-3:2022 — vibração em máquinas industriais (>15 kW, 120 a 30.000 rpm), com limites operacionais definidos.

| Grandeza | Unidade | Interpretação |
|---|---|---|
| Aceleração | m/s² | Leitura direta do acelerômetro (MPU6050) |
| Velocidade de vibração | mm/s RMS | Padrão de monitoramento industrial |
| Deslocamento | μm | Movimento físico acumulado |

Sensores industriais muitas vezes já entregam mm/s RMS: uma feature pronta.

Na indústria existe: medição, critério, histórico e limite operacional.

## Elemento visual recomendado
Card central "ISO 20816-3" + tabela de grandezas.

## Notas de fala
Os alunos não precisam decorar a norma, mas entender que aplicações reais se baseiam em critérios técnicos. Relembrar da aula 13: mm/s exige integração da aceleração — fora do escopo do curso.
```

```markdown
# Slide 19 — Baseline e anomalia

## Ideia central
Anomalia só existe em relação a uma referência de normalidade.

## Texto visível no slide
Sequência profissional:
Instrumentar → Coletar raw → Visualizar → Entender normalidade → Janela → Features → Baseline → Detectar desvios → Só depois, ML

Baseline = referência do comportamento normal.

Anomalia não é só "valor alto". Pode ser:
- RMS aumentando
- pico novo
- vibração irregular
- mudança de eixo dominante

ML sem entendimento do sinal vira chute automatizado.

## Elemento visual recomendado
Escada de maturidade: Instrumentar → Entender → Feature → Baseline → ML. Cards de tipos de anomalia.

## Notas de fala
Empresas não começam treinando modelo. Primeiro entendem o fenômeno, definem o normal e criam critérios. Anomalia é mudança de padrão em relação ao baseline.
```

```markdown
# Slide 20 — Anotação dos dados

## Ideia central
Anotar é associar a medição a uma condição conhecida; a label vem do experimento, não do sensor.

## Texto visível no slide
- sensor parado na mesa → label = normal
- sensor sofrendo impactos → label = vibracao

features = o que o sensor observou
label = o que o aluno sabe que estava acontecendo

features + label = dataset supervisionado

mean_ax,std_ax,rms_mag,p2p_mag,crest_mag,label
0.02,0.01,9.81,0.05,1.10,normal
0.15,1.42,11.30,5.80,3.90,vibracao

## Elemento visual recomendado
Fluxo: Fenômeno físico → Sensor → Medição → Label. Tabela com coluna label destacada.

## Notas de fala
O modelo não aprende "vibração" magicamente. Aprende porque alguém mostrou exemplos numéricos com a resposta correta. A label vem do protocolo experimental controlado pelo aluno.
```

```markdown
# Slide 21 — Duas formas de anotar

## Ideia central
A anotação pode acontecer no ESP32 (Estratégia A) ou no Python (Estratégia B).

## Texto visível no slide
Estratégia A — Anotação no dispositivo
botão → aguarda 5s → coleta janela → calcula features → envia com label
Mostra: Edge Analytics, eficiência, IoT realista.

Estratégia B — Anotação no Python
ESP32 envia apenas ax, ay, az → Python adiciona timestamp e label
Mostra: análise exploratória, visualização, teste de janelas.

As duas são corretas e complementares.

## Elemento visual recomendado
Duas colunas: ESP32 → features + label | ESP32 → raw → Python → timestamp + label. No centro: "Complementares".

## Notas de fala
Primeiro entendemos o fenômeno com raw (Estratégia B, usada na Sprint 3). Depois podemos levar o processamento para o edge (Estratégia A).
```

```markdown
# Slide 22 — Protocolo de coleta

## Ideia central
A qualidade da label depende da disciplina durante a coleta.

## Texto visível no slide
Para cada condição:
1. Pressionar botão / iniciar script
2. Aguardar 5 segundos (descartar transição)
3. Manter a condição estável durante toda a coleta
4. Coletar pelo tempo definido (ex.: 1 minuto por classe)
5. Conferir o gráfico

Por que esperar 5 segundos?
O aluno pode estar pegando o sensor, ajustando cabo, preparando o movimento.
Dados de transição contaminam o dataset.

Se a label diz vibracao, precisa existir vibração durante toda a janela.

## Elemento visual recomendado
Linha do tempo: botão → área cinza "descartar 5s" → área verde "coleta válida".

## Notas de fala
Se o aluno mexer no sensor durante a coleta normal, a label diz normal mas o sinal não representa normalidade. Nem todo dado coletado deve entrar no dataset.
```

```markdown
# Slide 23 — Timestamp

## Ideia central
O timestamp permite saber quando cada amostra/janela foi gerada e cruzar com eventos reais.

## Texto visível no slide
Timestamp absoluto: NTP / Internet → 2026-06-12T20:10:05
Bom para sistemas conectados.

Timestamp relativo: millis() do ESP32 → 0, 10, 20 ms
Bom para experimento local.

Na Sprint: o script Python registra o timestamp ao receber cada amostra via Serial.

Em sistemas reais, timestamp permite cruzar vibração com: falha, turno, manutenção, logs.

## Elemento visual recomendado
Dois cards: "NTP / Internet" | "millis() / local". Card lateral: "Tempo conecta sensores a eventos reais".

## Notas de fala
Para laboratório, o timestamp gerado pelo Python na recepção já atende. Para integração industrial, tempo absoluto sincronizado é o adequado.
```

```markdown
# Slide 24 — Estrutura dos dois datasets

## Ideia central
Raw e features têm granularidades diferentes: leitura vs janela.

## Texto visível no slide
Dataset raw — uma linha = uma leitura:

timestamp,ax,ay,az,label
2026-06-12T20:10:05.010,0.10,0.02,9.78,normal
2026-06-12T20:10:05.020,0.35,0.15,9.50,vibracao

Dataset de features — uma linha = uma janela:

window_id,mean_ax,std_ax,rms_ax,p2p_ax,...,rms_mag,label
1,0.02,0.01,0.03,0.08,...,9.81,normal

Label na janela:
- janela só normal → normal
- janela só vibração → vibracao
- janela misturada → descartar

## Elemento visual recomendado
Duas tabelas lado a lado. Três janelas: normal | vibracao | misturada (com alerta "descartar").

## Notas de fala
Não compare linha raw com linha de features: representam coisas diferentes. Não force label em janela ambígua — dado mal anotado é pior que pouco dado.
```

```markdown
# Slide 25 — Anotação ruim gera modelo ruim

## Ideia central
Qualidade da label e separação correta treino/teste determinam a confiabilidade do modelo.

## Texto visível no slide
Erros comuns:
- coletar normal mexendo no sensor
- não descartar transições
- label errada para "aumentar dados"
- misturar janelas sobrepostas no treino e teste

Janelas sobrepostas são quase iguais → train_test_split aleatório vaza informação.

Melhor: separar por coleta:
normal_01 e vibracao_01 → treino
normal_02 e vibracao_02 → teste

Garbage in, garbage out.

## Elemento visual recomendado
Comparação: Errado (linhas aleatórias) | Melhor (coleta 1 → treino, coleta 2 → teste).

## Notas de fala
O modelo pode parecer ótimo se vê no teste janelas quase iguais às do treino — avaliação enganosa. Coletar em rodadas separadas (normal_01, normal_02...) permite avaliação honesta.
```

```markdown
# Slide 26 — Aplicando à Sprint 3

## Ideia central
Fechar com as decisões técnicas que os alunos devem implementar.

## Texto visível no slide
| Decisão | Recomendação |
|---|---|
| Taxa | 100 Hz |
| Coleta | ≥ 1 minuto por classe (~6000 amostras/classe) |
| Formato raw | timestamp,ax,ay,az,label |
| Janela (Sprint 4) | 1 segundo (100 amostras) |
| Features básicas | mean, std, rms, min, max, p2p, rms_mag |
| Dataset final | mínimo 30 janelas por classe, balanceado |
| Labels | normal / anomala |

Frase-chave:
Em IoT, coletar dados é fácil.
Coletar dados bem anotados é engenharia.

## Elemento visual recomendado
Tabela de decisões + frase-chave em destaque no rodapé.

## Notas de fala
Conecte com as entregas: Sprint 3 = raw rotulado + visualização; Sprint 4 = features por janela + modelo + feature importance. Próxima aula: implementação no ESP32 e no Python.
