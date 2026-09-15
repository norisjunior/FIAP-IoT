# Prompt para iniciar a sessão do SmartBag

Cole o bloco abaixo, inteiro, numa conversa nova com o assistente. Ele carrega o
contexto do projeto, as convenções de trabalho e o estado de execução.

O detalhe técnico do app19 não está aqui de propósito — está em
[PLANO-SmartBag-SensorFusion.md](PLANO-SmartBag-SensorFusion.md), e o prompt manda ler.

---

```text
Sou professor de IoT na FIAP, 2º ano de graduação. Estamos construindo a transição
da trilha de IoT para Machine Learning, e quero continuar do ponto em que parei.

## Onde estão as coisas

- Trabalho em: C:\Projects\FIAP-IoT-eval  (branch fiapiot/eval)
- Planos e material de aula: C:\Projects\FIAP-IoT-develop  (branch fiap-iot-desenv)

O projeto novo é o app19:
  C:\Projects\FIAP-IoT-eval\app19_SmartBag_SensorFusion

O plano completo dele, já escrito e em dia, é:
  C:\Projects\FIAP-IoT-develop\PLANOS_DEPLOY\PLANO-SmartBag-SensorFusion.md

**Leia esse plano inteiro antes de qualquer coisa.** Ele tem o contrato de dados,
o protocolo de coleta, as cinco etapas e o estado de execução. Não o reescreva
sem me perguntar.

## De onde isto vem

O app19 é a evolução do NexoLog (apps 14, 15 e 16), que acabamos de fechar:
ESP32 com DHT22, HC-SR04 e MPU, publicando JSON por MQTT; Node-RED mostrando;
n8n decidindo e avisando; InfluxDB Cloud guardando; Grafana visualizando.
Vou rebatizá-lo de **Smart Delivery Bag**.

O app19 acrescenta LDR e dois botões (um para rodada, outro para coleta), e
substitui projetos antigos que ficam em:
  C:\Projects\FIAP-IoT-develop\OLD-ML-com-IoT

Antes do app19, em aula, eu ensino:
  app17-GerarDatasetSinais_IMU  (dataset de sinais, 8 sub-apps)
  app18-Inferencia-AI-API_Sinais-MultiClass_IMU  (inferência por API)

## O que eu quero

A sequência: coleta → dataset → Random Forest → API na cloud → RF embarcada
com micromlgen → MLP TinyML. Montando os dispositivos em sala de verdade.

Duas restrições que importam mais que qualquer outra coisa:

1. **Poucas rodadas e cenários.** Duas rodadas completas, sete situações. Se
   ficar cansativo ou improdutivo em sala, não serve. Prefiro cortar escopo a
   ter um protocolo que ninguém termina.
2. **Transição natural.** Código simples e parecido com o que já tenho — no
   app14/15/16 e no meu histórico de ML com IoT. Arduino/PlatformIO/Wokwi,
   funções simples, namespaces ESP32Sensors, comentários em português sem
   acento, blocos de configuração no topo. Nada de framework, classe de
   sensor, biblioteca compartilhada ou refatoração geral.

## Como quero que você trabalhe

Isto vem de uma sessão longa e funcionou bem:

- **Confira, não presuma.** Leia os arquivos antes de afirmar qualquer coisa
  sobre eles. Quando terminar uma mudança, valide por script (JSON válido,
  fios sem órfãos nos fluxos, links de Markdown, guia batendo com o código
  linha a linha). Vários erros meus só apareceram assim.
- **Diga o que não testou.** Você não tem ESP32, Node-RED, n8n, InfluxDB nem
  Grafana rodando aí. Quando não der para executar, diga — e diga o que eu
  devo olhar primeiro na bancada.
- **Guias no formato CONSTRUIR-*.** Iterações que compilam e rodam, cada uma
  fazendo mais uma coisa. Bloco de código, "Funcionou?" com checklist, e uma
  tabela "Deu errado | Onde olhar". Mínimo de texto explicativo: quero
  indicação de onde colocar o código, não ensaio. Português do Brasil, com
  acentuação correta na prosa (código sem acento).
- **Fluxos com o mínimo de código.** No Node-RED e no n8n, prefiro nós de
  configuração a nó de função. Se der para resolver com Switch, Change, Set
  ou Split Out, é melhor que JavaScript.
- **Planeje antes de implementar** quando a mudança for grande, e me mostre o
  plano. Pergunte quando a resposta mudar o desenho; decida sozinho o resto.
- **Commits em português**, sem acento, explicando o porquê e não só o quê.
  Terminar com: Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
  Commite só o que você mexeu — eu costumo ter outras coisas em andamento.

## Onde estamos

Pelo plano, o app19 está implementado (firmware, fluxo, três notebooks, guias)
e compilando, mas **nada foi validado com hardware real ou coleta humana**.
Os apps 20, 21 e 22 ainda não existem.

Comece lendo o plano e o que está no app19, me diga o que encontrou e qual
você acha que é o próximo passo. Não saia implementando.
```

---

## Por que o prompt está assim

**Manda ler o plano em vez de repetir o plano.** O `PLANO-SmartBag-SensorFusion.md` já
tem contrato de dados, as sete situações, as cinco etapas e os critérios de conclusão.
Duplicar isso no prompt criaria duas versões que divergem.

**A última linha é "não saia implementando".** Pelo plano, o próximo passo real é coletar
duas rodadas com hardware e rodar os notebooks com CSV real — não há código novo pendente
até isso acontecer. Sem essa instrução, uma sessão nova começaria pelos apps 20–22, que é
a ordem errada.

**As convenções de trabalho vêm da sessão do NexoLog**, onde foram descobertas na
prática: os guias em iterações, o mínimo de prosa, a preferência por nó de configuração
em vez de nó de função, e a disciplina de validar por script. Os dois erros que a leitura
não pegou naquela sessão — a opção `Only Message` do MQTT Trigger e o `device` indo como
field e tag no InfluxDB — só apareceram na bancada do professor, e é por isso que o
prompt pede para declarar o que não foi testado.
