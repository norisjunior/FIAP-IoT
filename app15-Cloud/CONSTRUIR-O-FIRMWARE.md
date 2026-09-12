# Construir o firmware do app15

**O app14 não assina nada.** Ele só publica: não tem `setCallback`, não tem
`subscribe`, não tem tópico de comando. O `mqttClient.loop()` que já está lá serve
para manter a conexão viva — é ele que vai entregar as mensagens à callback, mas
sem assinatura nenhuma mensagem chega.

Então este firmware é o do app14 mais um caminho de volta.

Duas iterações. Cada uma compila e roda.

1. O comando chegando no Serial.
2. O comando acendendo o LED.

---

## Iteração 0 — A base

Copie `app14_NexoLog` inteiro e renomeie. Ele precisa estar rodando as três
iterações de [CONSTRUIR-O-FIRMWARE.md do app14](../app14_CPS_e_Automation/app14_NexoLog/CONSTRUIR-O-FIRMWARE.md)
antes de continuar: publicando JSON a cada 2,5 s.

Nada do que está lá muda. **Paridade com o app14:**

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

## Iteração 1 — Receber o comando no Serial

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

**c) A função**, no fim do arquivo. `conteudo` não termina em `\0` — por isso o
`String` recebe o tamanho junto:

```cpp
void callbackMQTT(char* topico, byte* conteudo, unsigned int tamanho) {
  String mensagem(conteudo, tamanho);
  Serial.println("Comando recebido: " + mensagem);
}
```

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
mosquitto_pub -h localhost -t 'FIAPIoT/nexolog/equipe01/cmd' -m 'ON'
```

```
Comando recebido: ON
```

- [ ] Chega no Serial em menos de um segundo
- [ ] Continua publicando os dados normalmente entre um comando e outro

| Deu errado | Onde olhar |
|---|---|
| Nada chega | o `subscribe` ficou fora do `if` do `connect`, ou o tópico diverge |
| Nada chega e parou de publicar | a callback precisa ser rápida: nada de `delay()` ou `while` dentro dela |
| Chega o próprio JSON do sensor | você assinou `dados` em vez de `cmd` |
| `Comando recebido: ONlixo` | faltou o `tamanho` no construtor do `String` |

---

## Iteração 2 — Acender o LED

Só o corpo da callback muda:

```cpp
void callbackMQTT(char* topico, byte* conteudo, unsigned int tamanho) {
  String mensagem(conteudo, tamanho);
  Serial.println("Comando recebido: " + mensagem);

  if (mensagem == "ON") {
    ESP32Sensors::LED::on();
  } else if (mensagem == "OFF") {
    ESP32Sensors::LED::off();
  }
}
```

`ON` e `OFF` em maiúsculas, sem aspas nem quebra de linha no conteúdo MQTT.
Qualquer outro texto não mexe no LED.

**Funcionou?**

```
mosquitto_pub -h localhost -t 'FIAPIoT/nexolog/equipe01/cmd' -m 'ON'
mosquitto_pub -h localhost -t 'FIAPIoT/nexolog/equipe01/cmd' -m 'OFF'
```

- [ ] Acende e apaga
- [ ] `-m 'on'` minúsculo não faz nada — é o esperado

| Deu errado | Onde olhar |
|---|---|
| Serial mostra o comando, LED parado | `LED_PIN` e `ESP32Sensors::LED::inicializar()` no `setup()` |
| Acende e nunca apaga | `-n` no `mosquitto_pub`, ou o `else if` virou `if (mensagem == "ON")` duas vezes |

O firmware não publica confirmação: quem confere é o LED ou a Serial. Este é o
firmware final do app15 — o app16 roda o mesmo, só muda onde a plataforma está.

Agora a outra ponta: [o fluxo que decide e manda o comando](Plataformas_config/NodeRED/Fluxo_1_dashboard_graphs_e_cmd.json),
descrito no [README](README.md).
