# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## MQTT com ESP32 — Passo a Passo

Sequência de passos de como estruturar uma aplicação MQTT.

A seguir seguem exemplos para um ESP32 que **publica** a leitura de um sensor e para um ESP32 que **recebe** via callback.

> Os códigos-base já vêm com WiFi, sensor de distância (namespace `ESP32Sensors::Distancia`) e ArduinoJson funcionando (o mesmo modelo usado em aula). Deve-se apenas preencher os **TODOs** do MQTT.

---

### Passo 1: Adicionar a biblioteca no `platformio.ini`

Para utilizar o MQTT no ESP32 deve-se utilizar a biblioteca **PubSubClient**. Deve ser inserida no `platformio.ini` dessa maneira:

```ini
lib_deps =
    knolleary/PubSubClient@^2.8
```

No caso da aplicação modelo (app_WiFiModelo), o `platformio.ini` já tem outras bibliotecas (ArduinoJson, EasyUltrasonic). Basta **adicionar** a linha da PubSubClient junto com as demais:

```ini
lib_deps =
    knolleary/PubSubClient@^2.8
    bblanchon/ArduinoJson@^7.4.1
    EasyUltrasonic
```

---

### Passo 2: Incluir a biblioteca no código

No arquivo da aplicação (`src/app.ino`), adicionar o include da PubSubClient junto com os demais:

```cpp
#include <PubSubClient.h>
```

---

### Passo 3: Configurar os parâmetros do MQTT

Definir o endereço do broker, porta, tópico, ID do dispositivo e criar o objeto `PubSubClient`. Esses defines ficam na área de configurações, junto com as configs de WiFi e sensor:

#### Dispositivo que realiza publish:

```cpp
/* ---- Config MQTT ---- */
#define MQTT_HOST       "broker.emqx.io"
#define MQTT_PORT       1883
#define MQTT_PUB_TOPIC  "FIAP/sala/distancia"
#define MQTT_DEVICEID   "DistSensor00001"

PubSubClient client(wifiClient);
```

#### Dispositivo que realiza subscribe (adicionar também o tópico de subscribe):

```cpp
/* ---- Config MQTT ---- */
#define MQTT_HOST       "broker.emqx.io"
#define MQTT_PORT       1883
#define MQTT_SUB_TOPIC  "FIAP/sala/distancia"
#define MQTT_DEVICEID   "DistSensor00002"

PubSubClient client(wifiClient);
```

> **Importante:** cada ESP32 deve ter um `MQTT_DEVICEID` diferente. O tópico de publish de um dispositivo deve ser o mesmo tópico de subscribe do outro.

> Caso o dispositivo faça PUBLISH **e** SUBSCRIBE, todos os tópicos devem ser declarados, ou devem ser usados os wildcards "#" e "+" conforme a aplicação.

---

### Passo 4: Criar a função para conectar ao broker MQTT

Criar a função `conectarMQTT()` na seção de funções, junto com a `conectarWiFi()`. Essa função será chamada no `loop()`:

#### Quando o dispositivo apenas realiza publish:

```cpp
void conectarMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando ao broker MQTT...");
    if (client.connect(MQTT_DEVICEID)) {
      Serial.println(" Conectado!");
    } else {
      Serial.print(" Falhou. Código: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}
```

#### Quando o dispositivo realiza subscribe, ele deve assinar o tópico desejado logo depois de conectar:

> **Atenção:** antes de implementar esta função, complete o **Passo 8** (declaração do callback), pois ela precisa aparecer no código **antes** do `setup()`.

Função de conectar MQTT:
```cpp
void conectarMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando ao broker MQTT...");
    if (client.connect(MQTT_DEVICEID)) {
      Serial.println(" Conectado!");
      client.subscribe(MQTT_SUB_TOPIC);
    } else {
      Serial.print(" Falhou. Código: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}
```

---

### Passo 5: Apontar o client para o broker no `setup()`

No `setup()`, após conectar ao WiFi, informar ao client o endereço e porta do broker:

```cpp
client.setServer(MQTT_HOST, MQTT_PORT);
```

---

### Passo 6: Publicar uma mensagem (dispositivo que ENVIA)

Para enviar uma mensagem em um tópico, usar `client.publish()`. No nosso modelo, publicamos o JSON com a leitura do sensor dentro do bloco de coleta (onde já temos o `millis()`):

```cpp
client.publish(MQTT_PUB_TOPIC, buffer);
```

Onde `buffer` é o JSON serializado que já montamos com ArduinoJson.

---

### Passo 7: Assinar um tópico (dispositivo que RECEBE)

Para receber mensagens, são necessárias **mais alterações**:

**7a)** Registrar a função de callback no `setup()`:

```cpp
client.setCallback(callback);
```

**7b)** Assinar o tópico logo após conectar ao broker (dentro da função `conectarMQTT`, após o `connect` bem-sucedido) - como consta no exemplo do **Passo 4**;


---

### Passo 8: Criar a função de callback (dispositivo que RECEBE)

A função `callback` é chamada **automaticamente** toda vez que uma mensagem chega no tópico assinado. No nosso modelo, a mensagem chega como JSON e pode ser parseada com ArduinoJson:

```cpp
void callback(char* topic, byte* payload, unsigned int length) {
  String mensagem = "";
  for (unsigned int i = 0; i < length; i++) {
    mensagem += (char)payload[i];
  }
  Serial.print("Recebido no tópico [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(mensagem);

  // Exemplo: parsear o JSON recebido
  JsonDocument doc;
  deserializeJson(doc, mensagem);
  float dist = doc["dist"];
  Serial.printf("Distância recebida: %.2f cm\n", dist);

  // Aqui você decide o que fazer com o valor recebido
}
```

> **Atenção:** a função `callback` deve ser declarada **antes** do `setup()`.

---

### Passo 9 — Manter a conexão viva no `loop()`

O `client.loop()` é **obrigatório** no `loop()`. Ele mantém a conexão com o broker e processa as mensagens recebidas:

```cpp
if (!client.connected()) {
  conectarMQTT();
}
client.loop();
```

---

## Código-base:
`app_WiFi_modelo/src/iot-app-wifi-exemplo.ino`


---

## Referência rápida

| Função | O que faz |
|---|---|
| `client.setServer(host, porta)` | Define o broker |
| `client.setCallback(funcao)` | Registra o callback |
| `client.connect("id")` | Conecta ao broker |
| `client.subscribe("topico")` | Assina um tópico |
| `client.publish("topico", "msg")` | Publica uma mensagem |
| `client.loop()` | Mantém conexão e processa mensagens |
| `client.connected()` | Retorna `true` se conectado |