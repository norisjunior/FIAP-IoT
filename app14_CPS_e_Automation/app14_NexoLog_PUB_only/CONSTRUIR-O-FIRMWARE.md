# Construir o firmware do app14, do zero

Quatro iterações. Cada uma compila e roda.

1. Sensores no Serial.
2. Os mesmos dados saindo por MQTT.
3. O `loop()` limpo.
4. Um relógio por sensor.

Comece com `src/app14-NexoLog-PUBonly.ino` vazio. Os três `.hpp` dos sensores já
estão em `src/`. Este firmware só mede e publica — o LED entra no app15, quando
houver alguém mandando acender.

---

## Iteração 1 — Sensores no Serial

```cpp
#include <Arduino.h>
#include "ESP32SensorsAmbiente.hpp"
#include "ESP32SensorsDistancia.hpp"
#include "ESP32SensorsAccel.hpp"

const uint8_t DHT_PIN = 4;
const uint8_t DHT_MODEL = DHT22;
const uint8_t TRIG_PIN = 19;
const uint8_t ECHO_PIN = 18;
const uint8_t SCL_PIN = 23;
const uint8_t SDA_PIN = 22;

const unsigned long INTERVALO_COLETA = 2500;
unsigned long tempoAnterior = 0;

void setup() {
  Serial.begin(115200);
  ESP32Sensors::Ambiente::inicializar(DHT_PIN, DHT_MODEL);
  ESP32Sensors::Distancia::inicializar(TRIG_PIN, ECHO_PIN);
  ESP32Sensors::Accel::inicializar(SCL_PIN, SDA_PIN);

  // Linha de titulo do CSV. Serial.println ja termina em \r\n .
  Serial.println("temp,umid,dist,movimentacao");
}

void loop() {
  if (millis() - tempoAnterior >= INTERVALO_COLETA) {
    tempoAnterior = millis();

    ESP32Sensors::Ambiente::AMBIENTE amb = ESP32Sensors::Ambiente::medirAmbiente();
    ESP32Sensors::Distancia::DISTANCIA dist = ESP32Sensors::Distancia::medirDistancia();
    AccelData accel = ESP32Sensors::Accel::medirAccel();
    float movimentacao = ESP32Sensors::Accel::medirMovimentacao(accel);

    Serial.printf("%.1f,%.1f,%.1f,%.2f\r\n",
                  amb.temp, amb.umid, dist.cm, movimentacao);
  }
}
```

**Funcionou?** No monitor serial, a cada 2,5 s, uma linha de CSV:

```
temp,umid,dist,movimentacao
24.0,40.0,10.0,0.00
24.0,40.1,10.0,0.01
```

A linha de título sai uma vez, no `setup()`. Depois só números, na ordem dela — é assim
que um arquivo de dados se parece, e é o que você abre numa planilha ou lê com pandas.

O `Serial.println` do título já termina em `\r\n`; nos `printf` o `\r\n` vai escrito à
mão. Windows espera esse par, e é ele que faz cada leitura virar uma linha de verdade
no arquivo salvo.

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
    Serial.printf("[MQTT] Falha: %d\r\n", mqttClient.state());
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

  Serial.printf("%.1f,%.1f,%.1f,%.2f\r\n",
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

Compare com o `src/app14-NexoLog.ino`, tirando a iteração 4.

---

## Iteração 4 — Um relógio por sensor

Até aqui tudo acontece junto, a cada 2,5 s: lê os três sensores e publica. O problema
aparece quando você sacode o MPU. O acelerômetro é lido **uma vez** por envio, cerca
de 1 ms a cada 2500 ms. Sacudir a caixa por dois segundos inteiros e ver `0.00` no
Serial é o normal: a amostra caiu fora do movimento.

Cada sensor tem um ritmo próprio, e nenhum deles é o ritmo do envio:

| Sensor | Ritmo | Por quê |
|---|---|---|
| DHT22 | 2,1 s | o sensor não responde mais rápido — abaixo de 2 s ele devolve leitura inválida |
| MPU | 50 ms | para pegar o pico do movimento, não um instante sorteado |
| HC-SR04 | 1 s | junto do envio, a distância não muda em milissegundos |
| Publicar | 1 s | é o que a plataforma vai mostrar |

**a) Três relógios**, no lugar do bloco de intervalo:

```cpp
/* ---- Controle de intervalo ---- */
const unsigned long INTERVALO_COLETA = 1000;   // publica e le o ultrassonico
const unsigned long INTERVALO_DHT = 2100;      // o DHT22 nao responde mais rapido
const unsigned long INTERVALO_MPU = 50;        // 20 amostras por segundo
const unsigned long INTERVALO_RECONEXAO = 5000;
unsigned long tempoAnterior = 0, ultimoDHT = 0, ultimoMPU = 0;
unsigned long ultimaTentativaWiFi = 0, ultimaTentativaMQTT = 0;

/* ---- Medicoes guardadas entre um envio e outro ---- */
ESP32Sensors::Ambiente::AMBIENTE ambiente = {NAN, NAN, NAN, false};
ESP32Sensors::Distancia::DISTANCIA distancia = {NAN};
AccelData accel = {};
float movimentacaoMax = NAN;
```

Agora os três sensores são lidos fora da função que publica, então o valor de cada um
precisa ficar guardado até o envio — são essas quatro variáveis.

**b) Duas caixas novas no `loop()`**, antes do bloco de envio:

```cpp
  // O DHT22 e lento: guardamos a ultima leitura boa e enviamos ela.
  if (millis() - ultimoDHT >= INTERVALO_DHT) {
    ultimoDHT = millis();
    ESP32Sensors::Ambiente::AMBIENTE leitura = ESP32Sensors::Ambiente::medirAmbiente();
    if (leitura.valido) ambiente = leitura;
  }

  // O MPU e rapido: 20 amostras por segundo, guardamos so a maior.
  if (millis() - ultimoMPU >= INTERVALO_MPU) {
    ultimoMPU = millis();
    accel = ESP32Sensors::Accel::medirAccel();
    movimentacaoMax = fmaxf(movimentacaoMax, ESP32Sensors::Accel::medirMovimentacao(accel));
  }
```

`if (leitura.valido)` é o que faz o cache: leitura ruim é descartada e o último valor
bom continua valendo. `fmaxf` é o máximo entre o que já tinha e o que acabou de medir
— vinte comparações por segundo, e sobra o pico.

**c) O terceiro relógio** mede a distância e publica. E zera o pico, senão o maior valor
de hoje fica para sempre:

```cpp
  // O HC-SR04 e lido junto do envio: a distancia nao muda em milissegundos.
  if (millis() - tempoAnterior >= INTERVALO_COLETA) {
    tempoAnterior = millis();
    distancia = ESP32Sensors::Distancia::medirDistancia();
    enviarDadosColetados();
    movimentacaoMax = NAN;   // recomeca a procurar o pico
  }
```

**d) `enviarDadosColetados()` não mede mais nada.** Apague as quatro primeiras linhas,
que mediam os sensores, e troque `amb.` por `ambiente.`, `dist.` por `distancia.` e
`movimentacao` por `movimentacaoMax`:

```cpp
bool enviarDadosColetados() {
  Serial.printf("%.1f,%.1f,%.1f,%.2f\r\n",
                ambiente.temp, ambiente.umid, distancia.cm, movimentacaoMax);
```

Trocando também `doc["dist"] = dist.cm;` por `doc["dist"] = distancia.cm;`.

Repare no que sobrou: a função virou **só formatar e publicar**. Medir é assunto dos
três relógios do `loop()`, cada um no seu ritmo; a função pega o que eles deixaram
guardado.

O resto da função não muda. O payload continua com os mesmos oito campos: `accel_x/y/z`
passam a ser a última amostra do MPU, e `movimentacao` passa a ser o **pico do último
segundo**.

**Funcionou?**

- [ ] O Serial passa a sair uma vez por segundo
- [ ] Parado, `Movim` fica perto de 0,00 e a temperatura repete por dois envios seguidos
- [ ] Sacuda o MPU: `Movim` sobe e o valor daquele segundo é o mais forte do sacolejo
- [ ] Pare de sacudir: no envio seguinte já volta para perto de 0,00

| Deu errado | Onde olhar |
|---|---|
| `Movim` cresce e nunca cai | faltou o `movimentacaoMax = NAN;` do (c) |
| Temperatura sempre `nan` | `INTERVALO_DHT` abaixo de 2000: o sensor recusa e nunca preenche o cache |
| `Movim` continua sorteado | o `medirAccel()` ficou dentro de `enviarDadosColetados()`, em vez do relógio do MPU |
| Publica muito mais rápido que 1 s | tem dois `tempoAnterior = millis()` no `loop()` |
| `dist` sempre `nan` | o `medirDistancia()` ficou dentro da função de envio e some com a variável guardada |

Anote quanto marca com a caixa **parada**. O FastIMU está sem calibração, então um viés
de fábrica vira um piso constante — e é a partir desse piso que você escolhe o limiar
lá no n8n.

Esse é o `src/app14-NexoLog.ino` pronto. Compare.

---

Rodando as quatro: siga para [o fluxo no Node-RED](../Plataformas_config/NodeRED/CONSTRUIR-O-FLUXO.md).
