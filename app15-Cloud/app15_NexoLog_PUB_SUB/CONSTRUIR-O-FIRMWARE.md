# Construir o firmware do app15

**O app14 não assina nada** — não tem `setCallback`, `subscribe` nem tópico de comando,
e por isso também não tem LED. Aqui entra o caminho de volta: a nuvem manda
`{"alerta":"ON"}` e o dispositivo acende.

**Repare em quem decide.** O alerta só dispara com a tampa aberta **e** a caixa
sacudindo. Para o ESP32 fazer essa conta, precisaria guardar as duas medidas, conhecer
os dois limites e ser recompilado a cada ajuste. Na plataforma são dois nós ligados um
no outro, e o dispositivo recebe a conclusão pronta.

Três iterações. Cada uma compila e roda.

1. Um LED, testado no `setup()`.
2. O JSON do comando chegando no Serial.
3. O alerta acendendo o LED.

---

## Iteração 0 — A base

Copie `app14_NexoLog_PUB_only` inteiro e renomeie para `app15_NexoLog_PUB_SUB`. Ele
precisa estar rodando as quatro iterações do `CONSTRUIR-O-FIRMWARE.md` do app14
antes de continuar: publicando JSON a cada 1 s.

Nada do que sobe muda. **Paridade com o app14:**

| O quê | Valor |
|---|---|
| Tópico de dados | `FIAPIoT/nexolog/equipe01/dados` |
| Campos do JSON | os mesmos oito, sem acréscimo |
| Intervalo de publicação | 1000 ms |
| Saída no Serial | CSV com `\r\n`, título no `setup()` |
| Reconexão | dentro de `conectarWiFi()` e `conectarMQTT()`, uma tentativa a cada 5 s |
| Estilo | todo `if` com chaves, mesmo de uma linha |
| Client ID | `NexoLogEquipe01`, um por equipe |
| `mqttClient.setKeepAlive(120)` | mantido |

O dashboard do app15 lê o mesmo payload do app14. Mudar um nome de campo aqui
apaga um widget lá.

---

## Iteração 1 — Um LED

Um LED só, para uma decisão só. Não vale um módulo `.hpp` para duas linhas.

**a) Um pino**, junto dos outros:

```cpp
const uint8_t LED_ALERTA = 21;
```

**b) No `setup()`**, junto dos `inicializar` dos sensores — estas duas linhas ficam:

```cpp
  pinMode(LED_ALERTA, OUTPUT);
  digitalWrite(LED_ALERTA, LOW);
```

**c) Para conferir a ligação**, acrescente logo abaixo — e apague depois. O arquivo
pronto não tem este trecho:

```cpp
  digitalWrite(LED_ALERTA, HIGH);
  delay(500);
  digitalWrite(LED_ALERTA, LOW);
```

No Wokwi o LED leva um resistor de 220 Ω: ânodo no resistor, resistor no GPIO 21,
cátodo no GND. O `diagram.json` deste projeto já vem com ele.

**Funcionou?**

- [ ] Pisca uma vez ao ligar, meio segundo
- [ ] Depois disso fica apagado, e a publicação segue normal

| Deu errado | Onde olhar |
|---|---|
| `'LED_ALERTA' was not declared` | o `const uint8_t` tem que vir antes do `setup()` |
| `ESP32SensorsLED.hpp: No such file` | o módulo de LED não existe mais: apague o include herdado do app14 |
| LED aceso o tempo todo | faltou o `digitalWrite(LED_ALERTA, LOW)` depois do `delay` |

---

## Iteração 2 — Receber o JSON

**a) O tópico**, junto das outras configurações MQTT:

```cpp
#define MQTT_SUB_TOPIC "FIAPIoT/nexolog/equipe01/cmd"
```

Tópico diferente do de dados: publica em `dados`, escuta em `cmd`. Assinar o próprio
tópico de publicação faz o dispositivo receber o que ele mesmo mandou.

**b) O protótipo**, junto dos demais:

```cpp
void callbackMQTT(char* topico, byte* conteudo, unsigned int tamanho);
```

**c) A função**, no fim do arquivo. `ArduinoJson` já está no `platformio.ini` — é o
mesmo que monta o JSON que sobe:

```cpp
void callbackMQTT(char* topico, byte* conteudo, unsigned int tamanho) {
  // %.*s imprime so os primeiros "tamanho" caracteres: conteudo nao termina em \0.
  Serial.printf("[MQTT] Recebido: %.*s\r\n", tamanho, (const char*)conteudo);

  JsonDocument doc;
  DeserializationError erro = deserializeJson(doc, (const char*)conteudo, tamanho);
  if (erro) {
    Serial.printf("[CMD] JSON invalido: %s\r\n", erro.c_str());
    return;
  }

  const char* alerta = doc["alerta"];
  if (alerta == nullptr) {
    Serial.println("[CMD] Faltou o campo alerta");
    return;
  }

  // A acao entra na iteracao 3.
}
```

Três coisas para reparar:

- `conteudo` não termina em `\0`. Por isso o `tamanho` vai junto — sem ele o parser
  lê além da mensagem. É também por isso que o print usa `%.*s`, que recebe o tamanho
  antes do texto: um `%s` comum sairia lendo memória até achar um zero por acaso.
- `deserializeJson` devolve erro em vez de travar. JSON quebrado no meio da aula é
  comum; a callback avisa e volta.
- `doc["alerta"]` num campo ausente devolve `nullptr`, não string vazia. Por isso a
  checagem antes de usar.

O `[MQTT] Recebido` faz par com o `[MQTT] Publicado` do envio: um diz o que saiu, o
outro diz o que chegou. Juntos, o Serial conta a conversa inteira.

**d) Registrar**, no `setup()`, depois do `setServer`:

```cpp
  mqttClient.setCallback(callbackMQTT);
```

**e) Assinar**, em `conectarMQTT()`, dentro do `if (mqttClient.connect(...))` — depois
do controle de próxima tentativa, que já está no começo da função:

```cpp
    mqttClient.subscribe(MQTT_SUB_TOPIC);
```

Dentro do `if`, não fora: a assinatura vale por conexão e precisa acontecer de novo
a cada reconexão.

**Funcionou?** Com o firmware rodando:

```
mosquitto_pub -h localhost -t 'FIAPIoT/nexolog/equipe01/cmd' -m '{"alerta":"ON"}'
mosquitto_pub -h localhost -t 'FIAPIoT/nexolog/equipe01/cmd' -m 'ON'
```

```
[MQTT] Recebido: {"alerta":"ON"}
[MQTT] Recebido: ON
[CMD] JSON invalido: InvalidInput
```

- [ ] O JSON bom aparece inteiro, entre chaves
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

Uma linha no fim da callback:

```cpp
  // A nuvem ja decidiu. Aqui so obedecemos.
  digitalWrite(LED_ALERTA, strcmp(alerta, "ON") == 0 ? HIGH : LOW);
```

`strcmp` porque `alerta` é `const char*`, não `String`: `alerta == "ON"` compara
endereços e dá sempre falso. E `strcmp` devolve **0** quando os textos são iguais — daí
o `== 0`.

Qualquer `alerta` que não seja exatamente `ON` apaga. É uma escolha: em dúvida, o painel
fica apagado em vez de mentir que está tudo bem.

Repare no que **não** está na callback: nenhum número, nenhum limite, nenhum `if` sobre
distância ou movimentação. O firmware não sabe o que é 25 cm nem 3 m/s², e nem sabe que
são duas condições. Ele recebe a conclusão e obedece.

**Funcionou?**

```
mosquitto_pub -h localhost -t 'FIAPIoT/nexolog/equipe01/cmd' -m '{"alerta":"ON"}'
mosquitto_pub -h localhost -t 'FIAPIoT/nexolog/equipe01/cmd' -m '{"alerta":"OFF"}'
```

- [ ] Acende e apaga, e o `[MQTT] Recebido` mostra o JSON de cada comando
- [ ] `{"alerta":"on"}` minúsculo não acende
- [ ] `{}` sem o campo só reclama no Serial

| Deu errado | Onde olhar |
|---|---|
| `[MQTT] Recebido` aparece, LED parado | `pinMode` no `setup()`, ou pino trocado |
| Sempre apaga, mesmo com `ON` | `strcmp` devolve **0** quando é igual — repare no `== 0` |
| Acende e apaga sozinho o tempo todo | normal: a plataforma reavalia a cada leitura |

O firmware não publica confirmação: quem confere é o LED ou a Serial. Este é o
firmware final do app15 — o app16 roda o mesmo, só muda onde a plataforma está.

Agora a outra ponta: os dois switches do
[fluxo](../Plataformas_config/NodeRED/Fluxo_1_dashboard_graphs_e_cmd.json) é que decidem
e publicam esses JSON. Veja o [README](../README.md).
