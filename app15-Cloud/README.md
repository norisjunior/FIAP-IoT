# App15 — Recebimento de comandos MQTT

Parta de uma cópia do projeto [app14_NexoLog](../app14_CPS_e_Automation/app14_NexoLog). Mantenha sensores, coleta, JSON e tópicos de dados. Acrescente somente a recepção de comandos para o LED.

Para montar em duas iterações que rodam: [CONSTRUIR-O-FIRMWARE.md](CONSTRUIR-O-FIRMWARE.md). O roteiro abaixo é o mesmo conteúdo em forma de referência.

## 1. Tópico de comandos

Junto às configurações MQTT do `.ino`:

```cpp
#define MQTT_SUB_TOPIC "FIAPIoT/nexolog/equipe01/cmd"
```

## 2. Protótipo

Junto aos demais protótipos:

```cpp
void callbackMQTT(char* topico, byte* conteudo, unsigned int tamanho);
```

## 3. Callback

Acrescente a função ao final do arquivo. A construção de `String` segue o exemplo do app08: usa o conteúdo e seu tamanho, sem depender de um terminador nulo.

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

Os comandos são textos `ON` e `OFF`, em maiúsculas, sem aspas ou quebras de linha no conteúdo MQTT. Outros conteúdos não alteram o LED.

## 4. Registrar no setup

Mantenha `mqttClient.setKeepAlive(120);` do app14 e os timeouts padrão das bibliotecas.

Depois de `mqttClient.setServer(MQTT_SERVER, MQTT_PORT);`:

```cpp
mqttClient.setCallback(callbackMQTT);
```

## 5. Assinar após conectar

Em `conectarMQTT()`, dentro do `if (mqttClient.connect(MQTT_CLIENT_ID))`, depois do log de conexão:

```cpp
mqttClient.subscribe(MQTT_SUB_TOPIC);
```

A assinatura acontece novamente em cada reconexão. O `mqttClient.loop()` já está no app14 e entrega as mensagens à callback. Não é necessário mudar a publicação ou acrescentar um campo ao JSON.

## Plataforma

1. Desative o dashboard do app14 antes de ativar o do app15: ambos usam os mesmos tópicos.
2. Importe [Fluxo_1_dashboard_graphs_e_cmd.json](Plataformas_config/NodeRED/Fluxo_1_dashboard_graphs_e_cmd.json) no Node-RED.
3. Configure o broker e faça Deploy. O nó `Tampa: dist > 25 cm` é quem decide: saída 1 envia `ON`, saída 2 (`otherwise`) envia `OFF`. A decisão fica visível no canvas, sem código.
4. Para histórico, use [Fluxo_2_envio_InfluxDB.json](Plataformas_config/NodeRED/Fluxo_2_envio_InfluxDB.json). Mantenha apenas uma cópia do fluxo de gravação ativa.
5. Para notificações, importe [fluxo_mqtt.json](Plataformas_config/n8n/fluxo_mqtt.json) no n8n. Configure credenciais MQTT, Telegram e `SEU_CHAT_ID`. Ative apenas um workflow de eventos por equipe.

O dashboard mostra a condição calculada pela plataforma. A confirmação física é o próprio LED ou a Serial; este firmware não publica confirmação de atuação.

Os nós `Simular` alimentam o dashboard e o evento do n8n, mas não o comando: quem manda `ON`/`OFF` é o switch, ligado à distância real. Use inicialmente o cenário Normal e depois Tampa aberta. A simulação é interna ao dashboard e não alimenta o fluxo separado de histórico.

## Demonstração

Com a caixa fechada, o LED fica apagado. Levante a tampa — no Wokwi, mude a distância de 10 para 40 cm: a plataforma envia `ON`. Volte para 10 cm: envia `OFF`.

O dispositivo não sabe o que é 25 cm. Ele mede, publica e obedece; o limite mora na nuvem e muda sem recompilar o firmware.

Se a comunicação cair, o LED mantém o último comando. O ESP32 continua coletando, mas não decide sobre alertas.

Nesta etapa, a plataforma pode representar uma central em nuvem. Para executá-la remotamente, ajuste o endereço do broker e a conectividade. Um serviço executado localmente continua local.

[Montagem e configuração comuns](../app14_CPS_e_Automation/Guia-NexoLog.md)
