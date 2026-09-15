# App15 — Recebimento de comandos MQTT

Parta de uma cópia do projeto [app14_NexoLog](../app14_CPS_e_Automation/app14_NexoLog_PUB_only). Mantenha sensores, coleta, JSON e tópicos de dados. Acrescente somente a recepção de comandos: a nuvem decide e manda JSON, o dispositivo acende o LED.

Para montar em três iterações que rodam: [CONSTRUIR-O-FIRMWARE.md](app15_NexoLog_PUB_SUB/CONSTRUIR-O-FIRMWARE.md). O roteiro abaixo é o mesmo conteúdo em forma de referência.

## 1. Um LED

Um LED, no GPIO 21. Não há módulo `.hpp` para LED — duas linhas resolvem:

```cpp
const uint8_t LED_ALERTA = 21;
```

No `setup()`, junto dos `inicializar` dos sensores:

```cpp
pinMode(LED_ALERTA, OUTPUT);
digitalWrite(LED_ALERTA, LOW);
```

## 2. Tópico de comandos

Junto às configurações MQTT do `.ino`:

```cpp
#define MQTT_SUB_TOPIC "FIAPIoT/nexolog/equipe01/cmd"
```

## 3. Protótipo

Junto aos demais protótipos:

```cpp
void callbackMQTT(char* topico, byte* conteudo, unsigned int tamanho);
```

## 4. Callback

Acrescente a função ao final do arquivo. O comando chega em JSON, tratado com o mesmo ArduinoJson que monta o payload de subida.

```cpp
void callbackMQTT(char* topico, byte* conteudo, unsigned int tamanho) {
  // %.*s imprime so os primeiros "tamanho" caracteres: conteudo nao termina em \0.
  Serial.printf("[MQTT] Recebido: %.*s\r\n", tamanho, (const char*)conteudo);

  JsonDocument doc;
  DeserializationError erro = deserializeJson(doc, (const char*)conteudo, tamanho);
  if (erro) {
    Serial.printf("[CMD] JSON invalido: %s\n", erro.c_str());
    return;
  }

  const char* alerta = doc["alerta"];
  if (alerta == nullptr) {
    Serial.println("[CMD] Faltou o campo alerta");
    return;
  }

  // A nuvem ja decidiu. Aqui so obedecemos.
  digitalWrite(LED_ALERTA, strcmp(alerta, "ON") == 0 ? HIGH : LOW);
}
```

O comando é `{"alerta":"ON"}` — o dispositivo trata o JSON, extrai o valor e acende ou apaga. Nada além disso: quem decidiu foi a plataforma. `conteudo` não termina em `\0`, por isso o `tamanho` vai junto. `strcmp` porque `alerta` é `const char*`, e ele devolve **0** quando os textos batem. Qualquer valor diferente de `ON` apaga.

## 5. Registrar no setup

Mantenha `mqttClient.setKeepAlive(120);` do app14 e os timeouts padrão das bibliotecas.

Depois de `mqttClient.setServer(MQTT_SERVER, MQTT_PORT);`:

```cpp
mqttClient.setCallback(callbackMQTT);
```

## 6. Assinar após conectar

Em `conectarMQTT()`, dentro do `if (mqttClient.connect(MQTT_CLIENT_ID))`, depois do log de conexão:

```cpp
mqttClient.subscribe(MQTT_SUB_TOPIC);
```

A assinatura acontece novamente em cada reconexão. O `mqttClient.loop()` já está no app14 e entrega as mensagens à callback. Não é necessário mudar a publicação ou acrescentar um campo ao JSON.

## Plataforma

1. Desative o dashboard do app14 antes de ativar o do app15: ambos usam os mesmos tópicos.
2. Importe [Fluxo_1_dashboard_graphs_e_cmd.json](Plataformas_config/NodeRED/Fluxo_1_dashboard_graphs_e_cmd.json) no Node-RED.
   O dashboard tem um widget **LED da caixa**, que acende junto com o LED do dispositivo. É o `node-red-contrib-ui-led` — instale pelo Manage palette, como o `ui-level`.
3. Configure o broker e faça Deploy. Dois switches **em série** fazem o E: `A tampa está aberta?` só entrega ao `E está sacudindo?` o que passou de 25 cm; quem passa dos dois vai ao `Alerta ON`. As saídas `otherwise` dos dois caem no mesmo `Alerta OFF`. É uma condição composta desenhada no canvas, sem escrever código.
4. Para histórico, use [Fluxo_2_envio_InfluxDB.json](Plataformas_config/NodeRED/Fluxo_2_envio_InfluxDB.json). Mantenha apenas uma cópia do fluxo de gravação ativa.
5. Para notificações, importe [fluxo_mqtt.json](Plataformas_config/n8n/fluxo_mqtt.json) no n8n. Ele assina `dados` direto do ESP32, em paralelo com o Node-RED. Configure credenciais MQTT, Telegram e `SEU_CHAT_ID`. Ative apenas um workflow por equipe.
6. Para o histórico em tela, os dois dashboards do [Grafana](Plataformas_config/Grafana/README.md). Grafana local lendo o InfluxDB Cloud: um com gauges e séries, outro em **canvas**, com as medições posicionadas sobre a foto da bag.

O dashboard mostra a condição calculada pela plataforma. A confirmação física são os LEDs ou a Serial; este firmware não publica confirmação de atuação.

Os nós `Simular` entram antes da decisão, então também acionam o comando. Use-os nesta ordem: `Tampa aberta` — nada acontece; `Movimentação brusca` — nada acontece; `Tampa aberta + sacudindo` — o LED acende. É a condição composta em três cliques. Use inicialmente o cenário Normal e depois Tampa aberta. A simulação é interna ao dashboard e não alimenta o fluxo separado de histórico.

## Demonstração

Com a caixa fechada e parada, o LED fica apagado.

Levante a tampa — no Wokwi, mude a distância de 10 para 40 cm. **Nada acontece.** A tampa aberta sozinha não é alerta: uma entrega parada, sendo conferida, tem a tampa aberta.

Agora sacuda o MPU com a tampa ainda aberta. **O LED acende**, no Wokwi e no dashboard. Feche a tampa: apaga. No Serial, `[MQTT] Recebido` mostra o JSON que chegou, fazendo par com o `[MQTT] Publicado` de cada leitura.

O dispositivo não sabe o que é 25 cm, nem 3 m/s², nem que são duas condições. Ele mede, publica e obedece. Para fazer essa conta a bordo, ele precisaria guardar as duas medidas, conhecer os dois limites e ser recompilado a cada ajuste — na plataforma são dois nós ligados um no outro, e o limite muda com um duplo clique.

O LED do dashboard é um **princípio de gêmeo digital**: a tela mostra o estado do equipamento sem ter o equipamento na frente. Mas repare no limite — ele espelha o comando que a plataforma **mandou**, não o que o dispositivo **fez**. Se o ESP32 estiver desligado, o widget acende do mesmo jeito.

### O que falta: confirmação do dispositivo

Para o gêmeo dizer a verdade, faltaria um **acknowledgment**: depois de acender o LED, o ESP32 publicaria em `FIAPIoT/nexolog/equipe01/estado` algo como `{"led":"ON"}`, e o dashboard leria esse tópico em vez do comando. Aí o widget passaria a significar "o LED **está** aceso" em vez de "mandei acender".

**Isso não está implementado nesta versão.** Seria um tópico a mais, um `publish` no fim da callback e um `mqtt in` no dashboard. Fica como exercício — e como a diferença entre ordenar e confirmar, que é onde gêmeo digital deixa de ser enfeite.

Se a comunicação cair, o LED mantém o último comando que recebeu. O ESP32 continua coletando, mas não decide sobre alertas.

Nesta etapa, a plataforma pode representar uma central em nuvem. Para executá-la remotamente, ajuste o endereço do broker e a conectividade. Um serviço executado localmente continua local.

[Montagem e configuração comuns](../app14_CPS_e_Automation/Guia-NexoLog.md)
