# Construir o fluxo n8n do app25

Seis nós: receber as features, consultar a API, devolver a classe e avisar quando houver anomalia.

```text
MQTT Trigger → Code → Predict Motor
                           ├→ MQTT /cmd
                           └→ IF: E anomalia?
                                  └ true → Telegram
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

## 7 — Testar

1. Confira a API em `http://localhost:8000/`.
2. No n8n, clique em **Execute workflow** para aguardar uma mensagem MQTT.
3. Ligue o ESP32 e confira as features na saída do Code.
4. Confira a classe na saída do `Predict Motor` e a saída correspondente no ESP32.
5. Repita o teste provocando a condição de anomalia usada no treino: o IF deve
   seguir por **true** e enviar o alerta.
6. Salve e ative/publique o workflow para receber continuamente. Mantenha apenas
   uma cópia ativa, para evitar respostas e alertas duplicados.

| Problema | Confira |
|---|---|
| MQTT não recebe | Broker, credencial, tópico e ESP32 conectado |
| API retorna `422` | As oito features estão no JSON, com nomes corretos e valores numéricos? |
| HTTP não conecta | API iniciada em `0.0.0.0:8000` e URL correta |
| ESP32 mostra classe desconhecida | **Send Input Data** desligado e Message com `{{ $json.class }}` |
| Telegram não chega | Saída **true**, credencial, Chat ID e `/start` |

Referência dos campos: [HTTP Request](https://docs.n8n.io/integrations/builtin/core-nodes/n8n-nodes-base.httprequest/) e [IF](https://docs.n8n.io/integrations/builtin/core-nodes/n8n-nodes-base.if/).
