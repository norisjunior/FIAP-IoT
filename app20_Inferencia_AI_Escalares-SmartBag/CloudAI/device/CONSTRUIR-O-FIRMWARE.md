# Construir o firmware do app20, do zero

**O app19 media e guardava.** Aqui a bag mede, **pergunta** e obedece. As seis
features são idênticas — mesmo cálculo, mesmas unidades, mesma ordem. Divergir
em qualquer uma faz o modelo errar sem avisar, porque ele foi treinado com os
números do app19.

**Repare no que sai: os dois botões.** No app19 eles existiam porque aquilo era
um **gerador de dataset**, e uma pessoa rotulava cada amostra apertando COLETA.
Aqui quem rotula é o modelo. Sem humano no meio, não há o que apertar — e o
GPIO 27, que era o botão COLETA, vira saída.

Três iterações. Cada uma compila e roda.

1. Publicando as seis features, sem parar.
2. Os dois LEDs, testados no `setup()`.
3. A classe chegando da nuvem e acendendo o LED certo.

---

## Iteração 0 — A base

Copie a pasta `device` do app19 e renomeie o `.ino` para
`app20-SmartBag-Inferencia.ino`. Ele precisa estar rodando as quatro iterações
do guia de firmware do app19 antes de continuar.

**Paridade com o app19** — o que **não pode** mudar:

| O quê | Valor |
|---|---|
| As seis features | `temperatura`, `umidade`, `delta_distancia`, `luz`, `mov_max`, `incl_max` |
| Unidades | °C, %, cm com sinal, RAW 0–4095, m/s², graus |
| `INTERVALO_MPU` | 50 ms, e o pico por `fmaxf` |
| `INTERVALO_DHT` | 2500 ms, com cache da última leitura válida |
| `INTERVALO_COLETA` | 1000 ms |
| Baseline | média de 5 leituras válidas, espaçadas 60 ms |
| Montagem do LDR e do MPU | a mesma; a posição faz parte do modelo |

Esta tabela não é burocracia. O modelo aprendeu que `luz` vai de 0 a 4095 e que
`mov_max` raramente passa de 20. Trocar o LDR de lugar, ou converter para lux,
muda a escala de uma entrada e o modelo continua respondendo — com confiança,
e errado.

**O que sai:**

| O quê | Por quê |
|---|---|
| `BTN_COLETA`, `BTN_SITUACAO` e o debounce | ninguém rotula à mão aqui |
| `rodada`, `situacao`, `SITUACOES[]` | identificação de dataset; o modelo nunca as recebeu |
| `coletando` | o dispositivo publica sempre, não sob comando |
| `ESP32SensorsLED.hpp` | são dois LEDs agora, e o módulo guarda um pino só |

**O que entra:** um segundo LED, o tópico `cmd`, a callback e o prazo de
validade da resposta.

O `ESP32SensorsLED.hpp` sai pela mesma razão que o app15 nunca o teve: dois
`pinMode` e dois `digitalWrite` resolvem, e um módulo com um pino global não
serve para duas saídas.

---

## Iteração 1 — Publicar as seis features

Tire os botões e tudo que dependia deles. O `loop()` fica com três relógios —
DHT, MPU e envio — e mais nada de estado.

**a) O baseline muda de lugar.** No app19 ele era calculado ao iniciar a coleta,
porque havia um botão. Aqui vai no `setup()`, uma vez:

```cpp
  // A bag precisa estar FECHADA e PARADA quando o ESP32 liga.
  calibrarDistancia();
```

É a troca honesta: sem botão, alguém precisa garantir a condição inicial, e esse
alguém agora é quem liga o dispositivo. Se o HC-SR04 não responder, `distBase`
fica `NAN`, `delta_distancia` sai `NAN` — e a iteração 2 vai usar isso.

**b) O payload perde três campos:**

```cpp
  JsonDocument doc;
  doc["device"] = MQTT_CLIENT_ID;
  doc["temperatura"] = ambiente.temp;
  doc["umidade"] = ambiente.umid;
  doc["delta_distancia"] = delta;
  doc["luz"] = luz;
  doc["mov_max"] = movMax;
  doc["incl_max"] = inclMax;
```

Sem `rodada` e sem `situacao`. `situacao` era o **gabarito** — mandá-la aqui
seria entregar a resposta junto com a pergunta.

**c) Feature ausente não vira zero.** Antes de publicar:

```cpp
  if (!isfinite(ambiente.temp) || !isfinite(ambiente.umid) || !isfinite(delta) ||
      !isfinite(movMax) || !isfinite(inclMax)) {
    Serial.println("[SENSOR] Feature ausente. Amostra nao enviada.");
    return;
  }
```

No app19 um `null` ia para o InfluxDB e o Colab descartava a linha depois. Aqui
não há depois: ou existem as seis, ou não há pergunta a fazer. E zero seria o
pior substituto possível — é um valor plausível para qualquer uma das seis, e o
modelo o trataria como medida boa.

Compile e confira no `mosquitto_sub`:

- [ ] Uma mensagem por segundo, com **sete** chaves (as seis e o `device`)
- [ ] Tape o HC-SR04: para de publicar, e o Serial diz por quê
- [ ] O Serial mostra a linha CSV com os mesmos seis números do JSON

---

## Iteração 2 — Os dois LEDs

```cpp
const uint8_t LED_REVISAR = 21;   // era o LED de coleta do app19
const uint8_t LED_OK = 27;        // era o botao COLETA do app19
```

No `setup()`, os dois `pinMode`, e um teste que você apaga depois — ou deixa,
porque ele custa 800 ms no boot e evita horas de depuração:

```cpp
  digitalWrite(LED_OK, HIGH); delay(400); digitalWrite(LED_OK, LOW);
  digitalWrite(LED_REVISAR, HIGH); delay(400); digitalWrite(LED_REVISAR, LOW);
```

Se um não piscar, o problema é o fio — e você descobre isso **antes** de começar
a desconfiar do modelo.

**Três estados, não dois.** Verde, vermelho e **apagado**. O apagado não é
ausência de informação: ele significa "a nuvem não respondeu", e é por isso que
`apagarSaidas()` vem antes de acender qualquer coisa.

- [ ] Os dois piscam na ordem ao ligar
- [ ] Depois do teste, ficam os dois apagados

---

## Iteração 3 — A classe chegando

**a) O tópico e a callback**, como no app15:

```cpp
#define MQTT_SUB_TOPIC "FIAPIoT/smartbag/equipe01/cmd"
```

`mqttClient.setCallback(receberClasse);` no `setup()` e
`mqttClient.subscribe(MQTT_SUB_TOPIC);` dentro do `if` do `conectarMQTT()` — a
assinatura precisa acontecer de novo a cada reconexão.

**b) A callback** apaga tudo e acende uma só:

```cpp
  String classe(conteudo, tamanho);
  classe.trim();
  apagarSaidas();

  if (classe == "ENTREGA_OK") { ... }
  else if (classe == "REVISAR_ENTREGA") { ... }
  else { /* tudo apagado */ }
```

O `String(conteudo, tamanho)` leva o tamanho junto porque o payload MQTT **não
termina em `\0`**. E o Serial imprime o que chegou **antes** de julgar: se o n8n
publicar o JSON inteiro em vez do nome da classe, é ali que se vê.

Nome desconhecido deixa tudo apagado. Acender o verde nesse caso seria tratar
uma falha de comunicação como boa notícia.

**c) A resposta envelhece:**

```cpp
const unsigned long VALIDADE_RESPOSTA = 5000;
```

No `loop()`, passado esse tempo sem classe nova, as duas saídas apagam.

A saída é a **memória** do dispositivo: ela fica como está até chegar algo novo.
Sem prazo, um verde aceso às 14h continuaria aceso às 18h com a rede caída,
dizendo "está tudo bem" sobre uma bag que ninguém mede há quatro horas. Cinco
segundos é folgado para uma resposta que chega a cada um.

**Confira com a API e o n8n no ar:**

- [ ] Bag fechada e parada: LED **verde**
- [ ] Abra a tampa e sacuda: LED **vermelho**
- [ ] Pare o n8n: em 5 s os dois apagam
- [ ] Religue: volta a acender em 1 s
- [ ] Publique `mosquitto_pub -t '.../cmd' -m 'talvez'`: tudo apagado e o Serial explica

O terceiro item é o que distingue este firmware de um que só obedece: ele sabe
quando **não** sabe.

---

## Ordem do arquivo

Configurações → `setup()`/`loop()` → baseline → conexão → publicação →
callback. Sensores continuam em funções nos headers `ESP32Sensors*.hpp`.

## Depois do firmware

A API: [CONSTRUIR-A-API.md](../api/CONSTRUIR-A-API.md). A ponte:
[CONSTRUIR-O-FLUXO.md](../n8n/CONSTRUIR-O-FLUXO.md).
