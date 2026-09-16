# Construir o fluxo do app20 no n8n, do zero

Duas iterações. Cada uma roda.

1. A amostra da bag chegando no n8n e virando classe.
2. A classe voltando ao ESP32 e acendendo o LED.

O n8n aqui é uma **ponte**, não um decisor. Ele não sabe o que é `mov_max` nem
qual limiar importa — pega o que chegou por MQTT, entrega à API, e devolve a
resposta por MQTT. Quem decide é a floresta.

Antes de começar: a API do app20 respondendo em `http://localhost:8000/docs`
([CONSTRUIR-A-API.md](../api/CONSTRUIR-A-API.md)) e o firmware publicando
([CONSTRUIR-O-FIRMWARE.md](../device/CONSTRUIR-O-FIRMWARE.md)).

`http://localhost:5678`

---

## Iteração 1 — Da bag até a classe

**a) `MQTT Trigger`.**

| Campo | Valor |
|---|---|
| Credential | Host `mosquitto` · Port `1883` · sem usuário/senha |
| Topics | `FIAPIoT/smartbag/equipe01/dados` |

Clique em **Listen for test event**. Com o Wokwi rodando, chega um item por
segundo:

```json
{ "topic": "FIAPIoT/smartbag/equipe01/dados",
  "message": "{\"device\":\"SmartBagEquipe01\",\"temperatura\":24.3,...}" }
```

Repare: o payload vem como **texto** dentro de `message`, com as aspas
escapadas. É por isso que existe o próximo nó.

**b) Um nó `Code`.** Mode: **Run Once for Each Item**.

```javascript
const dados = typeof $json.message === "string"
    ? JSON.parse($json.message)
    : $json.message;

return { json: dados };
```

O `typeof` está ali porque o MQTT Trigger entrega string em uma configuração e
objeto em outra. Com o teste, o nó funciona nas duas.

**c) Um nó `HTTP Request`.**

| Campo | Valor |
|---|---|
| Method | `POST` |
| URL | `http://host.docker.internal:8000/predict` |
| Send Body | ligado |
| Body Content Type | `JSON` |
| Specify Body | `Using JSON` |
| JSON | `{{ $json }}` |

**`host.docker.internal` e não `localhost`.** O n8n está dentro de um contêiner;
para ele, `localhost` é ele mesmo, e a API está na máquina hospedeira. Se você
roda os dois nativamente, aí sim é `localhost`.

O `{{ $json }}` manda o objeto inteiro, com `device` junto. A API ignora o que
não é feature — não precisamos filtrar aqui.

Execute o workflow.

**Funcionou?** O nó HTTP verde, com:

```json
{ "class": "ENTREGA_OK", "code": 0,
  "probabilities": { "ENTREGA_OK": 0.95, "REVISAR_ENTREGA": 0.05 } }
```

- [ ] Chega um item por segundo do MQTT Trigger
- [ ] O `Code` devolve um objeto com as seis features, sem aspas escapadas
- [ ] O HTTP devolve `class`, `code` e `probabilities`

| Deu errado | Onde olhar |
|---|---|
| Nada chega | o ESP32 publica só com as seis features boas: veja o Serial |
| `ECONNREFUSED` | a API não está no ar, ou a porta é outra |
| `ECONNREFUSED` com a API no ar | `localhost` no lugar de `host.docker.internal` |
| `422` no HTTP | o `Code` não converteu: o corpo foi como texto, não objeto |
| `JSON.parse` estourou | o payload não era JSON — confira o que o ESP32 publicou |

---

## Iteração 2 — A classe de volta

**Um nó `MQTT`**, ligado na saída do HTTP.

| Campo | Valor |
|---|---|
| Credential | a mesma do Trigger |
| Topic | `FIAPIoT/smartbag/equipe01/cmd` |
| **Send Input Data** | **desligado** |
| Message | `{{ $json.class }}` |

**Send Input Data desligado é o ponto desta iteração.** Ligado, o n8n publica o
JSON inteiro da resposta; o firmware compara o payload com `"ENTREGA_OK"`, não
casa, e deixa tudo apagado — parecendo erro do modelo quando é formato de
mensagem. Desligado, vai só o texto da classe.

Mandamos `class` e não `code`. O firmware compara nomes porque, no Serial, ler
`REVISAR_ENTREGA` diz mais que ler `1`. O `code` continua na resposta HTTP para
comparar com o app21, que trabalha com o inteiro.

**Deploy** e ative o workflow.

- [ ] Bag fechada e parada: LED **verde** no Wokwi
- [ ] Abra a tampa e sacuda: LED **vermelho**
- [ ] Desative o workflow: em 5 s os dois LEDs apagam
- [ ] Reative: volta a acender em 1 s

| Deu errado | Onde olhar |
|---|---|
| Os dois LEDs apagados, fluxo verde | Send Input Data ligado: o Serial mostra o JSON que chegou |
| LED não muda | o tópico `cmd` do nó e do firmware precisam ser iguais, `equipe01` inclusive |
| Acende e apaga toda hora | o fluxo está publicando mais devagar que 1 a cada 5 s |
| Sempre a mesma classe | é o modelo, não o fluxo: confira no `/docs` da API com os mesmos números |

---

## Por que a ponte existe

O ESP32 poderia chamar a API direto por HTTP. Não é o que fazemos, por dois
motivos.

O primeiro é de projeto: o dispositivo já fala MQTT com a plataforma, e uma
segunda pilha de rede no firmware é mais código para manter e mais coisa para
falhar. Publicar continua sendo publicar.

O segundo é de tempo. O `HTTPClient` do ESP32 é **síncrono**: a chamada trava o
`loop()` até a resposta chegar ou estourar o timeout. Como o MPU é amostrado a
cada 50 ms, uma API lenta furaria a janela de medição — e o modelo receberia
picos calculados sobre menos amostras que as do treino. Com MQTT, publicar é
rápido e a resposta chega quando chegar, pela callback.

A ponte fica com a espera, o dispositivo fica com o relógio.
