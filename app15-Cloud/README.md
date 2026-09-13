# App15 — Recebimento de comandos MQTT

Parta de uma cópia do projeto [app14_NexoLog](../app14_CPS_e_Automation/app14_NexoLog). Mantenha sensores, coleta, JSON e tópicos de dados. Acrescente somente a recepção de comandos: a nuvem decide e manda JSON, o dispositivo acende o LED do alvo.

Para montar em três iterações que rodam: [CONSTRUIR-O-FIRMWARE.md](CONSTRUIR-O-FIRMWARE.md). O roteiro abaixo é o mesmo conteúdo em forma de referência.

## 1. Dois LEDs

Um LED por alvo. O `ESP32SensorsLED.hpp` cuida de um só, então aqui os pinos ficam no `.ino`: tire o include e troque `LED_PIN` por dois.

```cpp
const uint8_t LED_TAMPA = 21;
const uint8_t LED_MOVIMENTO = 17;
```

No `setup()`, no lugar de `ESP32Sensors::LED::inicializar(LED_PIN);`:

```cpp
pinMode(LED_TAMPA, OUTPUT);
pinMode(LED_MOVIMENTO, OUTPUT);
digitalWrite(LED_TAMPA, LOW);
digitalWrite(LED_MOVIMENTO, LOW);
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
  bool ligar = strcmp(estado, "ON") == 0;

  if (strcmp(alvo, "tampa") == 0) {
    digitalWrite(LED_TAMPA, ligar ? HIGH : LOW);
  } else if (strcmp(alvo, "movimento") == 0) {
    digitalWrite(LED_MOVIMENTO, ligar ? HIGH : LOW);
  } else {
    Serial.printf("[CMD] Alvo desconhecido: %s\n", alvo);
  }
}
```

O comando é `{"alvo":"tampa","estado":"ON"}`, um alvo por mensagem. `conteudo` não termina em `\0`, por isso o `tamanho` vai junto. `strcmp` porque `alvo` é `const char*`: `alvo == "tampa"` compara endereços. Qualquer `estado` diferente de `ON` apaga.

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
3. Configure o broker e faça Deploy. Dois switches decidem, um por LED: `Tampa: dist > 25 cm` e `Movimentação > 3 m/s²`. Cada um tem saída 1 (passou do limite) e saída 2 (`otherwise`), e cada saída vai a um nó Change que monta o JSON do comando. A decisão fica visível no canvas, sem código.
4. Para histórico, use [Fluxo_2_envio_InfluxDB.json](Plataformas_config/NodeRED/Fluxo_2_envio_InfluxDB.json). Mantenha apenas uma cópia do fluxo de gravação ativa.
5. Para notificações, importe [fluxo_mqtt.json](Plataformas_config/n8n/fluxo_mqtt.json) no n8n. Configure credenciais MQTT, Telegram e `SEU_CHAT_ID`. Ative apenas um workflow de eventos por equipe.

O dashboard mostra a condição calculada pela plataforma. A confirmação física são os LEDs ou a Serial; este firmware não publica confirmação de atuação.

Os nós `Simular` alimentam o dashboard e o evento do n8n, mas não os comandos: quem publica em `cmd` são os dois switches, ligados às medidas reais. Use inicialmente o cenário Normal e depois Tampa aberta. A simulação é interna ao dashboard e não alimenta o fluxo separado de histórico.

## Demonstração

Com a caixa fechada e parada, os dois LEDs ficam apagados.

Levante a tampa — no Wokwi, mude a distância de 10 para 40 cm. A plataforma publica `{"alvo":"tampa","estado":"ON"}` e só o LED da tampa acende. Volte para 10 cm e ele apaga.

Agora arraste o MPU com a tampa aberta: acende também o LED de movimento, por um comando separado. Os dois são independentes — é o painel da bag mostrando *o que* está errado, não só *que* algo está errado.

O dispositivo não sabe o que é 25 cm. Ele mede, publica e obedece; o limite mora na nuvem e muda sem recompilar o firmware.

Se a comunicação cair, cada LED mantém o último comando que recebeu. O ESP32 continua coletando, mas não decide sobre alertas.

Nesta etapa, a plataforma pode representar uma central em nuvem. Para executá-la remotamente, ajuste o endereço do broker e a conectividade. Um serviço executado localmente continua local.

[Montagem e configuração comuns](../app14_CPS_e_Automation/Guia-NexoLog.md)
