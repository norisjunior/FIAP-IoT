# Memória — NexoLog

Empresa fictícia de entregas de cargas sensíveis. Projeto white label: a mesma caixa
vira bag de entregador, e aí "tampa aberta" é indício de fraude.

O projeto vive em `FIAP-IoT-eval`. As cópias `_v2` deste diretório foram removidas em
15/09/2026 — o conteúdo continua recuperável no commit `3e0ad6f`.

## Decisão vigente — 15/09/2026

| Etapa | Firmware | Plataforma |
|---|---|---|
| App14 | `app14_CPS_e_Automation/app14_NexoLog_PUB_only` — só publica | Node-RED mostra; n8n decide e avisa |
| App15 | `app15-Cloud/app15_NexoLog_PUB_SUB` — publica e assina | Node-RED decide e manda comando; Grafana |
| App16 | o mesmo do app15 | os mesmos fluxos, no Raspberry Pi |

**Quem decide mudou de lugar.** Até 09/09 o Node-RED calculava o estado da entrega e
publicava eventos num tópico próprio. Hoje não: o Node-RED do app14 só visualiza, e o
n8n assina o **mesmo** tópico do ESP32 para decidir. Isso eliminou o tópico `/eventos`,
a função `Estado da entrega` e o nó de supressão de repetição.

## Hardware

DHT22 no GPIO 4; TRIG 19 / ECHO 18; MPU SDA 22 / SCL 23; LED 21, só no app15. Todos do
lado direito da placa, para facilitar a montagem física.

FastIMU 1.3.0. `MPU_TYPE` seleciona MPU6050 (Wokwi) ou MPU6500 (placa). Aceleração
convertida de g para m/s². Sem calibração automática — o viés de fábrica vira piso na
leitura, e é a partir dele que se escolhe o limiar.

`movimentacao` é a magnitude da aceleração menos a gravidade: caixa parada fica perto de
zero em qualquer orientação. Substituiu a inclinação em graus, que media postura e não
chacoalho.

## Amostragem — um relógio por sensor

| Sensor | Ritmo | Por quê |
|---|---|---|
| DHT22 | 2,1 s, com cache | o módulo recusa leitura abaixo de 2 s; sem cache metade dos envios sairia nula |
| MPU | 50 ms, guardando o máximo | uma leitura por envio deixa o pico do sacolejo passar |
| HC-SR04 | 1 s | junto do envio |
| Publicação | 1 s | é o que a plataforma mostra |

O campo `movimentacao` publicado é o **pico do último segundo**, zerado após cada envio.

## Contrato

Dois tópicos: `FIAPIoT/nexolog/equipe01/dados` e `.../cmd`. Client ID
`NexoLogEquipe01`, um por equipe.

Payload de subida, oito campos: `device`, `temp`, `umid`, `dist`, `accel_x`, `accel_y`,
`accel_z`, `movimentacao`. Leitura que falhou vai como `null`.

Comando de descida: `{"alerta":"ON"}` ou `{"alerta":"OFF"}`. O dispositivo faz
`deserializeJson`, extrai o campo e aciona o LED. Não publica confirmação.

Saída no monitor serial em CSV, com `\r\n` e linha de título no `setup()`.

## Regras

Limiares, todos didáticos: temperatura > 30 °C, umidade > 70 %, distância > 25 cm,
movimentação > 3 m/s².

**App14** — os quatro limiares vivem nas regras do Switch do n8n, e em nenhum outro
lugar do fluxo. Um aviso de Telegram por variável que passar, com `send data to all
matching outputs` ligado e sem fallback.

**App15** — o comando ao LED exige distância > 25 cm **e** movimentação > 3 m/s². Dois
`switch` em série fazem o E. É o argumento da aula: essa conta a bordo pediria guardar
as duas medidas, conhecer os dois limites e recompilar a cada ajuste.

Os limiares aparecem em três lugares — regras do n8n, faixas de cor dos widgets do
Node-RED e thresholds do Grafana. São cópias, e nada avisa se uma divergir.

## Armazenamento e visualização

InfluxDB **Cloud**, não o da plataforma. Measurement `nexolog`, `device` como tag e as
sete medições como fields. O token não vai nos arquivos exportados — o Node-RED guarda
token como credencial e a exportação remove.

O fluxo de gravação não filtra nada: leitura nula vira buraco na série, e tratar isso é
parte do trabalho do aluno.

Grafana **local** lendo a Cloud, dois dashboards: gauges e séries; e um **canvas** com a
foto da bag, as medições posicionadas em cima e setas saindo da tampa e da base, que
mudam de cor com o limiar.

## Roteiros de construção

Cinco arquivos `CONSTRUIR-*`, cada um montando do zero em iterações que rodam, com
checklist e tabela de "deu errado":

| Onde | O quê |
|---|---|
| `app14_NexoLog_PUB_only/` | firmware, 4 iterações |
| `app15_NexoLog_PUB_SUB/` | firmware, 3 iterações |
| `app14.../NodeRED/` | dashboard e InfluxDB |
| `app14.../n8n/` | trigger, Switch e Telegram |
| `app15.../NodeRED/` | decisão combinada e LED espelho |
| `app15.../Grafana/` | os dois dashboards |

## Limites conhecidos

- **Sem acknowledgment.** O `ui_led` do dashboard espelha o comando enviado, não o
  estado do dispositivo: ESP32 desligado, o widget acende do mesmo jeito. Faltaria o
  firmware publicar em `.../estado`. Anotado no README do app15.
- **Sem decisão local, fila offline ou reenvio.** Rede caiu, o LED mantém o último
  comando.
- **Telegram e a taxa.** Publicando a 1 s, o aviso sai a cada leitura fora do limite e o
  Telegram bloqueia. Para demonstrar o fluxo, sem credencial.
- **Dashboards exportados do Grafana** trazem a fonte de dados e o measurement da
  instalação de origem (`nexologteste1`), e perderam a variável de bucket. Quem importar
  precisa corrigir os dois.
- **Comportamento com `null` no InfluxDB não testado.** Falta saber se o campo nulo é
  ignorado ou se derruba a linha inteira.

## Validação

Firmware e fluxos conferidos por leitura e por comparação automática entre guia e
código: linhas do `.ino` presentes nos guias, chaves emitidas pelo Node-RED versus
roteadas pelo n8n, fios sem órfãos nos fluxos, links relativos dos Markdown.

**Não houve compilação, deploy nem execução** de Node-RED, n8n, InfluxDB ou Grafana
nesta fase. O que rodou de verdade foi o que o professor testou em bancada — e foi assim
que apareceram dois erros que a leitura não pegou: a opção `Only Message` do MQTT Trigger
embrulhando o item num array, e o `device` indo como field e tag ao mesmo tempo, que o
InfluxDB Cloud recusa com `batch schema conflict`.
