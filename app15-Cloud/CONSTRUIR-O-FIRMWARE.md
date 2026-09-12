# Construir o firmware do app15

**O app14 não assina nada.** Ele só publica: não tem `setCallback`, não tem
`subscribe`, não tem tópico de comando. O `mqttClient.loop()` que já está lá serve
para manter a conexão viva — é ele que vai entregar as mensagens à callback, mas
sem assinatura nenhuma mensagem chega.

Este firmware é o do app14 mais um caminho de volta. E o que volta é JSON, igual ao
que sobe: a nuvem manda `{"alvo":"tampa","estado":"ON"}`, o dispositivo lê com
ArduinoJson e acende o LED daquele alvo.

Três iterações. Cada uma compila e roda.

1. Dois LEDs, testados no `setup()`.
2. O JSON do comando chegando no Serial.
3. Cada alvo acendendo o seu LED.

---

## Iteração 0 — A base

Copie `app14_NexoLog` inteiro e renomeie. Ele precisa estar rodando as três
iterações de [CONSTRUIR-O-FIRMWARE.md do app14](../app14_CPS_e_Automation/app14_NexoLog/CONSTRUIR-O-FIRMWARE.md)
antes de continuar: publicando JSON a cada 2,5 s.

Nada do que sobe muda. **Paridade com o app14:**

| O quê | Valor |
|---|---|
| Tópico de dados | `FIAPIoT/nexolog/equipe01/dados` |
| Campos do JSON | os mesmos oito, sem acréscimo |
| Intervalo | 2500 ms |
| Client ID | `NexoLogEquipe01`, um por equipe |
| `mqttClient.setKeepAlive(120)` | mantido |

O dashboard do app15 lê o mesmo payload do app14. Mudar um nome de campo aqui
apaga um widget lá.

---

## Iteração 1 — Dois LEDs

O `ESP32SensorsLED.hpp` cuida de um LED só. Aqui são dois, e o foco da aula é o
JSON — então saem o include e a chamada do módulo, e os LEDs ficam no `.ino`.

**a) Tire** do topo do arquivo:

```cpp
#include "ESP32SensorsLED.hpp"
```

**b) Troque** a linha do `LED_PIN` por dois pinos. GPIO 17 estava livre desde que
o ultrassônico mudou de lugar, e fica do mesmo lado da placa:

```cpp
const uint8_t LED_TAMPA = 21;
const uint8_t LED_MOVIMENTO = 17;
```

**c) No `setup()`**, no lugar de `ESP32Sensors::LED::inicializar(LED_PIN);`:

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

No Wokwi, acrescente o segundo LED com resistor de 220 Ω: ânodo no resistor, o
resistor no GPIO 17, cátodo no GND.

**Funcionou?**

- [ ] Os dois piscam juntos ao ligar, meio segundo
- [ ] Depois disso ficam apagados, e a publicação segue normal

| Deu errado | Onde olhar |
|---|---|
| `'LED_PIN' was not declared` | sobrou uma referência ao pino antigo no `setup()` |
| `ESP32Sensors::LED has not been declared` | tirou o include mas deixou a chamada |
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
    Serial.printf("[CMD] JSON invalido: %s\n", erro.c_str());
    return;
  }

  const char* alvo = doc["alvo"];
  const char* estado = doc["estado"];
  if (alvo == nullptr || estado == nullptr) {
    Serial.println("[CMD] Faltou alvo ou estado");
    return;
  }

  Serial.printf("[CMD] %s -> %s\n", alvo, estado);
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
    Serial.printf("[CMD] Alvo desconhecido: %s\n", alvo);
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
[fluxo](Plataformas_config/NodeRED/Fluxo_1_dashboard_graphs_e_cmd.json) é que decidem
e publicam esses JSON. Veja o [README](README.md).
