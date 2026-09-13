# Construir o firmware do app14, do zero

Três iterações. Cada uma compila e roda.

1. Sensores no Serial.
2. Os mesmos dados saindo por MQTT.
3. O `loop()` limpo, igual ao arquivo pronto.

Comece com `src/app14-NexoLog.ino` vazio. Os quatro `.hpp` já estão em `src/`.

---

## Iteração 1 — Sensores no Serial

```cpp
#include <Arduino.h>
#include "ESP32SensorsAmbiente.hpp"
#include "ESP32SensorsDistancia.hpp"
#include "ESP32SensorsAccel.hpp"
#include "ESP32SensorsLED.hpp"

const uint8_t DHT_PIN = 4;
const uint8_t DHT_MODEL = DHT22;
const uint8_t TRIG_PIN = 19;
const uint8_t ECHO_PIN = 18;
const uint8_t SCL_PIN = 23;
const uint8_t SDA_PIN = 22;
const uint8_t LED_PIN = 21;

const unsigned long INTERVALO_COLETA = 2500;
unsigned long tempoAnterior = 0;

void setup() {
  Serial.begin(115200);
  ESP32Sensors::Ambiente::inicializar(DHT_PIN, DHT_MODEL);
  ESP32Sensors::Distancia::inicializar(TRIG_PIN, ECHO_PIN);
  ESP32Sensors::Accel::inicializar(SCL_PIN, SDA_PIN);
  ESP32Sensors::LED::inicializar(LED_PIN);
}

void loop() {
  if (millis() - tempoAnterior >= INTERVALO_COLETA) {
    tempoAnterior = millis();

    ESP32Sensors::Ambiente::AMBIENTE amb = ESP32Sensors::Ambiente::medirAmbiente();
    ESP32Sensors::Distancia::DISTANCIA dist = ESP32Sensors::Distancia::medirDistancia();
    AccelData accel = ESP32Sensors::Accel::medirAccel();
    float movimentacao = ESP32Sensors::Accel::medirMovimentacao(accel);

    Serial.printf("Temp: %.1f C | Umid: %.1f %% | Dist: %.1f cm | Movim: %.2f m/s2\n",
                  amb.temp, amb.umid, dist.cm, movimentacao);
  }
}
```

**Funcionou?** No monitor serial, a cada 2,5 s:

```
Temp: 24.0 C | Umid: 40.0 % | Dist: 10.0 cm | Movim: 0.00 m/s2
```

- [ ] Temperatura e umidade com número, não `nan`
- [ ] Distância muda ao mexer no slider do HC-SR04 no Wokwi
- [ ] Movimentação perto de 0 parado, sobe ao arrastar o MPU

| Deu errado | Onde olhar |
|---|---|
| Tudo `nan` | pinos do bloco `const uint8_t`, contra o `diagram.json` |
| `Erro: MPU não encontrado` | `MPU_TYPE` no `ESP32SensorsAccel.hpp`: `MPU6050` no Wokwi, `MPU6500` na placa |
| Distância sempre `nan` | TRIG e ECHO trocados, ou GND do HC-SR04 solto |
| Serial em branco | `monitor_speed = 115200` no `platformio.ini` |

---

## Iteração 2 — Publicar por MQTT

Acrescente ao que já funciona.

**Includes**, junto dos demais:

```cpp
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
```

**Configuração**, depois dos pinos:

```cpp
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
#define MQTT_SERVER "host.wokwi.internal"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "NexoLogEquipe01"
#define MQTT_PUB_TOPIC "FIAPIoT/nexolog/equipe01/dados"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

const unsigned long INTERVALO_RECONEXAO = 5000;
unsigned long ultimaTentativaWiFi = 0;
unsigned long ultimaTentativaMQTT = 0;
```

**Duas funções**, antes do `setup()`:

```cpp
void conectarWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setSleep(false);
}

void conectarMQTT() {
  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println("[MQTT] Conectado");
  } else {
    Serial.printf("[MQTT] Falha: %d\n", mqttClient.state());
  }
}
```

**No fim do `setup()`:**

```cpp
  conectarWiFi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setBufferSize(768);
  mqttClient.setKeepAlive(120);
```

**No fim do `loop()`**, depois do bloco de coleta — reconecta sem `delay()`:

```cpp
  if (WiFi.status() != WL_CONNECTED) {
    if (millis() - ultimaTentativaWiFi >= INTERVALO_RECONEXAO) {
      ultimaTentativaWiFi = millis();
      WiFi.reconnect();
    }
  } else if (!mqttClient.connected()) {
    if (millis() - ultimaTentativaMQTT >= INTERVALO_RECONEXAO) {
      ultimaTentativaMQTT = millis();
      conectarMQTT();
    }
  } else {
    mqttClient.loop();
  }
```

**Depois do `Serial.printf`**, monte e publique o JSON:

```cpp
  JsonDocument doc;
  doc["device"] = MQTT_CLIENT_ID;
  doc["temp"] = amb.temp;
  doc["umid"] = amb.umid;
  doc["dist"] = dist.cm;
  doc["accel_x"] = accel.accelX;
  doc["accel_y"] = accel.accelY;
  doc["accel_z"] = accel.accelZ;
  doc["movimentacao"] = movimentacao;

  String payload;
  serializeJson(doc, payload);
  if (mqttClient.connected()) {
    bool ok = mqttClient.publish(MQTT_PUB_TOPIC, payload.c_str());
    Serial.println(ok ? "[MQTT] Publicado" : "[MQTT] Falha ao publicar");
  }
```

Sem `return` aqui: o bloco de reconexão está logo abaixo e precisa rodar.

**Funcionou?**

- [ ] `[MQTT] Conectado` aparece uma vez
- [ ] `[MQTT] Publicado` a cada 2,5 s
- [ ] `mosquitto_sub -h localhost -t 'FIAPIoT/nexolog/equipe01/dados'` mostra o JSON

| Deu errado | Onde olhar |
|---|---|
| `[MQTT] Falha: -2` | broker no ar? No Wokwi o host é `host.wokwi.internal`; na placa, o IP da Ethernet do notebook |
| Conecta e cai sozinho | dois ESP32 com o mesmo `MQTT_CLIENT_ID` |
| `Publicado`, mas nada no `mosquitto_sub` | tópico diferente entre firmware e assinante — confira `equipe01` |
| Publica e para depois de uns segundos | falta o `mqttClient.loop()` |

---

## Iteração 3 — Arrumar o `loop()`

O `loop()` está com duas responsabilidades misturadas: medir/publicar e cuidar da
conexão. Nada de novo aqui — só mover código. Compile antes e depois: a saída no
Serial tem que ser idêntica.

**a) Um protótipo**, junto dos outros, antes do `setup()`:

```cpp
bool enviarDadosColetados();
```

**b) Recorte** tudo que está dentro do `if (millis() - tempoAnterior >= ...)`, da
primeira medição até o `publish`, e cole no fim do arquivo dentro desta função —
trocando o `if (mqttClient.connected())` por dois `return`:

```cpp
bool enviarDadosColetados() {
  ESP32Sensors::Ambiente::AMBIENTE amb = ESP32Sensors::Ambiente::medirAmbiente();
  ESP32Sensors::Distancia::DISTANCIA dist = ESP32Sensors::Distancia::medirDistancia();
  AccelData accel = ESP32Sensors::Accel::medirAccel();
  float movimentacao = ESP32Sensors::Accel::medirMovimentacao(accel);

  Serial.printf("Temp: %.1f C | Umid: %.1f %% | Dist: %.1f cm | Movim: %.2f m/s2\n",
                amb.temp, amb.umid, dist.cm, movimentacao);

  JsonDocument doc;
  doc["device"] = MQTT_CLIENT_ID;
  doc["temp"] = amb.temp;
  doc["umid"] = amb.umid;
  doc["dist"] = dist.cm;
  doc["accel_x"] = accel.accelX;
  doc["accel_y"] = accel.accelY;
  doc["accel_z"] = accel.accelZ;
  doc["movimentacao"] = movimentacao;

  String payload;
  serializeJson(doc, payload);
  if (!mqttClient.connected()) return false;
  bool ok = mqttClient.publish(MQTT_PUB_TOPIC, payload.c_str());
  Serial.println(ok ? "[MQTT] Publicado" : "[MQTT] Falha ao publicar");
  return ok;
}
```

Agora o `return false` é seguro: ele sai de `enviarDadosColetados()`, não do `loop()`.

**c) No lugar do que você recortou**, uma linha:

```cpp
void loop() {
  if (millis() - tempoAnterior >= INTERVALO_COLETA) {
    tempoAnterior = millis();
    enviarDadosColetados();
  }

  // ... o bloco de reconexão da iteração 2, sem mudança
}
```

**Funcionou?**

- [ ] Serial idêntico ao da iteração 2
- [ ] `loop()` cabe na tela

| Deu errado | Onde olhar |
|---|---|
| `'enviarDadosColetados' was not declared in this scope` | faltou o protótipo do (a) |
| `'amb' was not declared` | sobrou no `loop()` uma linha que era para ter ido junto |

Esse é o `src/app14-NexoLog.ino` pronto. Compare.

---

Rodando as três: siga para [o fluxo no Node-RED](../Plataformas_config/NodeRED/CONSTRUIR-O-FLUXO.md).
