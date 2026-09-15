# Construir o firmware do app15

**O app14 não assina nada.** Ele só publica: não tem `setCallback`, não tem
`subscribe`, não tem tópico de comando. O `mqttClient.loop()` que já está lá serve
para manter a conexão viva — é ele que vai entregar as mensagens à callback, mas
sem assinatura nenhuma mensagem chega.

Este firmware é o do app14 mais um caminho de volta. E o que volta é JSON, igual ao
que sobe: a nuvem manda `{"alerta":"ON","motivo":"Tampa aberta com movimentacao"}`, e
o dispositivo acende o LED e conta no Serial por quê.

O app14 não tem LED — ele só mede e publica, e um LED ali seria enfeite. É aqui que o
LED ganha função, porque agora existe alguém mandando acender.

**Repare em quem decide.** O alerta só dispara quando a tampa está aberta **e** a caixa
está sacudindo. Para o ESP32 fazer essa conta sozinho, ele precisaria guardar as duas
medidas, conhecer os dois limites e ser recompilado a cada ajuste. Na plataforma, são
dois nós ligados um no outro. O dispositivo recebe a conclusão pronta.

Três iterações. Cada uma compila e roda.

1. Um LED, testado no `setup()`.
2. O JSON do comando chegando no Serial.
3. O alerta acendendo o LED.

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

## Iteração 1 — Um LED

Um LED só, para uma decisão só. Duas linhas de `pinMode` e `digitalWrite` — não vale
criar um módulo `.hpp` para isso, e o foco da aula é o JSON que chega.

**a) Um pino**, junto dos outros:

```cpp
const uint8_t LED_ALERTA = 21;
```

**b) No `setup()`**, junto dos `inicializar` dos sensores:

```cpp
  pinMode(LED_ALERTA, OUTPUT);
  digitalWrite(LED_ALERTA, LOW);

  // Teste de bancada: pisca uma vez.
  digitalWrite(LED_ALERTA, HIGH);
  delay(500);
  digitalWrite(LED_ALERTA, LOW);
```

No Wokwi o LED leva um resistor de 220 Ω: ânodo no resistor, resistor no GPIO 21,
cátodo no GND. O `diagram.json` deste projeto já vem com ele.

**Funcionou?**

- [ ] Pisca uma vez ao ligar, meio segundo
- [ ] Depois disso ficam apagados, e a publicação segue normal

| Deu errado | Onde olhar |
|---|---|
| `'LED_ALERTA' was not declared` | o `const uint8_t` tem que vir antes do `setup()` |
| `ESP32SensorsLED.hpp: No such file` | o módulo de LED não existe mais: apague o include herdado do app14 |

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

  const char* alerta = doc["alerta"];
  const char* motivo = doc["motivo"];
  if (alerta == nullptr) {
    Serial.println("[CMD] Faltou o campo alerta");
    return;
  }

  Serial.printf("[CMD] %s: %s\r\n", alerta, motivo ? motivo : "sem motivo");
}
```

Três coisas para reparar:

- `conteudo` não termina em `\0`. Por isso o `tamanho` vai junto — sem ele o parser
  lê além da mensagem.
- `deserializeJson` devolve erro em vez de travar. JSON quebrado no meio da aula é
  comum; a callback avisa e volta.
- `doc["alerta"]` num campo ausente devolve `nullptr`, não string vazia. Por isso a
  checagem antes de usar. O `motivo` pode faltar sem problema — o `?:` cobre.

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
mosquitto_pub -h localhost -t 'FIAPIoT/nexolog/equipe01/cmd' -m '{"alerta":"ON","motivo":"teste"}'
mosquitto_pub -h localhost -t 'FIAPIoT/nexolog/equipe01/cmd' -m 'ON'
```

```
[CMD] ON: teste
[CMD] JSON invalido: InvalidInput
```

- [ ] O JSON bom aparece com o alerta e o motivo
- [ ] O `ON` solto é recusado sem travar o firmware
- [ ] Continua publicando os dados normalmente entre um comando e outro

| Deu errado | Onde olhar |
|---|---|
| Nada chega | o `subscribe` ficou fora do `if` do `connect`, ou o tópico diverge |
| Nada chega e parou de publicar | a callback precisa ser rápida: nada de `delay()` ou `while` dentro dela |
| Chega o próprio JSON do sensor | você assinou `dados` em vez de `cmd` |
| `NoMemory` | mensagem maior que o buffer: `mqttClient.setBufferSize(768)` no `setup()` |

---

## Iteração 3 — O alerta acendendo o LED

Duas linhas no fim da callback, depois do `Serial.printf`:

```cpp
  bool ligar = strcmp(alerta, "ON") == 0;
  digitalWrite(LED_ALERTA, ligar ? HIGH : LOW);
```

`strcmp` porque `alerta` é `const char*`, não `String`: `alerta == "ON"` compara
endereços e dá sempre falso. E `strcmp` devolve **0** quando os textos são iguais — daí
o `== 0`.

Qualquer `alerta` que não seja exatamente `ON` apaga. É uma escolha: em dúvida, o painel
fica apagado em vez de mentir que está tudo bem.

Repare no que **não** está aqui: nenhum número, nenhum limite, nenhum `if` sobre
distância ou movimentação. O firmware não sabe o que é 25 cm nem 3 m/s², e nem sabe que
são duas condições. Ele recebe a conclusão e obedece.

**Funcionou?**

```
mosquitto_pub -h localhost -t 'FIAPIoT/nexolog/equipe01/cmd' -m '{"alerta":"ON","motivo":"Tampa aberta com movimentacao"}'
mosquitto_pub -h localhost -t 'FIAPIoT/nexolog/equipe01/cmd' -m '{"alerta":"OFF","motivo":"Sem alerta"}'
```

- [ ] Acende e apaga, e o Serial conta o motivo de cada um
- [ ] `{"alerta":"on"}` minúsculo não acende — é o esperado
- [ ] `{"motivo":"teste"}` sem o `alerta` só reclama no Serial

| Deu errado | Onde olhar |
|---|---|
| Serial mostra o comando, LED parado | `pinMode` no `setup()`, ou pino trocado |
| Sempre apaga, mesmo com `ON` | `strcmp` devolve **0** quando é igual — repare no `== 0` |
| Acende e apaga sozinho o tempo todo | normal: a plataforma reavalia a cada leitura |

O firmware não publica confirmação: quem confere é o LED ou a Serial. Este é o
firmware final do app15 — o app16 roda o mesmo, só muda onde a plataforma está.

Agora a outra ponta: os dois switches do
[fluxo](../Plataformas_config/NodeRED/Fluxo_1_dashboard_graphs_e_cmd.json) é que decidem
e publicam esses JSON. Veja o [README](../README.md).
