# Prompt — 3 slides de InfluxDB na Aula 07

Arquivo alvo: `IoT - Aula 07 - Node-RED.pptx` (34 slides hoje, 37 ao final).

## Como usar

1. Abra o PowerPoint com o assistente.
2. Cole o **Bloco fixo** uma vez, no início da conversa.
3. Cole a **Seção 1** depois. É a única seção deste arquivo.

Estrutura de hoje, para referência:

| Slides | O que é |
|---|---|
| 1–19 | capa, CPS, MQTT, JSON, conceito de dashboard |
| 20 | divisória **HANDS ON!** |
| 21 | Aplicação IoT White Label: NexoLog |
| 22–27 | Node-RED: plataforma, aplicação, dashboard, layout |
| 28–31 | Node-RED + Notificações / Telegram |
| 32–33 | Preparação para a próxima aula |
| 34 | encerramento |

Os três slides novos entram **depois do slide 27**, entre o fim do dashboard e o começo
das notificações.

---

## Bloco fixo

> Cole isto primeiro.

```
Você vai acrescentar slides ao arquivo "IoT - Aula 07 - Node-RED.pptx".

CONTEXTO
- Disciplina de IoT, graduação FIAP, 2º ano.
- Esta aula é sobre Node-RED: receber dados do ESP32 por MQTT e montar um
  dashboard.
- O caso de uso é a NexoLog, uma empresa fictícia que acompanha entregas de
  carga sensível. É um projeto white label: a mesma caixa vira bag de
  entregador, e aí "tampa aberta" passa a ser indício de fraude.
- A caixa tem um ESP32 com DHT22 (temperatura e umidade), HC-SR04
  (distância até a tampa) e MPU6050/MPU6500 (movimentação, em m/s²).
- O firmware publica um JSON por segundo no tópico
  FIAPIoT/nexolog/equipe01/dados com: device, temp, umid, dist,
  accel_x, accel_y, accel_z, movimentacao.

ONDE ESTA AULA FICA NA TRILHA
- Aula 07 (esta): Node-RED mostra os dados em tempo real.
- Aula 08 (próxima): n8n assina o MESMO tópico, decide se algo está fora
  do limite e avisa no Telegram.
- Aula 15 (mais adiante): consultar o InfluxDB com Python para montar
  dataset de ML. Os slides que você vai criar são a primeira vez que o
  aluno vê um banco de série temporal — apresentar, não aprofundar.

O QUE JÁ ESTÁ PRONTO - NÃO ALTERE
- Todos os 34 slides existentes.
- Você só vai INSERIR três slides novos, depois do slide 27 (o último de
  dashboard, antes de "Node-RED + Notificações").

ESTILO - obrigatório em todos os slides que você criar
- Português do Brasil.
- No máximo 5 marcadores por slide, frases curtas.
- TODO slide precisa de um elemento visual construído com FORMAS do
  PowerPoint (retângulos, setas, círculos, linhas). Nada de clipart, nada
  de imagem externa.
- Sempre que possível, comparação lado a lado.
- Notas do apresentador em todos os slides.
- Manter o fundo, as fontes e o tamanho de título do deck atual.

CÓDIGO DE CORES DA TRILHA - use consistentemente
- AZUL: medição de sensor
- LARANJA: valor calculado
- ROXO: ajuste do operador
- CINZA: metadado
- VERDE: dentro da faixa
- VERMELHO: fora da faixa
```

---

## Seção 1 — Guardar o histórico no InfluxDB

> Cole depois do Bloco fixo.

```
Insira TRÊS slides novos imediatamente depois do slide atual 27 (o último
slide de dashboard do Node-RED, logo antes do slide "Node-RED +
Notificações"). Numere-os como 28, 29 e 30; os slides seguintes deslocam.

======================================================================
SLIDE A - Título: "O gráfico não é memória"
======================================================================

Marcadores (máximo 5):
- O gráfico do dashboard guarda os últimos 5 minutos.
- Um F5 e ele começa do zero.
- "Como foi a entrega de ontem?" não tem resposta ali.
- Série temporal: cada medição com a hora em que chegou.
- O dashboard mostra agora; o banco guarda para depois.

VISUAL - comparação lado a lado, feita com formas:
Dois retângulos grandes, lado a lado, mesma altura.

Esquerda, título "Dashboard" (CINZA no topo): dentro, uma linha de gráfico
serrilhada AZUL curta, ocupando só o terço direito do retângulo. O resto do
retângulo, à esquerda dessa linha, fica vazio e hachurado em CINZA claro,
com o texto pequeno "sem memória". Embaixo, o rótulo "últimos 5 minutos".

Direita, título "InfluxDB" (CINZA no topo): a MESMA linha serrilhada AZUL
no terço direito, mas continuando para a esquerda ao longo de todo o
retângulo, mais clara conforme se afasta. Embaixo, o rótulo "desde o
primeiro dia".

Entre os dois retângulos, uma seta LARANJA apontando da esquerda para a
direita com o texto "mesmo dado, outro destino".

NOTAS DO APRESENTADOR:
Pergunte à turma antes de mostrar o slide: "se eu fechar essa aba, para
onde vai o que a caixa mediu nos últimos dez minutos?" A resposta é: para
lugar nenhum. O gráfico do Node-RED vive na memória do navegador. Isso não
é defeito — é a função dele, mostrar o agora. O problema aparece na
pergunta que um cliente faz de verdade: "a carga estragou em que ponto do
trajeto?" Aí é preciso ter guardado. Banco de série temporal é um banco
otimizado para dado que chega o tempo todo, sempre com um horário junto, e
que quase nunca é alterado depois — só consultado por intervalo de tempo.

======================================================================
SLIDE B - Título: "Tag ou field?"
======================================================================

Marcadores (máximo 5):
- Cada ponto gravado tem tags, fields e um horário.
- Tag: identifica de quem é a medição. Texto, indexada.
- Field: o número medido. É o que você soma, ou tira média.
- device é tag. As sete medições são fields.
- A pergunta que você vai fazer depois decide o que é o quê.

VISUAL - anatomia de um ponto, feita com formas:
Um retângulo largo horizontal, dividido em quatro faixas verticais, como um
cartão. Da esquerda para a direita:

Faixa 1, VERDE-CINZA, rótulo acima "measurement": texto "nexolog".
Faixa 2, CINZA, rótulo acima "tag": texto "device = NexoLogEquipe01".
Faixa 3, AZUL, rótulo acima "fields" e a mais larga das quatro: quatro
linhas de texto pequeno, "temp = 24.0", "umid = 55.2", "dist = 10.4",
"movimentacao = 0.03".
Faixa 4, CINZA, rótulo acima "time": texto "2026-09-15 09:41:02".

Abaixo do retângulo, duas chaves com legendas:
Sob as faixas 1 e 2, uma chave CINZA: "por onde você filtra".
Sob a faixa 3, uma chave AZUL: "o que você calcula".

NOTAS DO APRESENTADOR:
Essa distinção parece burocrática e não é. Tag é indexada: filtrar por
device é barato, mesmo com milhões de pontos. Field não é: filtrar por
"temperatura acima de 30" custa varrer os dados. A regra prática: se você
vai usar para dizer DE QUEM é o dado, é tag; se vai usar para fazer conta,
é field. Errar isso não quebra nada hoje, mas deixa a consulta lenta lá na
frente. Vale registrar que movimentacao é o pico do último segundo, não a
leitura do instante — o firmware amostra o MPU vinte vezes por segundo e
guarda a maior. Na Aula 15 essa mesma estrutura vira DataFrame do pandas
para treinar um modelo.

======================================================================
SLIDE C - Título: "Dois fluxos, um tópico"
======================================================================

Marcadores (máximo 5):
- O mesmo tópico alimenta dashboard e banco.
- Um fluxo mostra. O outro grava.
- Nenhum dos dois sabe que o outro existe.
- Publique uma vez; quem quiser, assina.
- Na Aula 08, o n8n assina o mesmo tópico e decide.

VISUAL - diagrama de ramificação, feito com formas:
À esquerda, um retângulo AZUL "ESP32 NexoLog" com o texto pequeno
"1 leitura/s". Dele sai uma seta para um retângulo CINZA estreito e alto,
rotulado verticalmente "MQTT — FIAPIoT/nexolog/equipe01/dados".

Desse retângulo saem TRÊS setas para a direita, abrindo em leque:
- Seta 1, para um retângulo "Node-RED — Dashboard" com um mini-gauge
  desenhado dentro (semicírculo com ponteiro).
- Seta 2, para um retângulo "Node-RED — InfluxDB" com um mini-cilindro de
  banco desenhado dentro.
- Seta 3, para um retângulo "n8n — Telegram", desenhado TRACEJADO e em
  CINZA claro, com a etiqueta "Aula 08".

Sob o leque, uma linha de texto centralizada: "o dispositivo publica uma
vez e não sabe quem está ouvindo".

NOTAS DO APRESENTADOR:
Esse é o argumento do MQTT inteiro numa tela. Em HTTP o dispositivo
precisaria chamar cada destino, saber o endereço de cada um e ser alterado
sempre que aparecesse um novo interessado. Aqui ele publica num tópico e
acabou. Acrescentar o banco não exigiu recompilar o firmware nem mexer no
dashboard — foi importar um segundo fluxo. Na prática de hoje os dois
ficam rodando ao mesmo tempo, em abas diferentes do Node-RED. Deixe isso
visível na tela: duas abas, dois fluxos, a mesma caixa. E adiante que na
próxima aula entra a terceira seta, que é onde a decisão vai morar.
```

---

## Depois de aplicar

O `README.md` do app14 e o `CONSTRUIR-O-FLUXO.md` do Node-RED já descrevem o segundo
fluxo com esses mesmos termos, então a prática bate com o slide. O roteiro de bancada
está em `FIAP-IoT-eval/app14_CPS_e_Automation/Plataformas_config/NodeRED/CONSTRUIR-O-FLUXO.md`,
na seção "O segundo fluxo: guardar no InfluxDB".

Lembre de citar em aula que o token do InfluxDB **não** vem no arquivo importado: o
Node-RED guarda token como credencial e a exportação sempre remove. Cada aluno cola o
seu, junto com a organização e o bucket dele.
