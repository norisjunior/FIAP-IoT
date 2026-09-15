# Construir o firmware do app15

**O app14 não assina nada.** Ele só publica: não tem `setCallback`, não tem
`subscribe`, não tem tópico de comando. O `mqttClient.loop()` que já está lá serve
para manter a conexão viva — é ele que vai entregar as mensagens à callback, mas
sem assinatura nenhuma mensagem chega.

Este firmware é o do app14 mais um caminho de volta. E o que volta é JSON, igual ao
que sobe: a nuvem manda `{"alvo":"tampa","estado":"ON"}`, o dispositivo lê com
ArduinoJson e acende o LED daquele alvo.

O app14 não tem LED — ele só mede e publica, e um LED ali seria enfeite. É aqui que o
LED ganha função, porque agora existe alguém mandando acender.

Três iterações. Cada uma compila e roda.

1. Dois LEDs, testados no `setup()`.
2. O JSON do comando chegando no Serial.
3. Cada alvo acendendo o seu LED.

---

## Iteração 0 — A base

Copie `app14_NexoLog_PUB_only` inteiro e renomeie para `app15_NexoLog_PUB_SUB`. Ele
precisa estar rodando as quatro iterações de
[CONSTRUIR-O-FIRMWARE.md do app14](../../app14_CPS_e_Automation/app14_NexoLog_PUB_only/CONSTRUIR-O-FIRMWARE.md)
antes de continuar: publicando JSON a cada 1 s.

Nada do que sobe muda. **Paridade com o app14:**

| O quê | Valor |
|---|---|
| Tópico de dados | `FIAPIoT/nexolog/equipe01/dados` |
| Campos do JSON | os mesmos oito, sem acréscimo |
| Intervalo de publicação | 1000 ms |
| Saída no Serial | CSV com `\r\n`, título no `setup()` |
| Client ID | `NexoLogEquipe01`, um por equipe |
| `mqttClient.setKeepAlive(120)` | mantido |

O dashboard do app15 lê o mesmo payload do app14. Mudar um nome de campo aqui
apaga um widget lá.

---

## Iteração 1 — Dois LEDs

Dois LEDs, um por causa: um diz "a tampa abriu", o outro diz "sacudiu demais". São
duas linhas de `pinMode` e `digitalWrite` — não vale criar um módulo `.hpp` para isso,
e o foco da aula é o JSON que chega.

**a) Dois pinos**, junto dos outros. GPIO 21 e 17 estão livres e ficam do mesmo lado
da placa:

```cpp
const uint8_t LED_TAMPA = 21;
const uint8_t LED_MOVIMENTO = 17;
```

**b) No `setup()`**, junto dos `inicializar` dos sensores:

```cpp
  pinMode(LED_TAMPA, OUTPUT);
  pinMode(LED_MOVIMENTO, OUTPUT);
  digitalWrite(LED_TAMPA, LOW);
  digitalWrite(LED_MOVIMENTO, LOW);

  // Teste de bancada: pisca os dois uma vez.
  digitalWrite(LED_TAMPA, HIGH);
  digitalWrite(LED_MOVIMENTO, HIGH);
  delay(500);
  digitalWrite(LED_TAMPA, LOW);
  digitalWrite(LED_MOVIMENTO, LOW);
```

No Wokwi, cada LED leva um resistor de 220 Ω: ânodo no resistor, resistor no GPIO
(21 e 17), cátodo no GND. O `diagram.json` deste projeto já vem com os dois.

**Funcionou?**

- [ ] Os dois piscam juntos ao ligar, meio segundo
- [ ] Depois disso ficam apagados, e a publicação segue normal

| Deu errado | Onde olhar |
|---|---|
| `'LED_TAMPA' was not declared` | os dois `const uint8_t` têm que vir antes do `setup()` |
| `ESP32SensorsLED.hpp: No such file` | o módulo de LED não existe mais: apague o include herdado do app14 |
| Um pisca, o outro não | LED invertido: perna longa (ânodo) é a do resistor |

Tire o teste de bancada depois de conferir, ou deixe — ele é um bom sinal de que
a placa reiniciou.

---

## Iteração 2 — Receber o JSON

**a) O tópico**, junto das outras configurações MQTT:

```cpp
#define MQTT_SUB_TOPIC "FIAPIoT/nexolog/equipe01/cmd"
```

Tópico diferente do de dados: o ESP32 publica em `dados` e escuta em `cmd`. Assinar
o próprio tópico de publicação faz o dispositivo receber o que ele mesmo mandou.

**b) O protótipo**, junto dos demais:

```cpp
void callbackMQTT(char* topico, byte* conteudo, unsigned int tamanho);
```

**c) A função**, no fim do arquivo. `ArduinoJson` já está no `platformio.ini` — é o
mesmo que monta o JSON que sobe:

```cpp
void callbackMQTT(char* topico, byte* conteudo, unsigned int tamanho) {
  JsonDocument doc;
  DeserializationError erro = deserializeJson(doc, (const char*)conteudo, tamanho);
  if (erro) {
    Serial.printf("[CMD] JSON invalido: %s\r\n", erro.c_str());
    return;
  }

  const char* alvo = doc["alvo"];
  const char* estado = doc["estado"];
  if (alvo == nullptr || estado == nullptr) {
    Serial.println("[CMD] Faltou alvo ou estado");
    return;
  }

  Serial.printf("[CMD] %s -> %s\r\n", alvo, estado);
}
```

Três coisas para reparar:

- `conteudo` não termina em `\0`. Por isso o `tamanho` vai junto — sem ele o parser
  lê além da mensagem.
- `deserializeJson` devolve erro em vez de travar. JSON quebrado no meio da aula é
  comum; a callback avisa e volta.
- `doc["alvo"]` num campo ausente devolve `nullptr`, não string vazia. Por isso a
  checagem antes de usar.

**d) Registrar**, no `setup()`, depois do `setServer`:

```cpp
  mqttClient.setCallback(callbackMQTT);
```

**e) Assinar**, em `conectarMQTT()`, dentro do `if (mqttClient.connect(...))`:

```cpp
    mqttClient.subscribe(MQTT_SUB_TOPIC);
```

Dentro do `if`, não fora: a assinatura vale por conexão e precisa acontecer de novo
a cada reconexão.

**Funcionou?** Com o firmware rodando:

```
mosquitto_pub -h localhost -t 'FIAPIoT/nexolog/equipe01/cmd' -m '{"alvo":"tampa","estado":"ON"}'
mosquitto_pub -h localhost -t 'FIAPIoT/nexolog/equipe01/cmd' -m 'ON'
```

```
[CMD] tampa -> ON
[CMD] JSON invalido: InvalidInput
```

- [ ] O JSON bom aparece com alvo e estado
- [ ] O `ON` solto é recusado sem travar o firmware
- [ ] Continua publicando os dados normalmente entre um comando e outro

| Deu errado | Onde olhar |
|---|---|
| Nada chega | o `subscribe` ficou fora do `if` do `connect`, ou o tópico diverge |
| Nada chega e parou de publicar | a callback precisa ser rápida: nada de `delay()` ou `while` dentro dela |
| Chega o próprio JSON do sensor | você assinou `dados` em vez de `cmd` |
| `NoMemory` | mensagem maior que o buffer: `mqttClient.setBufferSize(768)` no `setup()` |

---

## Iteração 3 — Cada alvo no seu LED

Só o fim da callback muda. Depois do `Serial.printf`:

```cpp
  bool ligar = strcmp(estado, "ON") == 0;

  if (strcmp(alvo, "tampa") == 0) {
    digitalWrite(LED_TAMPA, ligar ? HIGH : LOW);
  } else if (strcmp(alvo, "movimento") == 0) {
    digitalWrite(LED_MOVIMENTO, ligar ? HIGH : LOW);
  } else {
    Serial.printf("[CMD] Alvo desconhecido: %s\r\n", alvo);
  }
```

`strcmp` porque `alvo` é `const char*`, não `String`: `alvo == "tampa"` compara
endereços e dá sempre falso.

Qualquer `estado` que não seja exatamente `ON` apaga o LED. É uma escolha: em
dúvida, o painel fica apagado em vez de mentir que está tudo bem.

**Funcionou?**

```
mosquitto_pub -h localhost -t 'FIAPIoT/nexolog/equipe01/cmd' -m '{"alvo":"tampa","estado":"ON"}'
mosquitto_pub -h localhost -t 'FIAPIoT/nexolog/equipe01/cmd' -m '{"alvo":"movimento","estado":"ON"}'
mosquitto_pub -h localhost -t 'FIAPIoT/nexolog/equipe01/cmd' -m '{"alvo":"tampa","estado":"OFF"}'
```

- [ ] Acende o da tampa, depois o de movimento, depois só o de movimento fica aceso
- [ ] Os dois são independentes: um comando não mexe no outro LED
- [ ] `{"alvo":"buzina","estado":"ON"}` só reclama no Serial

| Deu errado | Onde olhar |
|---|---|
| Serial mostra o comando, LED parado | `pinMode` no `setup()`, ou pino trocado |
| Um comando apaga o outro LED | `else if`, não dois `if` com o mesmo `digitalWrite` |
| Sempre apaga, mesmo com `ON` | `strcmp` devolve **0** quando é igual — repare no `== 0` |

O firmware não publica confirmação: quem confere é o LED ou a Serial. Este é o
firmware final do app15 — o app16 roda o mesmo, só muda onde a plataforma está.

Agora a outra ponta: os dois switches do
[fluxo](../Plataformas_config/NodeRED/Fluxo_1_dashboard_graphs_e_cmd.json) é que decidem
e publicam esses JSON. Veja o [README](../README.md).
