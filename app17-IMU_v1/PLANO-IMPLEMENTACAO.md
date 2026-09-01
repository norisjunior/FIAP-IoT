# Plano de implementação — app17-IMU (vibração/IMU + ML)

> **Este documento é a especificação completa da implementação.** Foi escrito para ser
> executado sem precisar redescobrir nada: todos os caminhos, números medidos e decisões
> de projeto já estão fixados aqui.
>
> - **Arquivo de decisões (leitura):** `C:\Projects\FIAP-IoT-develop\app17-IMU\` (este arquivo)
> - **Diretório de trabalho (escrita):** `C:\Projects\FIAP-IoT-eval\app17-IMU\`
>
> Não altere nada fora de `app17-IMU/` no worktree `eval` — há modificações pendentes do
> usuário em `app06`, `app07` e `app09` que não fazem parte deste trabalho.

---

## 0. Linha de base (fazer primeiro)

O worktree `eval` está no commit do `main` e tem o layout **antigo** do `app17-IMU`:
`analytics-coletor-raw-IMU/`, **sem** `analytics/notebooks/`. Os notebooks que os itens 2–4
modificam só existem no `develop`, que também tem versões mais novas do `app17-9` e `app17-10`.

Traga `app17-IMU/` do develop para o eval como ponto de partida (preservando os `.pio/`
existentes do eval, que são apenas cache de build):

- copiar `FIAP-IoT-develop/app17-IMU/analytics/` → `FIAP-IoT-eval/app17-IMU/analytics/`,
  **exceto** `AUTOAVALIACAO.md` e `README.md` (ver "Documentação" abaixo)
- copiar `FIAP-IoT-develop/app17-IMU/app17-9-EdgeJanelaFixaInflux/{src,README.md,diagram.json,platformio.ini}` sobre os do eval
- copiar `FIAP-IoT-develop/app17-IMU/app17-10-RawSerialCSV/{src,README.md,diagram.json,platformio.ini}` sobre os do eval
- copiar `FIAP-IoT-develop/app17-IMU/app17-0-Plot/*.py` sobre os do eval
- remover `FIAP-IoT-eval/app17-IMU/analytics-coletor-raw-IMU/` (foi renomeado para `analytics/coletor_raw/` no develop)

Depois disso o eval fica idêntico ao develop em `app17-IMU`, e os itens 1–5 são aplicados lá.

**Passo 0 e a separação de documentação já foram executados.** O eval está com a linha de base
pronta. Ler a seção "Documentação: o que vai em cada worktree" abaixo antes de criar qualquer
arquivo `.md`.

---

## Documentação: o que vai em cada worktree

> **Decisão do usuário:** arquitetura e decisões de projeto ficam **só no develop**.
> O eval leva apenas documentação **operacional**.

| tipo | exemplo | develop | eval |
|---|---|---|---|
| Arquitetura / decisões / processo | `PLANO-IMPLEMENTACAO.md`, `analytics/AUTOAVALIACAO.md`, `analytics/README.md` | **sim** | **não** |
| README operacional de app (wiring, pinagem, como rodar) | `app17-9-.../README.md`, `analytics/coletor_raw/README.md` | sim | **sim** |

Já removidos do eval: `analytics/AUTOAVALIACAO.md` e `analytics/README.md`.
Os 13 READMEs operacionais restantes ficam.

**Consequências para esta implementação:**

- **Não criar `analytics/README.md` no eval.** Se o mapa de arquitetura (notebooks novos, mapa
  arquivo → item da Sprint) precisar ser atualizado, atualize o do **develop**.
- O `app17-11-MotorMultiClasse/README.md` **deve ser criado no eval** — é operacional
  (montagem, pinagem, protocolo dos dois botões, como rodar). Mantenha-o operacional: sem
  justificativa de projeto, sem comparação de alternativas, sem "por que decidimos assim".

---

## Contexto do hardware (definido pelo usuário)

- **Sensor real: MPU6500** acoplado a um **mini motor com hélice**. Todo o item 5 é em
  **hardware físico** — não em Wokwi.
- O usuário consegue inclinar o conjunto para frente e para trás com facilidade.
- Na FlixPeriph, `MPU6500` é `typedef` de `MPU9250` (ver `MPU6500.h`). A API é a mesma.
- Aceleração retornada em **m/s²**. Em repouso, um eixo fica ≈ 9,81.

---

## Item 1 — Firmware: timing de amostragem + anti-aliasing

**Arquivos:**
- `app17-10-RawSerialCSV/src/app17-10-rawserialcsv.cpp`
- `app17-9-EdgeJanelaFixaInflux/src/app17-9-edgejanelafixa.cpp`

### 1a) Corrigir o drift da amostragem

Os dois sketches usam `tempoAnterior = millis()`, que acumula o atraso de cada iteração e faz a
taxa real escorregar abaixo de 100 Hz. O `app17-8-MotorML` **já tem o padrão correto**
(linhas 183–190) — copiar de lá:

```cpp
if (millis() - tempoAnterior >= AMOSTRA_MS) {
    // Avança em passos fixos de AMOSTRA_MS (e nao "= millis()"): assim o atraso
    // de um ciclo não empurra o próximo e a taxa não escorrega abaixo de 100 Hz.
    tempoAnterior += AMOSTRA_MS;
    // Se ainda estamos mais de uma amostra atrasados, não adianta amostrar em
    // rajada para recuperar: as amostras sairiam sem espacamento real. Recomeça do agora.
    if (millis() - tempoAnterior >= AMOSTRA_MS) tempoAnterior = millis();
    // ...
```

**PRESERVAR** o `tempoAnterior = millis();` da **linha 211 do app17-9** (dentro do handler do
botão, comentado como "reinicia o timing de amostragem"). Ali a ressincronização é deliberada,
porque a janela em andamento está sendo descartada. Está correto.

### 1b) Configurar o DLPF (anti-aliasing)

Nenhum dos sketches configura o filtro passa-baixa do sensor. Consequência verificada na lib:

- **MPU6050:** `begin()` não mexe no DLPF → fica no default de reset `DLPF_CFG=0` = **260 Hz**
- **MPU6500/9250:** `begin()` deixa em `DLPF_BANDWIDTH_184HZ` = **184 Hz**

Amostrando a 100 Hz, Nyquist = 50 Hz. Ou seja: **nos dois sensores todo o conteúdo acima de
50 Hz volta rebatido (aliasing)** para dentro da banda. Para `std`/`rms` isso é viés; para
qualquer feature de FFT é fatal.

Adicionar logo após o `setAccelRange`, nos dois sketches:

```cpp
// Anti-aliasing: amostramos a 100 Hz, entao Nyquist = 50 Hz. Sem este filtro o
// sensor entrega banda de 184 Hz (MPU6500) ou 260 Hz (MPU6050) e tudo acima de
// 50 Hz volta REBATIDO para dentro do sinal. DLPF_50HZ_APPROX = 41 Hz de banda.
mpu.setDLPF(IMUInterface::DLPF_50HZ_APPROX);
```

Confirmado na lib que `DLPF_50HZ_APPROX` existe e funciona nos dois sensores
(`MPU6050.cpp::setDLPF` → `BW_42`; `MPU9250.cpp::setDLPF` → `DLPF_BANDWIDTH_41HZ`).

### Verificação
`pio run` nos dois projetos. O PlatformIO está instalado e funcional nesta máquina.

---

## Item 2 — Notebook 2.4 e coletor: rodada, descarte correto, fs efetiva

**Arquivos:** `analytics/notebooks/2.4_janelas_features_split.ipynb`,
`analytics/coletor_raw/coletor_raw.py`

### 2a) Coletor: validar label e numerar rodada

Hoje o `input()` aceita qualquer string. Se o usuário digitar `anomalia` em vez de `anomalo`,
o notebook 2.5 faz `df[df["label"].isin(CLASSES)]` e **descarta a classe inteira em silêncio**.

- validar o label contra uma lista fechada, repetindo a pergunta se não bater
- **numerar a rodada automaticamente** (01, 02, 03…), incrementando a cada rodada da sessão —
  ver o item 5c, que define uma sessão do coletor como uma sequência de rodadas. Nome do arquivo:
  `coleta_<label>_<NN>_<AAAAMMDD_HHMMSS>.csv`
- lista de labels válidos (cobre os dois experimentos):
  `normal`, `anomalo`, `desligado`, `operando`, `inclinado_frente`, `inclinado_tras`, `anomalia`

### 2b) Notebook: coluna `rodada`

Ao concatenar os CSVs, criar `rodada` a partir do nome do arquivo (o stem, sem extensão).
É o identificador de grupo usado no descarte e no split.

### 2c) Corrigir o descarte de transição

**Bug atual:** o código faz `raw.groupby("label")` e depois `.iloc[DESCARTE_S*FS:]`, o que
descarta 3 s **do grupo concatenado inteiro**. Com 5 rodadas de `normal`, quatro delas mantêm
a transição contaminada.

Correção: `groupby(["label", "rodada"])`, descartando `DESCARTE_S * FS` amostras
**de cada rodada**.

### 2d) Célula nova: fs efetiva

O CSV já tem `timestamp` e ninguém o usa. Adicionar, por rodada:

```
fs_efetiva = 1 / mediana(diff(timestamp).dt.total_seconds())
```

Imprimir por rodada e avisar se desviar mais de 5% de 100 Hz. É o sanity check que revela
problemas de timing no firmware ou de latência serial antes de treinar qualquer coisa.

### 2e) Split por rodada

Substituir o split cronológico 70/30 dentro da classe por **holdout por rodada** (a última
rodada de cada classe vai para teste). Manter fallback para o cronológico quando houver apenas
uma rodada por classe, com uma célula markdown explicando a diferença e por que o split por
rodada é o correto (janelas vizinhas no tempo são quase idênticas e vazam informação).

---

## Item 3 — Features: MANTER `mean_*` e `rms_mag` como exercício didático

> **Decisão explícita do usuário:** não remover `mean_ax/ay/az` nem `rms_mag`. O exercício de
> *descobrir* que devem sair é mais valioso que já entregá-los removidos.

### 3a) O 2.4 passa a gerar todas as features (nada é removido)

Manter as 7 atuais — `mean_ax`, `mean_ay`, `mean_az`, `std_ax`, `std_ay`, `std_az`, `rms_mag` —
e **acrescentar**, calculadas sobre a magnitude `mag = sqrt(ax²+ay²+az²)`:

| feature | fórmula | por que |
|---|---|---|
| `std_mag` | `std(mag)` | RMS da componente AC — a métrica correta de vibração |
| `p2p_mag` | `max(mag) - min(mag)` | amplitude pico-a-pico |
| `crest_mag` | `max(abs(mag - mean(mag))) / std_mag` | fator de crista: falha impulsiva |
| `kurt_mag` | curtose de `mag` | impulsividade — indicador clássico de rolamento |
| `zcr_mag` | taxa de cruzamento por zero de `mag - mean(mag)` | proxy barato da frequência dominante |

Usar `ddof=0` no `std` (o default do `np.std`), para bater com o `calcStd` do firmware.

### 3b) O 2.5 ganha uma seção comparativa de conjuntos de features

Treinar o mesmo Random Forest com três conjuntos e comparar lado a lado:

| conjunto | composição | lição |
|---|---|---|
| `FEATURES_ORIG` | as 7 atuais | acurácia ≈ 1,00 |
| `FEATURES_AC` | sem `mean_*`, `std_mag` no lugar de `rms_mag`, mais as novas | acurácia se mantém e o modelo passa a ser físico |
| `FEATURES_TUDO` | todas | a importância revela o atalho |

Pontos que o texto do notebook deve explicitar:

1. **`mean_ax/ay/az` codificam orientação, não vibração.** Parado na mesa, `mean_az ≈ 9,81`.
   Ao chacoalhar na mão a orientação média muda, então o modelo pode acertar 100% classificando
   *postura*. Quebra assim que o sensor for montado em outro ângulo.
2. **`rms_mag` está saturado pela gravidade.** `rms_mag = sqrt(mean(ax²+ay²+az²)) ≈ sqrt(9,81² + σ²)`.
   Para σ = 1 m/s², `rms_mag` = 9,861 contra 9,81 em repouso: variação de **0,5%**. A mesma
   informação em `std_mag` vai de ~0 para 1,0. Mostrar essa conta no notebook.
3. **`rms_ax ≈ sqrt(mean_ax² + std_ax²)`** — quase função determinística das outras duas, por isso
   `rms_ax/ay/az` já tinham sido descartadas do conjunto de 7.

### 3c) O gancho entre os dois experimentos (importante)

O fechamento do exercício é o **item 5**: no problema binário `mean_*` é um atalho a remover;
no problema de 5 classes `mean_*` é **a única família de features que resolve as classes de
inclinação**. Mesmas features, veredictos opostos, dependendo da pergunta de engenharia.
Deixar isso explícito nos dois notebooks, com referência cruzada.

---

## Item 4 — Notebook 2.5: baseline, GroupKFold, curva de aprendizado

**Arquivo:** `analytics/notebooks/2.5_random_forest_raw.ipynb`

### 4a) Baseline obrigatório antes do Random Forest

- `DummyClassifier(strategy="most_frequent")` — o piso
- `DecisionTreeClassifier(max_depth=1)` usando **só `std_mag`**, imprimindo o limiar aprendido

Se o baseline empatar com o RF, a conclusão honesta é que o problema é trivial. Parado na mesa
vs. chacoalhado difere em σ por 2–3 ordens de grandeza; um único limiar resolve. Dizer isso no
notebook é mais formativo que comemorar 100% de acurácia.

### 4b) `LeaveOneGroupOut` com `groups=rodada`

Substituir o split único pela validação cruzada deixando uma rodada de fora por vez.
Detectar o caso de menos de 2 rodadas por classe e explicar a degradação em vez de quebrar.

### 4c) Curva de aprendizado por número de rodadas

Treinar com 1, 2, 3, … rodadas e avaliar na rodada retida, plotando o resultado. Esta é a
resposta metodologicamente correta para "de quantos dados eu preciso": quando a curva achata,
já há dados suficientes. Substitui regra de bolso por medição.

### 4d) Métricas

Acrescentar `f1_macro` à acurácia (indispensável quando o item 5 trouxer 5 classes).

---

## Item 5 — `app17-11-MotorMultiClasse` (hardware físico) + notebook 2.6

> **REVISÃO IMPORTANTE — o app17-11 é FORMA 1, não Forma 2.**
> O usuário quer que o aluno rode a aplicação inteira no ESP32: o próprio ESP32 janela,
> calcula as features e imprime no Serial Monitor a cada 2 s. Depois MQTT → Node-RED →
> InfluxDB → Colab. **Sem `coletor_raw.py`.**
>
> Isso é exatamente a arquitetura do `app17-9`, e as seções 5b/5c/5d abaixo foram escritas
> para a Forma 2. **Ler a seção 5-REV antes delas** — ela substitui o que for conflitante.
> O `app17-10` + `coletor_raw.py` **continuam existindo** para as lições de raw e
> re-janelamento (notebooks 2.3/2.4); o contraste entre as duas formas é o ponto didático.

---

## 5-REV — Revisão do app17-11 para Forma 1

### 5-REV.1) Por que Forma 1 aqui (e não só conveniência)

Features no edge **desacoplam a taxa de amostragem do transporte**. Streaming raw a 115200 baud
cabe ~520 linhas/s no limite teórico (22 B/linha ÷ 11.520 B/s) — na prática ~300 Hz. Com
features, trafegam ~15 floats a cada 2 s **independente da fs**. Isso é o que permite subir a
amostragem, e subir é necessário pelo motivo seguinte.

### 5-REV.2) Nyquist x RPM do motor — a razão técnica principal

Mini motor com hélice gira tipicamente entre 3.000 e 10.000 RPM:

| RPM | frequência de rotação |
|---|---|
| 3.000 | 50 Hz |
| 6.000 | 100 Hz |
| 10.000 | 167 Hz |

A 100 Hz de amostragem, Nyquist = 50 Hz. **A fundamental do motor está em cima ou acima disso
em qualquer velocidade plausível.** Duas consequências:

1. Não dá para observar a fundamental — onde o desbalanceamento se manifesta (1× RPM). A
   classificação ainda funciona (a energia acopla para a banda baixa pela estrutura), mas o
   notebook **não pode afirmar** que está vendo desbalanceamento em 1×.
2. **Sem DLPF, a rotação volta rebatida como pico fantasma.** Motor a 5.500 RPM (92 Hz) aparece
   a 100−92 = 8 Hz e parece fenômeno real. Isso eleva a importância do item 1: no app17-11 o
   DLPF é a diferença entre medir o fenômeno e medir um artefato.

**Decisão: fs = 500 Hz, `DLPF_MAX` (184 Hz no MPU6500).** Nyquist = 250 Hz, cobre rotação até
~11.000 RPM. Viabilidade verificada: o `read()` do MPU6500 transfere ~23 bytes; a 400 kHz de
I2C dá ~560 µs por leitura, confortável no orçamento de 2.000 µs de 500 Hz. A 1 kHz ficaria
apertado. (No MPU6050 usar `DLPF_100HZ_APPROX` = 98 Hz, porque o `DLPF_MAX` dele é 256 Hz e
ultrapassaria Nyquist.)

**Atenção:** `MPU9250::read()` retorna `false` quando não há dado novo. Os sketches atuais
ignoram o retorno (`mpu.read(); mpu.getAccel(...)`). A 100 Hz não fazia diferença; a 500 Hz,
**testar o retorno** e só contabilizar a amostra quando for `true`.

### 5-REV.3) Passo de caracterização (fazer antes de fixar a fs)

Rodar o `app17-10` uma vez a 500 Hz gravando raw, tirar a FFT no notebook e localizar o pico —
esse é o RPM real do motor. Só então fixar a fs de produção. É a lição de **medir o fenômeno
antes de escolher como amostrá-lo**, e vale registrar no README do app17-11.

### 5-REV.4) Janela: 2 s (1000 amostras @ 500 Hz)

O usuário perguntou entre 1 s e 2 s. **2 s.** A curtose é estimador de 4º momento, com erro
padrão ≈ `sqrt(24/N)`:

| janela @ 500 Hz | N | erro padrão da curtose |
|---|---|---|
| 1 s | 500 | 0,22 |
| **2 s** | **1000** | **0,155** |

Como a curtose é justamente a feature que separa falha impulsiva (item 3), a janela maior se
paga. Memória: 1000 × 3 × 4 B = 12 KB, irrelevante nos 320 KB do ESP32. Custo: metade das
janelas por minuto — 2 min de rodada para as mesmas 60 janelas.

### 5-REV.5) De onde vem o label: SEQUÊNCIA CÍCLICA DE CLASSES

> Substitui a ideia anterior de digitar no Serial Monitor. **Não há teclado no fluxo.**

O firmware guarda uma **sequência fixa de 5 classes**. Cada coleta usa a classe da posição
atual e avança para a próxima. O aluno só encaixa no gabarito, ajusta o motor no botão 1 e
toca o botão 2.

```c
const char* SEQUENCIA[] = { "desligado", "operando", "inclinado_frente",
                            "inclinado_tras", "anomalia" };
const int  N_CLASSES          = 5;
const int  JANELAS_POR_RODADA = 30;   // 30 janelas de 2 s = 60 s por rodada
const int  SETTLE_MS          = 1000; // mao do operador saindo do conjunto
```

**A sequência é CÍCLICA.** Depois da 5ª classe, o próximo toque volta para `desligado` com
`rodada = 02`. Isso é obrigatório, não opcional: uma única passada dá 1 rodada por classe, e os
itens **4b (`LeaveOneGroupOut`) e 4c (curva de aprendizado) exigem ao menos 2, idealmente 5**.
Cinco voltas completas = 5 rodadas por classe = 150 janelas por classe.

Orçamento de coleta: 5 classes × 5 voltas × 1 min = **25 min de gravação**, mais o
reposicionamento entre rodadas.

**Contar janelas, não segundos.** Usar `JANELAS_POR_RODADA` em vez de um cronômetro de 60 s:
toda rodada sai com exatamente o mesmo número de linhas, sem depender de arredondamento de
`millis()`.

### 5-REV.5b) Botão 2: um único gesto, e avisos de preparo no monitor

**O botão 2 tem uma ação só:** toque → settle de 1 s → coleta `JANELAS_POR_RODADA` janelas da
classe atual → para sozinho → avança a sequência. **Sem toque longo, sem pular etapa.**

O problema que o pulo resolveria era a fita da `anomalia`: ela precisa ser colada antes da
classe 5 e **removida** antes da volta seguinte, senão `operando` com fita vira `anomalia`
disfarçada. **Decisão do usuário: resolver por aviso no monitor, não por controle no botão.**
O ciclo roda reto pelas 5 classes e o monitor diz o que fazer em cada transição.

Avisos obrigatórios, em **CAIXA ALTA** e emoldurados, no mesmo espírito da guarda da 5-REV.5c.
Antes da classe 5:

```
==================================================
PROXIMA [5/5] anomalia   rodada=02   motor: LIGADO
>>> COLE A FITA NA HELICE AGORA <<<
    depois toque o botao 2
==================================================
```

Ao virar para a próxima volta (classe 1), a contrapartida:

```
==================================================
PROXIMA [1/5] desligado  rodada=03   motor: DESLIGADO
>>> REMOVA A FITA DA HELICE <<<
    depois desligue o motor e toque o botao 2
==================================================
```

Escrever esses banners em **ASCII puro, sem acentos** — Serial Monitor mal configurado embaralha
UTF-8, e um aviso ilegível não cumpre a função. (No resto das mensagens seguir o estilo já usado
nos outros sketches.)

**Rodada que saiu ruim** (aluno esbarrou na bancada, motor engasgou): não há botão para refazer.
Descartar depois, no notebook, filtrando por `classe` + `rodada` — as duas colunas existem
justamente para isso. É melhor lição que esconder a limpeza no firmware: identificar e remover
uma rodada contaminada **faz parte do pipeline de dados**, e o aluno precisa aprender a fazê-lo.
Registrar o descarte numa célula markdown do notebook 2.6.

### 5-REV.5c) Guarda contra dessincronização

O risco central deste desenho é o aluno perder a conta da posição na sequência — a partir daí
tudo sai rotulado errado, **em silêncio**. Duas defesas obrigatórias:

**1. Validar o estado do motor.** O firmware controla o motor pelo botão 1, então sabe o estado.
A sequência sabe o que cada classe espera: `desligado` exige motor parado; as classes 2–5 exigem
motor ligado. Se não bater, **recusar iniciar** e imprimir o aviso. Elimina o erro mais provável
do protocolo.

> **Limite que não pode ser ultrapassado:** o firmware **não pode** validar a inclinação lendo o
> acelerômetro. Isso seria usar o sensor para rotular o dado que o modelo depois vai aprender a
> classificar — vazamento. Validar estado de **atuador que o próprio firmware comanda** é
> legítimo; inferir a classe a partir do **sensor** não é. Não "melhorar" isso depois.

**2. Imprimir sempre o próximo passo em destaque**, antes de cada toque:

```
PROXIMA [3/5] inclinado_frente  rodada=02  (motor deve estar LIGADO)
> posicione no gabarito e toque o botao 2
```

E durante a coleta, o progresso:

```
COLETANDO [3/5] inclinado_frente rodada=02 ... janela 12/30
```

Opcional, se o usuário quiser ler sem o monitor: LED externo pisca o índice da classe
(1 a 5 piscadas) enquanto aguarda o toque.

### 5-REV.6) O que o firmware imprime

Uma linha CSV por janela, a cada 2 s, com **todas** as features do item 3a (as 7 originais mais
`std_mag`, `p2p_mag`, `crest_mag`, `kurt_mag`, `zcr_mag`) — o exercício do item 3 depende de
elas estarem todas presentes para depois serem comparadas em subconjuntos.

Acrescentar as colunas que a Forma 2 obtinha do Python e que aqui o ESP32 sabe **melhor**:

- `classe` — da sequência (5-REV.5); é o `label` do dataset
- `rodada` — **incrementada a cada volta completa da sequência**, não a cada toque. Todas as
  5 classes da 1ª volta são `rodada=01`; da 2ª volta, `rodada=02`. É o `group` do
  `LeaveOneGroupOut` (item 4b), então precisa identificar a **passada**, não a coleta individual.
- `fs_real` — `N_amostras * 1000.0 / (millis_fim - millis_inicio)`, a taxa efetiva medida na
  própria janela. Substitui com vantagem o cálculo por `timestamp` do item 2d, que dependia da
  hora de recepção no PC.

Cabeçalho CSV sugerido (imprimir uma vez no `setup()`):

```
classe,rodada,janela,fs_real,mean_ax,mean_ay,mean_az,std_ax,std_ay,std_az,rms_mag,std_mag,p2p_mag,crest_mag,kurt_mag,zcr_mag
```

Manter os marcadores `# INICIO` / `# FIM` do item 5c — eles continuam úteis e o Node-RED os
ignora pelo mesmo critério.

### 5-REV.7) Fases de entrega

1. **Fase serial:** ESP32 janela, calcula, imprime CSV no monitor a cada 2 s, com a sequência
   de classes e os dois gestos do botão 2. Sem WiFi/MQTT. É a fase que o usuário quer que o
   aluno rode primeiro.
2. **Fase MQTT:** acrescenta WiFi + publish JSON por janela, no molde do `app17-9`
   (mesmo broker da IoT-platform, **nunca** `test.mosquitto.org`), fluxo Node-RED → InfluxDB.
   O JSON leva os mesmos campos do CSV, incluindo `classe`, `rodada` e `fs_real`.
3. **Colab:** notebook 2.6 lendo do InfluxDB, no molde dos notebooks 1.3/1.5.

Implementar a fase 1 completa antes de acrescentar a 2 — o firmware deve funcionar e ser
demonstrável sem rede.

**Cuidado na fase 2:** o `publish` do MQTT pode bloquear por dezenas de ms em reconexão. Como a
janela só fecha a cada 2 s, há folga de sobra — mas a publicação tem que acontecer **entre**
janelas, nunca dentro do laço de amostragem a 500 Hz. O `app17-8` já resolve isso; seguir o
mesmo padrão e conferir `fs_real` na saída para confirmar que a taxa não caiu.

---

### 5a) Desenho experimental (confirmado com o usuário)

| classe | motor | inclinação | desbalanceamento |
|---|---|---|---|
| `desligado` | desligado | plano | não |
| `operando` | ligado | plano | não |
| `inclinado_frente` | **ligado** | ~+25° | não |
| `inclinado_tras` | **ligado** | ~−25° | não |
| `anomalia` | ligado | plano | **sim** (fita/massa na hélice) |

As classes de inclinação são coletadas com o **motor ligado**. É isso que torna o problema
interessante: `mean_ax`/`mean_az` separam as inclinações, `std_*`/`kurt_mag` separam
desligado/operando/anomalia, e **nenhuma família sozinha resolve as 5 classes**. Se a inclinação
fosse com o motor desligado, o problema voltaria a ser trivial e o exercício do item 3 perderia
o desfecho.

Expectativa honesta a registrar no README: `desligado` vs. resto continuará fácil. O par que
deve gerar confusão real é `operando`×`anomalia`. As duas classes de inclinação tendem a
separar bem, porque o gabarito 3D (item 5b) mantém o ângulo repetível entre rodadas.

### 5b) Dinâmica de operação (definida pelo usuário)

- **Botão 1 (GPIO 26): liga/desliga o motor** (toggle)
- **Botão 2 (GPIO 25): liga/desliga a coleta da rodada** (toggle)

> **SUBSTITUÍDO PELA 5-REV.5.** Nem Python, nem teclado: a classe vem de uma **sequência
> cíclica** no firmware e o botão 2 **não é toggle** — um toque dispara uma rodada completa que
> **para sozinha** após `JANELAS_POR_RODADA` janelas. Ler a 5-REV.5, 5-REV.5b e 5-REV.5c;
> desta seção, aproveitar apenas o parágrafo do gabarito 3D abaixo.

Fluxo real (5-REV.5): o monitor anuncia a próxima classe da sequência → o aluno ajusta o motor
no botão 1 conforme anunciado → posiciona no gabarito → **um toque** no botão 2 → settle de 1 s
→ 30 janelas coletadas → para sozinho e anuncia a próxima classe.

**Gabarito 3D:** o usuário vai imprimir um modelo 3D com as posições de inclinação, e o aluno
apenas encaixa o motor em cada uma. Isso torna o ângulo **repetível entre rodadas**, que era o
principal risco de `inclinado_frente` × `inclinado_tras` se misturarem. Com o gabarito, essas
duas classes devem separar bem.

### 5c) Marcadores de rodada na Serial

O firmware imprime, **fora do formato numérico de 3 campos**:

```
# INICIO motor=1
# FIM n=6000
```

No **app17-11 (Forma 1)** esses marcadores continuam, apenas emoldurando as linhas de features
em vez das de raw. Incluir a classe no `# INICIO`, para o CSV ficar auto-explicativo:

```
# INICIO classe=inclinado_frente rodada=03 motor=1
# FIM rodada=03 janelas=30 fs_real=498.7
```

**As mudanças no `coletor_raw.py` descritas a seguir valem apenas para a Forma 2**
(`app17-10`), que continua existindo para as lições de raw e re-janelamento. O app17-11 não
usa o coletor.

- **ecoar** as linhas `#` no console
- ao receber `# INICIO`, **abrir um arquivo novo** com o próximo número de rodada
- ao receber `# FIM`, **fechar o arquivo e voltar a aguardar** o próximo `# INICIO` —
  **não encerrar o programa**. Imprimir um resumo da rodada (número, amostras, arquivo).
- `Ctrl+C` encerra a sessão inteira; se houver rodada aberta, fechar o arquivo antes de sair

Esse desenho é o que torna barato coletar **5 rodadas por classe**, que é exatamente o que os
itens 4b (`LeaveOneGroupOut`) e 4c (curva de aprendizado) precisam para funcionar. No app17-11
o mesmo se obtém percorrendo a sequência cíclica 5 vezes (5-REV.5), sem Python.

### 5d) Settle após o botão 2

`SETTLE_MS = 1000` entre o toque do botão 2 e a primeira amostra, para a mão do operador sair do
conjunto. Imprimir o `# INICIO` **depois** do settle.

No app17-11 o settle é a **única** proteção contra o transiente — não há descarte posterior,
porque as janelas já chegam prontas do edge e o notebook 2.4 não entra no caminho (5h). Se na
prática 1 s não bastar, aumentar `SETTLE_MS`; é constante no topo do arquivo justamente para
isso. Na Forma 2 (`app17-10`) o descarte de 3 s do notebook 2.4 continua valendo.

### 5e) Pinagem e LEDs

Reaproveitar a do `app17-9` (que já dirige os dois LEDs):

| função | GPIO |
|---|---|
| MPU SDA | 19 |
| MPU SCL | 18 |
| Botão MOTOR | 26 |
| Botão COLETA | 25 |
| LED externo — **coleta ativa** | 27 |
| LED onboard — **motor ligado** | 2 |
| Motor IN1 / IN2 | 22 / 23 |

Controle do motor: copiar `ligarMotor()`/`desligarMotor()` do `app17-7-TesteMotor` (pulso de
partida em 180 e depois `PWM_MOTOR = 100`).

### 5f) Sensor default

`MPU6500 mpu(Wire);` **ativo** (é o hardware real do usuário), com a linha do `MPU6050`
comentada logo abaixo, seguindo o estilo dos outros sketches. **DLPF conforme a 5-REV.2**
(`DLPF_MAX` a 500 Hz), não o do item 1 — o item 1 vale para os apps que amostram a 100 Hz.

### 5g) Arquivos a criar

```
app17-11-MotorMultiClasse/
├── src/app17-11-motormulticlasse.cpp
├── platformio.ini      # Fase 1: só okalachev/FlixPeriph.
│                       # Fase 2 acrescenta PubSubClient + ArduinoJson (ver 5-REV.7)
├── diagram.json        # ESP32 + MPU + motor + 2 botões + LED
├── wokwi.toml
├── README.md           # operacional (ver "Documentação"): montagem, pinagem,
│                       # protocolo dos 2 botões, como rodar. Sem justificativa de projeto.
└── .gitignore
```

`diagram.json` e `wokwi.toml` entram por consistência com o resto do repositório, mas o
**README deve deixar claro que este app foi projetado para hardware físico** — o motor e a
inclinação real são o ponto do experimento.

### 5h) Notebook 2.6

> **Ajustado pela 5-REV:** como o app17-11 é Forma 1, o 2.6 **não** passa pelo notebook 2.4.
> As features já chegam prontas do ESP32 — o 2.6 carrega direto, como fazem o 1.3/1.5.

`analytics/notebooks/2.6_multiclasse_motor.ipynb` faz só o treino, com **duas fontes possíveis**
na célula de carga (o aluno escolhe conforme a fase da 5-REV.7):

- **Fase serial:** CSV salvo do Serial Monitor (as linhas de feature, descartando as `#`)
- **Fase MQTT:** consulta ao InfluxDB, no molde do notebook 1.3

Conteúdo:

- matriz de confusão 5×5 e `f1_macro`
- **o experimento por família de features** (o desfecho do item 3): treinar com só `mean_*`,
  com só as features AC, e com todas — mostrando que **cada família resolve um subconjunto
  diferente das classes** e nenhuma resolve todas
- `LeaveOneGroupOut` por `rodada` (coluna que o firmware já emite — ver 5-REV.6)
- checagem de `fs_real`: avisar se alguma janela desviar mais de 5% dos 500 Hz nominais.
  Substitui o cálculo por `timestamp` do item 2d, que só existe na Forma 2.
- export micromlgen multiclasse

---

## Item 6 — `3.1_dataset_real_cbm.ipynb` (dados reais de motor)

Notebook novo, numeração `3.x` porque é uma trilha diferente: validação em dados de referência,
não a cadeia "Forma 2" dos notebooks 2.x.

### 6a) Fonte

`https://github.com/analogdevicesinc/CbM-Datasets`
Caminho: `SampleMotorDataLimerick/SpectraQuest_Rig_Data_Voyager_3/Data_ADXL356C/`

Bancada SpectraQuest Machinery Fault Simulator, acelerômetro **Analog Devices ADXL356**.

### 6b) Formato (verificado baixando os arquivos)

- **separador `;`**, **sem cabeçalho**
- coluna 0 = tempo em segundos; colunas 1, 2, 3 = X, Y, Z
- **a primeira linha tem 3 campos extras** — resolver com `usecols=[0,1,2,3]`
- **fs = 20 kHz** (passo de 5e-5 s), **2 s por arquivo**, 40.000 linhas
- **1,8 MB por CSV, 130 CSVs, 229 MB no total**

```python
df = pd.read_csv(caminho, sep=';', header=None, usecols=[0,1,2,3],
                 names=['t','x','y','z']).astype(float)
```

### 6c) Convenção de nomes (decodificada)

`RPM_rolamento_eixo_desbalanceamento_alinhamento_carga.Wfm.csv`

| campo | valores | significado |
|---|---|---|
| 0 | `0600` `1200` `1800` `2400` `3000` | RPM (26 arquivos cada) |
| 1 | `GoB`(70) `HBF` `HIR` `HOR` `LBF` `LIR` `LOR`(10 cada) | rolamento: bom / esfera e pistas interna-externa, pesado e leve |
| 2 | `GS`(120) `BS`(10) | eixo bom / empenado |
| 3 | `BaLo`(90) `VLIL` `LImL` `HImL` `VHIL`(10 cada) | balanceado / desbalanceamento muito leve→muito pesado |
| 4 | `WA`(120) `MA`(10) | bem alinhado / desalinhado |
| 5 | `00lb` `11lb` | carga (65 cada) |

São 13 configurações de falha × 2 cargas × 5 RPM = 130 arquivos.

### 6d) Unidades e licença — dois avisos obrigatórios no notebook

- **Unidades são volts, não m/s².** Os valores ficam em torno de 0,89–0,91 nos três eixos: é o
  bias ratiométrico do ADXL356 (≈ Vs/2 = 0,9 V), não gravidade. Não afeta a classificação
  (StandardScaler normaliza), mas o notebook não pode chamar isso de m/s².
- **O repositório não declara licença.** Portanto: **baixar em runtime** de
  `raw.githubusercontent.com`, com atribuição à Analog Devices, e **não versionar** os CSVs no
  repositório da disciplina. Isso também evita somar 229 MB ao repo.

Para uma sessão de Colab, selecionar um subconjunto — p.ex. 1800 RPM, `00lb`, um arquivo por
configuração = 13 arquivos ≈ 23 MB.

### 6e) Estatísticas já medidas (1800 RPM, `00lb`, eixo X)

Usar como sanity check: se o notebook reproduzir estes números, a leitura está correta.

| condição | média x | std x | curtose x |
|---|---|---|---|
| `GoB_GS_BaLo_WA` (saudável) | 0,8911 | 0,00964 | 0,27 |
| `GoB_GS_VHIL_WA` (desbal. muito pesado) | 0,8909 | 0,01659 | 0,07 |
| `HIR_GS_BaLo_WA` (pista interna pesada) | 0,8883 | 0,05292 | **18,69** |
| `GoB_BS_BaLo_WA` (eixo empenado) | 0,8913 | 0,01020 | 0,07 |

### 6f) As três lições que o notebook deve extrair

1. **`mean_*` é constante em todas as falhas** (0,888–0,891). É o bias DC do sensor, não o
   fenômeno. É a confirmação empírica, com motor real, da lição do item 3 — muito mais forte
   que o argumento teórico.
2. **`kurtosis` resolve falha de rolamento** (0,27 → 18,69) onde `std` apenas dobra. Valida a
   feature acrescentada no item 3.
3. **Eixo empenado é invisível no domínio do tempo** (std 0,0102 contra 0,0096 do saudável).
   É o caso que exige FFT — o melhor argumento possível para introduzir o domínio da frequência.

### 6g) O contraste de taxa de amostragem (o ponto alto)

Rodar a mesma classificação duas vezes: **a 20 kHz** e **decimada para 100 Hz** (com filtro
anti-aliasing, `scipy.signal.decimate`), mostrando que o desbalanceamento sobrevive à decimação
e as falhas de rolamento desaparecem.

Isso responde com medição, e não com opinião, às perguntas originais do usuário sobre qual fs e
quais features são necessárias — e amarra de volta no item 1 (por que o DLPF importa).

### 6h) Janelamento

A 20 kHz, 2 s = 40.000 amostras. Usar janelas de **4096 amostras** (≈ 0,2 s) → ~9 janelas por
arquivo; 130 arquivos → ~1170 janelas.

**Janelas do mesmo arquivo não são independentes.** O split tem que agrupar por arquivo
(`GroupKFold`/`LeaveOneGroupOut` com `groups=arquivo`) — é a mesma lição do item 4, agora com
dados reais onde ela realmente muda o resultado.

---

## Verificação (o que dá para provar nesta máquina)

| o que | como | disponível |
|---|---|---|
| Firmware compila | `pio run` nos 3 projetos | **sim** — PlatformIO instalado |
| Lógica dos notebooks roda | extrair as células para script e executar | **sim** — sklearn 1.9, pandas 3.0, numpy 2.5, scipy 1.18 |
| Notebook 3.1 ponta a ponta | baixar o subconjunto do CbM e rodar | **sim** — é dado real, não sintético |
| Notebooks 2.4/2.5/2.6 ponta a ponta | exigem coleta no hardware do usuário | **não** — validar só a lógica |
| Export micromlgen | `micromlgen` não está instalado localmente | **não** — a célula continua `!pip install` no Colab |

### Pontos que continuam em aberto (não são omissão — são dependências reais)

Registrados aqui para que ninguém os apresente como resolvidos:

1. **fs = 500 Hz é recomendação, não medição.** Vem da aritmética RPM→Hz (5-REV.2) e do
   orçamento de I2C verificado, mas o RPM real do motor do usuário é desconhecido. Reconfirmar
   após o passo de caracterização (5-REV.3). **Não bloquear nisso** — ver "Dependência externa".
2. **A previsão do item 6g** (falhas de rolamento somem ao decimar para 100 Hz) é fundamentada,
   mas é previsão. O notebook deve **reportar o que mediu**, não confirmar a expectativa.
3. **micromlgen** não é verificável nesta máquina.
4. **Notebooks 2.4/2.5/2.6** só rodam ponta a ponta com a coleta do usuário. Validar a lógica
   com os dados do CbM reformatados, e dizer claramente que a execução real ficou pendente.

O notebook 3.1 é o que garante que o pipeline de janelamento/features/split funciona de fato,
com dados reais de um motor real. Por isso ele substitui com vantagem a ideia anterior de
gerar dados sintéticos.

---

## Restrições de estilo (seguir o repositório)

- Código **linear**, sem classes nem abstrações. Configuração no topo do arquivo.
- Comentários e markdown **em português**.
- Célula markdown **explicando o conceito antes** de cada célula de código.
- Firmware: amostragem por `millis()`, **nunca `delay()`** no laço de amostragem.
- Não alterar `app17-0` até `app17-8` — são código de aula já dado.
- Broker MQTT, quando houver, é o da IoT-platform da disciplina. **Nunca** `test.mosquitto.org`.

---

## Ordem de execução sugerida

1. **Passo 0** — linha de base do eval *(já executado)*
2. **Item 1** — firmware app17-9/10 (rápido, mecânico, compila para validar)
3. **Item 2** — coletor + 2.4 (Forma 2)
4. **Item 3** — features no 2.4 + seção comparativa no 2.5
5. **Item 4** — baseline/GroupKFold/curva no 2.5
6. **Item 6** — notebook 3.1 do CbM (**antes** do item 5: é o que valida a lógica de
   janelamento e features com dados reais, sem depender de coleta)
7. **Item 5 / 5-REV** — app17-11 em Forma 1. Nesta ordem interna:
   1. **Fase serial** do firmware (5-REV.7 fase 1): janela de 2 s @ 500 Hz, features no CSV
      do Serial Monitor, classe pela sequência cíclica (5-REV.5), um toque do botão 2 por
      rodada, guarda do estado do motor e avisos da fita. Compilar com `pio run`.
   2. **Caracterização do RPM** (5-REV.3) — o usuário roda no hardware dele e informa o pico
      da FFT. Só então confirmar a fs de 500 Hz.
   3. Notebook 2.6 lendo o CSV da fase serial.
   4. **Fase MQTT** (5-REV.7 fase 2) + fluxo Node-RED → InfluxDB + fonte InfluxDB no 2.6.
8. Atualizar o `analytics/README.md` **do develop** com os notebooks novos, o mapa
   arquivo → item da Sprint, e o fato de que agora há **dois** apps em Forma 1 (app17-9 a
   100 Hz para vibração binária; app17-11 a 500 Hz para as 5 classes do motor).
   **Não** criar esse arquivo no eval.

### Dependência externa a respeitar

O passo 7.2 depende do hardware do usuário. **Não bloquear nele:** implementar a fase serial
com `FS_HZ = 500` como default parametrizado no topo do arquivo, entregar, e deixar registrado
que o valor deve ser reconfirmado após a medição do RPM. Se a medição mostrar rotação acima de
~180 Hz (>11.000 RPM), subir a fs e reavaliar o DLPF.
