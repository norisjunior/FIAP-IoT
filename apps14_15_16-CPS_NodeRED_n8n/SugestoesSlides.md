# Ajustes nos slides

Referência: o projeto em `FIAP-IoT-eval/app14_CPS_e_Automation`, `app15-Cloud` e
`app16-Edge`. Os PowerPoints permanecem intactos — este arquivo só descreve o que mudar.

Atualizado em 15/09/2026.

## O que mudou desde a versão anterior deste arquivo

Confira antes de reaproveitar qualquer texto antigo:

| Antes | Agora |
|---|---|
| `inclinacao`, em graus | `movimentacao`, em m/s² — **pico** do último segundo |
| Publicação a cada 2,5 s | 1 s, com um relógio por sensor |
| DHT 26 · TRIG/ECHO 17/16 · MPU 18/19 · LED 27 | DHT 4 · TRIG/ECHO 19/18 · MPU 22/23 · LED 21 |
| Node-RED decide e publica em `/eventos` | Node-RED mostra; **n8n** assina `/dados` e decide |
| Aviso só na mudança de estado | Aviso a cada leitura fora do limite |
| Comando `ON`/`OFF` em texto, dois LEDs | `{"alerta":"ON"}`, **um** LED no GPIO 21 |
| Alerta por qualquer limiar | No app15, tampa aberta **E** movimentação alta |
| Serial com rótulos | **CSV** com `\r\n` e linha de título |
| InfluxDB da plataforma | InfluxDB **Cloud** |
| Sem Grafana | Dois dashboards, um deles **canvas** sobre a foto da bag |
| `app14_NexoLog` · `app15-Cloud` | `app14_NexoLog_PUB_only` · `app15_NexoLog_PUB_SUB` |

Material novo que pode virar aula: cinco roteiros `CONSTRUIR-*` — firmware do app14 e do
app15, fluxo do Node-RED de cada um, n8n e Grafana. Cada um monta do zero, em iterações
que rodam.

---

## Aula 07 — Node-RED

Arquivo: `IoT - Aula 07 - Node-RED.pptx`, **34 slides**.

Cinco alterações, sem acrescentar slides — mais a **seção nova de InfluxDB**, que
acrescenta três (ver `PromptSlides-Aula07-InfluxDB.md`).

### 1. Slide 4 — Objetivo

Trocar o quarto marcador, "Criar alertas inteligentes baseados em condições", por:

- Guardar o histórico da entrega num banco de série temporal.

O alerta saiu do Node-RED: quem decide e avisa é o n8n, na Aula 08.

### 2. Slide 9 — Arquitetura de um CPS para IoT

Camada Física: "DHT22, HC-SR04, MPU6050/MPU6500 (FastIMU)". Sem LED — ele entra no
app15.

Camada de Decisão: trocar "Regras e alertas automáticos" por "Regras e alertas — no
n8n, a partir do mesmo tópico MQTT".

### 3. Slide 21 — Aplicação IoT White Label: NexoLog

O slide tem só o título. Acrescentar a caixa de carga com os sensores identificados,
feita com formas:

- DHT22: temperatura e umidade dentro da caixa.
- HC-SR04: distância até a tampa.
- MPU6050/MPU6500: movimentação da caixa, em m/s².

Pergunta: "A caixa chegou nas mesmas condições em que saiu?"

Nota de fala: white label é o ponto. A mesma caixa vira bag de entregador, e aí "tampa
aberta" é indício de fraude. Só muda o texto do aviso.

### 4. Slide 24 — Aplicação: app14

O payload de hoje:

```json
{"device":"NexoLogEquipe01","temp":24,"umid":55,"dist":10,
 "accel_x":0,"accel_y":0,"accel_z":9.81,"movimentacao":0.03}
```

E o que sai no monitor serial, que é um CSV:

```
temp,umid,dist,movimentacao
24.0,55.0,10.0,0.03
```

Roteiro: MQTT-in, JSON, Debug, Function e Gauge. Abrir `separarDadosSensores` e
observar que cada saída leva uma mensagem com um valor em `payload`.

Nota de fala: `movimentacao` é o **maior** valor do último segundo, não a leitura do
instante. O firmware amostra o MPU a cada 50 ms porque uma leitura por envio deixa o
pico do sacolejo passar — sacudir a caixa e ver `0.00` é o sintoma.

### 5. Slide 27 — Dashboard da entrega

Substituir a captura por uma do dashboard atual: temperatura, umidade, tampa
(`ui_level` vertical), movimentação e o gráfico.

Ao lado, só a pergunta: "O que muda quando levantamos a tampa?"

Nota de fala: demonstrar no Wokwi mudando a distância de 10 para 40 cm. Não há mais nós
`Simular` no app14 — o dado vem do dispositivo. O gráfico guarda cinco minutos e zera no
F5: é o gancho para os slides de InfluxDB que vêm a seguir.

---

## Aula 08 — Node-RED e n8n

Cinco alterações, sem acrescentar slides.

### 1. Slide 4 — Objetivo

"Avisar o responsável quando a entrega sair das condições combinadas."

- Assinar o mesmo tópico do dispositivo, no n8n.
- Decidir com um nó Switch, sem escrever código.
- Enviar o aviso ao Telegram.

### 2. Slide 6 — Automação com n8n

Substituir o texto informal e a afirmação de que n8n local não permite webhooks por:

"O Node-RED mostra o que está acontecendo. O n8n decide se aquilo é um problema e
comunica o responsável. Os dois assinam o mesmo tópico e nenhum sabe do outro."

Nota de fala: esse é o argumento do MQTT numa tela só — publicar uma vez, entregar a
quantos assinarem. Acrescentar o banco não exigiu recompilar o firmware.

### 3. Slide 11 — Avisos da entrega

Substituir a imagem de alertas de sala/PIR por dois exemplos reais:

```text
NexoLog | NexoLogEquipe01
Tampa aberta
Distância: 40.0 cm
```

```text
NexoLog | NexoLogEquipe01
Movimentação brusca
Movimentação: 4.87 m/s²
```

Nota de fala: o limite não vai na mensagem. O aviso conta o que aconteceu e com que
número; a regra é assunto de quem decide. Caixa em condição normal não gera mensagem.

### 4. Slide 12 — Workflow da entrega

Captura do fluxo de `Plataformas_config/n8n/fluxo_mqtt.json`. Sete nós, **nenhum de
código**:

`MQTT Trigger` → `Passou de algum limiar?` → quatro `Edit Fields` → um `Telegram`.

Texto: "Tópico de entrada: `FIAPIoT/nexolog/equipe01/dados` — o mesmo do Node-RED".

Nota de fala: dois detalhes do Switch decidem o comportamento. **Send data to all
matching outputs**, senão a caixa quente *e* aberta avisa só a temperatura. E **sem
Fallback Output**, porque quando nada passa do limite nada deve sair. Um nó de Telegram
só, uma credencial só, e ainda assim uma mensagem por variável — o que muda é o caminho
até ele.

### 5. Slide 14 — Hands on: a mesma caixa nas próximas etapas

1. Caixa normal: dashboard vivo, Telegram em silêncio.
2. Levantar a tampa: chega o aviso da tampa.
3. Aquecer com a tampa aberta: chegam **dois** avisos, um por variável.
4. Fechar e esfriar: para sozinho.

Fechamento oral: "No app15 a plataforma manda o comando de volta, em JSON, e acende o
LED. No app16 essa mesma plataforma fica no Raspberry Pi. O que continua funcionando
quando a internet cai, mas a rede local permanece?"

Aviso para a demonstração: o dispositivo publica a cada 1 s, e o aviso sai a cada
leitura fora do limite. O Telegram bloqueia nesse ritmo. Para demonstrar o fluxo, deixe
a credencial de fora e acompanhe pelas execuções.

---

## Aula do app15 — o caminho de volta

Não existe deck próprio mapeado aqui. Se for acrescentar, estes são os quatro pontos que
o projeto hoje oferece, em ordem de impacto:

### 1. A nuvem faz a conta que o dispositivo não faria

O alerta só dispara com a tampa aberta **e** a caixa sacudindo. No Node-RED são dois
nós `switch` em série — o segundo só recebe quem passou pelo primeiro. Para o ESP32
fazer isso sozinho, precisaria guardar as duas medidas, conhecer os dois limites e ser
recompilado a cada ajuste.

Demonstração em três cliques: `Tampa aberta` — nada; `Movimentação brusca` — nada;
`Tampa aberta + sacudindo` — o LED acende.

### 2. O comando é JSON, e o dispositivo só obedece

`{"alerta":"ON"}` chega no tópico `cmd`. A callback faz `deserializeJson`, extrai o
campo e acende. Mostrar o que **não** está nela: nenhum número, nenhum limite, nenhuma
noção de que são duas condições.

Ponto técnico que rende: `conteudo` não termina em `\0`, por isso o `tamanho` vai junto
no parse e o print usa `%.*s`.

### 3. Gêmeo digital, e o seu limite

O dashboard tem um `ui_led` que acende junto com o LED da bancada. Perguntar à turma:
"se eu desligar o ESP32 agora e disparar o alerta, o que acontece com o widget?"

Ele acende do mesmo jeito — porque espelha o comando **enviado**, não o estado do
dispositivo. Falta um acknowledgment do ESP32 num tópico `estado`. Está anotado como não
implementado no README do app15.

### 4. Grafana: o mesmo dado, outra pergunta

Dois dashboards, Grafana local lendo InfluxDB Cloud. O primeiro é o de sempre; o
segundo é um **canvas** com a foto da bag, as medições posicionadas em cima e setas que
saem da tampa e da base, mudando de cor com o limiar.

Vale como fechamento visual da trilha: o mesmo `dados` que virou gauge no Node-RED,
aviso no Telegram e linha no banco, agora vira uma bag na tela que fica vermelha.
