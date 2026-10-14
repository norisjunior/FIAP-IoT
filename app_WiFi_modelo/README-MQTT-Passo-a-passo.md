# MQTT com ESP32 — Passo a passo

Use `src/iot-app-wifi-exemplo.ino` como base. WiFi, sensor e JSON já estão prontos; os TODOs indicam onde acrescentar MQTT. O publicador envia e o assinante recebe pelo callback. A assinatura é uma chamada explícita, acrescentada somente no assinante.

## 1. Biblioteca

O `platformio.ini` já declara PubSubClient. A versão do padrão é:

```ini
knolleary/PubSubClient @ 2.8
```

Mantenha as dependências já usadas pelo modelo, ArduinoJson e EasyUltrasonic. O monitor é 115200.

## 2. Include

```cpp
#include <PubSubClient.h>
```

## 3. Configuração

No publicador:

```cpp
const char* MQTT_BROKER = "host.wokwi.internal";
const uint16_t MQTT_PORTA = 1883;
const char* MQTT_CLIENT_ID = "a12345678-modelopub-01";
const char* MQTT_PUB_TOPIC = "FIAP/sala/distancia";

PubSubClient mqttClient(wifiClient);
uint32_t ultimaTentativaMQTT = 0;
```

No assinante, substitua a declaração de Client ID e acrescente o tópico recebido:

```cpp
const char* MQTT_CLIENT_ID = "a12345678-modelosub-01";
const char* MQTT_SUB_TOPIC = "FIAP/sala/distancia";
```

Troque o RA de exemplo pelo seu. Cada aluno/dispositivo precisa de um Client ID distinto. A MAC padrão das simulações Wokwi não é suficiente para distinguir todos os devices. O tópico do assinante deve ser igual ao do publicador.

No Wokwi, `host.wokwi.internal` acessa o broker local pelo gateway. No kit, informe o IP do computador do laboratório (por exemplo `172.16.10.101`). Porta 1883, sem autenticação ou TLS neste modelo.

`INTERVALO_RECONEXAO` e `ultimaTentativaWiFi` já estão no código-base. O intervalo é `uint32_t` e vale 5000 ms. Não redeclare essas variáveis.

## 4. Conectar ao MQTT

A função fica antes do setup, na seção de funções. No publicador:

```cpp
void conectarMQTT() {
  if (millis() - ultimaTentativaMQTT < INTERVALO_RECONEXAO) {
    return;
  }
  ultimaTentativaMQTT = millis();

  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println("[MQTT] Conectado");
  } else {
    Serial.printf("[MQTT] Falha: %d\r\n", mqttClient.state());
  }
}
```

No assinante, acrescentamos `subscribe` dentro do `if` que confirma a conexão:

```cpp
void conectarMQTT() {
  if (millis() - ultimaTentativaMQTT < INTERVALO_RECONEXAO) {
    return;
  }
  ultimaTentativaMQTT = millis();

  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println("[MQTT] Conectado");
    mqttClient.subscribe(MQTT_SUB_TOPIC);
  } else {
    Serial.printf("[MQTT] Falha: %d\r\n", mqttClient.state());
  }
}
```

Assim, toda reconexão renova a assinatura. Não há chave ou flag para selecionar assinatura.

## 5. Setup

Depois da chamada WiFi, configure o cliente:

```cpp
mqttClient.setServer(MQTT_BROKER, MQTT_PORTA);
mqttClient.setKeepAlive(120);
mqttClient.setSocketTimeout(2);
```

A chamada WiFi mantém o controle por intervalo: a primeira tentativa acontece quando passam os primeiros 5 s. Quando uma tentativa é iniciada, `conectarWiFi()` aguarda a conexão e mostra a mensagem e o IP dentro da própria função. O canal é automático; somente SSID e senha são passados ao `WiFi.begin`.

## 6. Publicar

Dentro do bloco de coleta, depois de serializar o JSON no `buffer`:

```cpp
mqttClient.publish(MQTT_PUB_TOPIC, buffer);
```

O conteúdo e o ritmo de coleta do modelo permanecem os mesmos.

## 7. Registrar o callback

Somente no assinante, acrescente ao setup:

```cpp
mqttClient.setCallback(callback);
```

A assinatura do tópico já está dentro de `conectarMQTT()`. O callback precisa estar declarado antes do setup ou ter um protótipo ali.

## 8. Callback

```cpp
void callback(char* topico, byte* conteudo, unsigned int tamanho) {
  String mensagem(conteudo, tamanho);
  Serial.println("Recebido: " + mensagem);

  JsonDocument doc;
  deserializeJson(doc, mensagem);
  float dist = doc["dist"];
  Serial.printf("Distância recebida: %.2f cm\n", dist);

  // Acrescente aqui a ação da aplicação.
}
```

O formato dos parâmetros é o exigido pela biblioteca. Não é preciso criar outra abstração para tratar o recebimento.

## 9. Loop

Substitua o bloco WiFi existente e o TODO do loop pela sequência completa do app14:

```cpp
if (WiFi.status() != WL_CONNECTED) {
  conectarWiFi();
} else if (!mqttClient.connected()) {
  conectarMQTT();
} else {
  mqttClient.loop();
}
```

Depois desse bloco, mantenha o bloco de coleta/JSON do modelo. `mqttClient.loop()` processa as mensagens e mantém a sessão.

## Esperas deste exemplo

WiFi usa `while` e `delay(500)` para aguardar a conexão iniciada, conforme o padrão didático escolhido. Só imprime a mensagem quando conectar e mostra o IP obtido; uma reconexão mostra o IP atual. Enquanto aguarda, o restante do loop fica pausado.

MQTT faz uma tentativa por intervalo de 5 s, mas `PubSubClient::connect()` é síncrono. `setSocketTimeout(2)` limita a resposta MQTT; não limita toda a conexão TCP. Não há armazenamento/reenvio offline.

## Referência rápida

| Função | Uso |
|---|---|
| `setServer` | Broker e porta |
| `connect` | Conectar com Client ID |
| `subscribe` | Assinar explicitamente após conectar |
| `setCallback` | Registrar quem trata as mensagens |
| `publish` | Enviar o payload |
| `loop` | Processar mensagens e manter a sessão |

APIs conferidas: [PubSubClient 2.8](https://github.com/knolleary/pubsubclient/blob/v2.8/src/PubSubClient.h), [implementação da conexão](https://github.com/knolleary/pubsubclient/blob/v2.8/src/PubSubClient.cpp), [WiFi do core 2.0.17](https://github.com/espressif/arduino-esp32/blob/2.0.17/libraries/WiFi/src/WiFiSTA.h) e [rede Wokwi](https://docs.wokwi.com/guides/esp32-wifi).
