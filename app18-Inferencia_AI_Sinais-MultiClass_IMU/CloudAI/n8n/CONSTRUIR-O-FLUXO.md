# Construir o fluxo n8n

Sete nós: receber as features, consultar a API, devolver a classe, avisar
quando houver anomalia e avisar quando a própria predição falhar.

```text
MQTT Trigger → Code → Predict Motor ┬(ok)──→ MQTT /cmd
                                    ├(ok)──→ IF: E anomalia? ─true→ Telegram
                                    └(erro)→ Telegram: a API não respondeu
```

## 0 — Antes de começar

- Inicie a IoT-platform e abra `http://localhost:5678`.
- Inicie a API conforme [CONSTRUIR-A-API.md](../api/CONSTRUIR-A-API.md).
- Deixe o ESP32 preparado para publicar as features.
- Tenha as credenciais MQTT e Telegram da aula e o Chat ID do destinatário.

Crie um workflow vazio. Para usar a versão pronta, importe
[Fluxo-n8n-predict.json](Fluxo-n8n-predict.json) e selecione suas credenciais nos nós MQTT e Telegram. Ajuste o Chat ID para o seu chat.

## 1 — MQTT Trigger: receber a janela

Adicione **MQTT Trigger** e configure:

| Campo | Valor |
|---|---|
| Nome do nó | `FIAPIoT/motor/multiclasse` |
| Credential | Sua credencial MQTT da IoT-platform |
| Topics | `FIAPIoT/motor/multiclasse` |

O payload chega no campo `message`.

Se precisar criar a credencial MQTT no n8n da IoT-platform, use Host
`mqtt-broker`, Port `1883` e SSL desligado. Use a mesma credencial nos dois nós MQTT.

## 2 — Code: transformar a mensagem em JSON

Conecte um nó **Code** ao MQTT Trigger.

- Nome: `Code (gera JSON)`.
- Language: **JavaScript**.
- Mode: **Run Once for Each Item**.

Cole:

```javascript
const dados = typeof $json.message === "string"
    ? JSON.parse($json.message)
    : $json.message;

return { json: dados };
```

Se a mensagem for texto, `JSON.parse()` transforma em objeto.
A API valida as oito features e ignora campos extras, como `device`.

## 3 — HTTP Request: consultar o modelo

Conecte um **HTTP Request** ao Code:

| Campo | Valor |
|---|---|
| Nome do nó | `Predict Motor` |
| Method | `POST` |
| URL | `http://host.docker.internal:8000/predict` |
| Authentication | `None` |
| Send Body | Ligado |
| Body Content Type | `JSON` |
| Specify Body | `Using JSON` |
| JSON — modo Expression | `{{ $json }}` |

Essa URL considera n8n no Docker Desktop e API no Windows. Com ambos rodando
diretamente no mesmo computador, use `http://localhost:8000/predict`.

Ainda neste nó, abra a aba **Settings** e ligue duas coisas:

| Campo | Valor |
|---|---|
| Retry On Fail | ligado — `Max Tries` 3, `Wait Between Tries` 1000 ms |
| On Error | **Continue (using error output)** |

O `Retry` absorve o soluço: se a API demorar um instante, o nó tenta de novo em
vez de falhar. O `On Error` faz aparecer uma **segunda saída, vermelha**, por
onde sai o item quando as três tentativas falham. É nela que a etapa 7 se
conecta.

Sem isso, uma API fora do ar derruba a execução inteira e ninguém fica sabendo.

Confira na saída: `class` contém a classe e `probabilities` contém as probabilidades.

## 4 — MQTT: devolver a classe ao ESP32

Conecte um nó **MQTT** diretamente ao `Predict Motor`:

| Campo | Valor |
|---|---|
| Nome do nó | `FIAPIoT/motor/multiclasse/cmd` |
| Credential | Sua credencial MQTT da IoT-platform |
| Topic | `FIAPIoT/motor/multiclasse/cmd` |
| Send Input Data | **Desligado** |
| Message — modo Expression | `{{ $json.class }}` |

O ESP32 precisa receber somente o texto, como `operando`, sem aspas e sem o JSON inteiro.

## 5 — IF: verificar se é anomalia

Crie uma **segunda conexão saindo do Predict Motor**, agora para um nó **IF**.

| Campo | Valor |
|---|---|
| Nome do nó | `E anomalia?` |
| Valor esquerdo — modo Expression | `{{ $json.class }}` |
| Tipo e operação | `String` → `is equal to` |
| Valor direito — modo Fixed | `anomalia` |

Deixe a saída **false** sem conexão. O MQTT do passo 4 recebe todas as classes,
independentemente do resultado desse IF.

## 6 — Telegram: enviar o alerta

Conecte a saída **true** do IF a um nó **Telegram**:

| Campo | Valor |
|---|---|
| Nome do nó | `ANOMALIA: alerta no Telegram` |
| Credential | Sua credencial Telegram |
| Resource | `Message` |
| Operation | `Send Message` |
| Chat ID | ID do seu chat |
| Text — modo Fixed | Texto abaixo |

```text
🚨 ANOMALIA no motor!
O modelo identificou uma anomalia. Verifique o motor.
```

Abra a conversa com seu bot no Telegram e envie `/start` antes do teste.

**Cada predição `anomalia` gera um alerta.** Enquanto a anomalia continuar,
as mensagens se repetem: cerca de uma por segundo com o firmware da aula.

### Provocar uma anomalia sem o motor

No Wokwi você **não consegue** produzir anomalia: ela é vibração, e o controle
de aceleração parado deixa as 100 amostras da janela idênticas, com `std_*` e
`p2p_mag` em zero. No motor de verdade, desbalancear a hélice no meio da aula
também não é prático.

O caminho é publicar a janela direto no tópico, fingindo ser o ESP32:

```bash
mosquitto_pub -h localhost -t "FIAPIoT/motor/multiclasse" -m '{"device":"TesteManual","mean_ax":-0.02,"mean_ay":0.00,"mean_az":1.00,"std_ax":0.450,"std_ay":0.300,"std_az":0.400,"std_mag":0.420,"p2p_mag":1.60}'
```

O fluxo não sabe a diferença: chega uma mensagem no tópico, ele classifica. A
predição sai `anomalia`, o alerta cai no Telegram e — se o ESP32 estiver
ligado — o **buzzer dele toca**, porque a classe volta pelo tópico de comando.

Para comparar, a mesma janela com vibração normal, que dá `operando`:

```bash
mosquitto_pub -h localhost -t "FIAPIoT/motor/multiclasse" -m '{"device":"TesteManual","mean_ax":-0.02,"mean_ay":0.00,"mean_az":1.00,"std_ax":0.030,"std_ay":0.028,"std_az":0.032,"std_mag":0.035,"p2p_mag":0.16}'
```

As duas mensagens têm a **mesma orientação** — os três `mean_*` são idênticos.
Só a vibração muda, e é só ela que separa as duas classes.

> No PowerShell, troque as aspas simples por duplas e escape as internas, ou
> rode as duas linhas no Git Bash / WSL, onde elas funcionam como estão.

## 7 — Telegram: avisar que a predição falhou

Conecte um **segundo nó Telegram** à **saída vermelha** do `Predict Motor`:

| Campo | Valor |
|---|---|
| Nome do nó | `FALHA: a API nao respondeu` |
| Credential | a mesma credencial Telegram |
| Chat ID | o mesmo chat |
| Text — modo Expression | o texto abaixo |

```text
⚠️ A predição falhou.
O motor continua medindo, mas ninguém está classificando.

Erro: {{ $json.error?.message || $json.error || 'sem detalhe' }}
```

**Por que este ramo existe.** Sem ele, a API fora do ar não avisa ninguém: o
ESP32 continua publicando, e a falha só aparece para quem abrir a lista de
execuções do n8n.

**E há um detalhe pior, que vale explicar em voz alta.** A saída do dispositivo
é a memória da última resposta recebida. Com a nuvem fora do ar ela não apaga —
o LED **fica aceso na classe velha**, e quem olhar para a bancada vê um sistema
que parece funcionando. É exatamente o tipo de falha que o alerta precisa cobrir.

> **Uma mensagem por janela enquanto durar.** O `Retry On Fail` da etapa 3
> absorve a falha passageira; uma queda longa manda um aviso por segundo. É a
> mesma escolha do alerta de anomalia: repetir é melhor do que calar.

### Provocar a falha sem derrubar a API

Dá para parar o `uvicorn`, e é o teste mais realista. Mas há um jeito de ver o
ramo de erro com a API no ar: mandar uma janela com `NaN` numa das features.

```bash
mosquitto_pub -h localhost -t "FIAPIoT/motor/multiclasse" -m '{"device":"TesteManual","mean_ax":-0.02,"mean_ay":0.00,"mean_az":1.00,"std_ax":0.030,"std_ay":0.028,"std_az":0.032,"std_mag":"NaN","p2p_mag":0.16}'
```

O caminho da mensagem é o seguinte: o Pydantic **aceita** a string `"NaN"` num
campo `float` e a converte em `nan`; o `StandardScaler` propaga; e o
scikit-learn levanta `ValueError: Input X contains NaN`. A API devolve **500**,
o nó `Predict Motor` falha depois das três tentativas e o item sai pela saída
vermelha, direto no Telegram.

**Repare nas aspas em volta do `NaN`.** Elas são o detalhe que faz o teste
funcionar:

| O que você publica | Onde quebra |
|---|---|
| `"std_mag": "NaN"` | na **API**, com 500 — é o ramo de erro que queremos |
| `"std_mag": nan` | no nó **Code**, antes: `nan` sem aspas não é JSON válido, e o `JSON.parse()` estoura |
| `"std_mag": null` | na **API**, com 422 — o Pydantic recusa o campo |

Os três avisam alguém, mas por caminhos diferentes. Só o primeiro exercita o
ramo que acabamos de montar; o segundo mostra que o `JSON.parse()` do nó Code
já é uma barreira, e é por isso que não existe um nó de validação antes dele.

## 8 — Testar

1. Confira a API em `http://localhost:8000/`.
2. No n8n, clique em **Execute workflow** para aguardar uma mensagem MQTT.
3. Ligue o ESP32 e confira as features na saída do Code.
4. Confira a classe na saída do `Predict Motor` e a saída correspondente no ESP32.
5. Repita o teste provocando a condição de anomalia usada no treino: o IF deve
   seguir por **true** e enviar o alerta.
6. **Teste a falha:** pare a API (`Ctrl+C` no terminal do uvicorn) e deixe o
   ESP32 publicando. Depois de três tentativas o item sai pela saída vermelha e
   o aviso chega no Telegram. Repare que o LED do ESP32 **não apaga** — fica na
   última classe recebida. Suba a API de novo e o ciclo volta sozinho.
7. Salve e ative/publique o workflow para receber continuamente. Mantenha apenas
   uma cópia ativa, para evitar respostas e alertas duplicados.

| Problema | Confira |
|---|---|
| MQTT não recebe | Broker, credencial, tópico e ESP32 conectado |
| API retorna `422` | As oito features estão no JSON, com nomes corretos e valores numéricos? |
| HTTP não conecta | API iniciada em `0.0.0.0:8000` e URL correta |
| ESP32 mostra classe desconhecida | **Send Input Data** desligado e Message com `{{ $json.class }}` |
| Telegram não chega | Saída **true**, credencial, Chat ID e `/start` |

Referência dos campos: [HTTP Request](https://docs.n8n.io/integrations/builtin/core-nodes/n8n-nodes-base.httprequest/) e [IF](https://docs.n8n.io/integrations/builtin/core-nodes/n8n-nodes-base.if/).
