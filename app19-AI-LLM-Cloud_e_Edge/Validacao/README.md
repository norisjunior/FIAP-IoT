# Validacao — a borda continua concordando com a nuvem?

O dispositivo decide sozinho, com a Random Forest que mora na flash dele. Isso é
ótimo: responde em microssegundos e funciona sem rede. Mas levanta uma pergunta
que não existia enquanto a nuvem decidia tudo:

> **Como eu sei que o modelo pequeno da ponta continua certo?**

Este app responde medindo. O dispositivo publica a janela **junto com a própria
predição**, o n8n faz a mesma pergunta ao modelo maior que roda na nuvem, e
guarda as duas respostas lado a lado.

```text
ESP32 ─► decide na flash ─► acende a saída
   │
   └─► MQTT (janela + predição da borda) ─► n8n ─► API (a rede neural) ─► PostgreSQL
                                                              └► as duas respostas
```

Na indústria isso se chama **shadow mode**: o modelo grande roda em paralelo,
sem mandar em nada, só para medir o quanto o modelo pequeno concorda com ele.

## O dispositivo não obedece a ninguém

Repare no que sumiu do firmware em relação ao app da nuvem: `subscribe`,
`callback`, tópico de comando. O device **já respondeu** quando publica — ele
não espera nada de volta.

A consequência prática aparece quando a rede cai: o motor continua sendo
monitorado e os LEDs continuam certos. **O que para é a conferência, não o
monitoramento.** Na versão em que a nuvem decidia, cair a rede era ficar cego.

## 1) O firmware

```bash
cd device
pio run
```

Ajuste no `.cpp` o Wi-Fi e o `MQTT_SERVER`. Os dois `.hpp` do modelo já estão em
`device/src/` — são os mesmos que você gerou no Colab do app anterior. Se
treinou um modelo novo, troque os dois **juntos**.

Para montar o firmware do zero, em etapas que compilam:
[CONSTRUIR-O-FIRMWARE.md](CONSTRUIR-O-FIRMWARE.md).

| GPIO | Componente | Classe | Índice |
|---|---|---|---|
| 19 | buzzer | `anomalia` | 0 |
| 21 | LED amarelo | `inclinado_frente` | 1 |
| 18 | LED vermelho | `inclinado_tras` | 2 |
| 4 | LED azul | `operando` | 3 |
| 2 | LED onboard | — | aceso = conectado ao broker |

### O tópico é outro

`FIAPIoT/motor/validacao`, e não o `FIAPIoT/motor/multiclasse` das outras
aplicações. Se fosse o mesmo, o fluxo do `Chatbot` reagiria a estas mensagens
também — ele responderia no tópico de comando, e as duas aplicações se
embolariam no mesmo dispositivo.

### O que sai no JSON

```json
{
  "device": "IoTDevValidacaoMotor001",
  "predicao_borda": "operando",
  "mean_ax": -0.124, "mean_ay": -0.014, "mean_az": 0.879,
  "std_ax": 0.248, "std_ay": 0.133, "std_az": 0.225,
  "std_mag": 0.203, "p2p_mag": 0.781
}
```

As oito features vão para a nuvem rodar o modelo dela **nos mesmos números**, e
a `predicao_borda` vai junto para haver o que comparar. Sem ela chegaria uma
predição só, e não existiria conferência nenhuma.

A predição vai como **nome**, não como índice: a API também devolve nome, e
comparar texto com texto dispensa qualquer tabela de tradução no meio.

## 2) O fluxo

Importe `n8n/Fluxo-validacao.json` e configure as credenciais de **MQTT** e
**PostgreSQL**. São quatro nós depois do gatilho:

| # | Nó | O que faz |
|---|---|---|
| 1 | `FIAPIoT/motor/validacao` | recebe a janela e a predição da borda |
| 2 | `Code (gera JSON)` | converte a mensagem MQTT em JSON |
| 3 | `Predict Motor (nuvem)` | `POST` para a API, que ignora os campos a mais |
| 4 | `Compara borda e nuvem` | monta as duas predições e o `concordam` |
| 5 | `Armazena a comparacao` | cria a tabela se não existir e insere |

No nó 4 há um detalhe que vale explicar em voz alta: depois do `POST`, o item
que circula é a **resposta da API** — a predição da borda ficou para trás. Por
isso ela é buscada pelo nome do nó:

```js
const nuvem = $input.first().json.class;
const borda = $('Code (gera JSON)').first().json.predicao_borda;
```

### A tabela

```sql
CREATE TABLE motor_validacao (
  id                SERIAL PRIMARY KEY,
  predicao_borda    TEXT,
  predicao_nuvem    TEXT,
  concordam         BOOLEAN,
  timestamp_medicao TEXT,
  periodo           TEXT,
  created_at        TIMESTAMPTZ DEFAULT NOW()
);
```

O `concordam` é calculado no n8n e gravado pronto. Daria para deixar a conta
para o `SELECT`, mas assim a consulta que interessa fica trivial — e é ela que
você vai projetar na aula.

## 3) A medida que importa

```sql
SELECT
  COUNT(*)                                   AS janelas,
  COUNT(*) FILTER (WHERE concordam)          AS concordaram,
  ROUND(100.0 * COUNT(*) FILTER (WHERE concordam) / COUNT(*), 1) AS taxa_pct
FROM motor_validacao;
```

E, quando discordarem, quais classes se confundiram:

```sql
SELECT predicao_borda, predicao_nuvem, COUNT(*)
FROM motor_validacao
WHERE NOT concordam
GROUP BY 1, 2
ORDER BY 3 DESC;
```

### Quando discordam, quem está errado?

Nenhum dos dois, necessariamente. São **modelos diferentes**: na flash roda uma
floresta de 15 árvores, na nuvem roda uma rede neural. Treinados com o mesmo
dataset, mas aprendendo de jeitos diferentes — é natural que discordem nas
janelas ambíguas, aquelas que caem perto da fronteira entre duas classes.

O número que interessa não é "errou/acertou", é a **taxa de concordância** e,
principalmente, **se ela muda com o tempo**. Uma taxa estável em 95% é um
sistema saudável. A mesma taxa caindo para 60% ao longo de uma semana é o
sinal de que alguma coisa mudou — o motor, a montagem do sensor, ou o mundo.

Se as duas discordarem sempre da mesma forma — por exemplo, a borda dizendo
`operando` onde a nuvem diz `anomalia` —, vale conferir a janela que causou
isso: ela está no Serial Monitor do dispositivo, que imprime as oito features a
cada segundo.

## Se der errado

**`concordam` é sempre `true`, em todas as janelas.** Confira se a API está
servindo a **rede neural** (`modelo_motor_multiclasse.pkl`). Se alguém apontar
o `MODELO_ARQUIVO` para a mesma Random Forest que está na flash, a comparação
vira tautologia: mesmo modelo, mesmos números, mesma resposta, para sempre.

**Nada chega no fluxo.** Confira o tópico: este app usa
`FIAPIoT/motor/validacao`. Um `mosquitto_sub -h localhost -t "FIAPIoT/motor/validacao" -v`
mostra se o dispositivo está publicando.

**A API recusa o corpo.** Ela ignora `device` e `predicao_borda` e lê só as oito
features. Se estiver recusando, é o JSON que chegou quebrado — veja o que o nó 2
produziu.

**Os LEDs acendem mas nada é gravado.** É o comportamento esperado quando o
broker está fora do ar: o dispositivo decide primeiro e publica depois, de
propósito.
