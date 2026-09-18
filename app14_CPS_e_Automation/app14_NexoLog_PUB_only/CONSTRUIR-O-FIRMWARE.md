# Construir o firmware do app14, do zero

Quatro iterações. Cada uma compila e roda.

1. Sensores no Serial.
2. Um relógio por sensor.
3. Os mesmos dados saindo por MQTT.
4. O `loop()` limpo.

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
    float dist = ESP32Sensors::Distancia::medirDistancia();
    AccelData accel = ESP32Sensors::Accel::medirAccel();
    float movimentacao = ESP32Sensors::Accel::medirMovimentacao(accel);

    Serial.printf("%.1f,%.1f,%.1f,%.2f\r\n",
                  amb.temp, amb.umid, dist, movimentacao);
  }
}
```

**Funcionou?** No monitor serial, a cada 2,5 s, uma linha de CSV:

```
temp,umid,dist,movimentacao
24.0,40.0,10.0,0.00
24.0,40.1,10.0,0.01
```

Título uma vez, no `setup()`; depois só números, na ordem dele. O `println` já termina
em `\r\n`; nos `printf` o `\r\n` vai escrito à mão, que é o que o Windows espera.

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

## Iteração 2 — Um relógio por sensor

Sacuda o MPU por dois segundos e veja `0.00` no Serial. É o normal: o acelerômetro é
lido **uma vez** por ciclo, 1 ms a cada 2500 ms, e a amostra cai fora do movimento.

Cada sensor tem um ritmo próprio, e nenhum deles é o ritmo do envio:

| Sensor | Ritmo | Por quê |
|---|---|---|
| DHT22 | 2,1 s | o sensor não responde mais rápido — abaixo de 2 s ele devolve leitura inválida |
| MPU | 50 ms | para pegar o pico do movimento, não um instante sorteado |
| HC-SR04 | 1 s | junto do envio, a distância não muda em milissegundos |
| Enviar | 1 s | é o que a plataforma vai mostrar |

**a) Três relógios**, no lugar do bloco de intervalo:

```cpp
/* ---- Controle de intervalo ---- */
const unsigned long INTERVALO_COLETA = 1000;   // publica e le o ultrassonico
const unsigned long INTERVALO_DHT = 2100;      // o DHT22 nao responde mais rapido
const unsigned long INTERVALO_MPU = 50;        // 20 amostras por segundo
unsigned long tempoAnterior = 0, ultimoDHT = 0, ultimoMPU = 0;

/* ---- Medicoes guardadas entre um envio e outro ---- */
ESP32Sensors::Ambiente::AMBIENTE ambiente = {NAN, NAN, NAN, false};
float distancia = NAN;
AccelData accel = {};
float movimentacaoMax = NAN;
```

Os três sensores passam a ser lidos em momentos diferentes, então o valor de cada um
precisa ficar guardado até a hora de mostrar — são essas quatro variáveis. Elas saem de
dentro do `loop()` e viram globais.

**b) Duas caixas novas no `loop()`**, antes do bloco que já existe:

```cpp
  // O DHT22 e lento: guardamos a ultima leitura boa e enviamos ela.
  if (millis() - ultimoDHT >= INTERVALO_DHT) {
    ultimoDHT = millis();
    ESP32Sensors::Ambiente::AMBIENTE leitura = ESP32Sensors::Ambiente::medirAmbiente();
    if (leitura.valido) {
      ambiente = leitura;
    }
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

**c) O terceiro relógio** já existe; ele perde as três medições que subiram, mede só a
distância e zera o pico:

```cpp
  // O HC-SR04 e lido junto do envio: a distancia nao muda em milissegundos.
  if (millis() - tempoAnterior >= INTERVALO_COLETA) {
    tempoAnterior = millis();
    distancia = ESP32Sensors::Distancia::medirDistancia();

    Serial.printf("%.1f,%.1f,%.1f,%.2f\r\n",
                  ambiente.temp, ambiente.umid, distancia, movimentacaoMax);

    movimentacaoMax = NAN;   // recomeca a procurar o pico
  }
```

Sem zerar, o maior valor do dia ficaria para sempre.

**Funcionou?**

- [ ] O Serial passa a sair uma vez por segundo
- [ ] Parado, a última coluna fica perto de 0,00 e a temperatura repete por dois envios
- [ ] Sacuda o MPU: a última coluna sobe e mostra o pico daquele segundo
- [ ] Pare de sacudir: no envio seguinte já volta para perto de 0,00

| Deu errado | Onde olhar |
|---|---|
| A última coluna cresce e nunca cai | faltou o `movimentacaoMax = NAN;` do (c) |
| Temperatura sempre `nan` | `INTERVALO_DHT` abaixo de 2000: o sensor recusa e nunca preenche o cache |
| A última coluna continua sorteada | o `medirAccel()` ficou no bloco de 1 s, em vez do relógio do MPU |
| Sai muito mais rápido que 1 s | tem dois `tempoAnterior = millis()` no `loop()` |
| `'amb' was not declared` | as medições viraram globais com outros nomes: `ambiente`, `distancia` |

Anote quanto marca com a caixa **parada**. O FastIMU está sem calibração, então um viés
de fábrica vira um piso constante — e é a partir desse piso que você escolhe o limiar
lá no n8n.

---

## Iteração 3 — Publicar por MQTT

Acrescente ao que já funciona. Os nomes das medições não mudam mais.

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
```

**Mais um intervalo e dois relógios**, no bloco de controle:

```cpp
const unsigned long INTERVALO_RECONEXAO = 5000;
unsigned long proximaTentativaWiFi = 0, proximaTentativaMQTT = 0;
```

**Duas funções**, no fim do arquivo:

```cpp
void conectarWiFi() {
  // Uma tentativa a cada INTERVALO_RECONEXAO.
  if (millis() < proximaTentativaWiFi) {
    return;
  }
  proximaTentativaWiFi = millis() + INTERVALO_RECONEXAO;

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setSleep(false);
}

void conectarMQTT() {
  if (millis() < proximaTentativaMQTT) {
    return;
  }
  proximaTentativaMQTT = millis() + INTERVALO_RECONEXAO;

  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println("[MQTT] Conectado");
  } else {
    Serial.printf("[MQTT] Falha: %d\r\n", mqttClient.state());
  }
}
```

Cada função cuida do próprio ritmo de tentativa. Quem chama não precisa saber disso —
chama à vontade, a função decide se é hora.

A variável guarda **quando pode tentar de novo**, e começa em zero: no boot,
`millis()` já é maior que zero, então a primeira chamada passa direto.

**Dois protótipos**, antes do `setup()`, porque as funções agora ficam depois de quem
as chama:

```cpp
void conectarWiFi();
void conectarMQTT();
```

**No `setup()`**, entre os `inicializar` e a linha de título do CSV:

```cpp
  conectarWiFi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setBufferSize(768);
  mqttClient.setKeepAlive(120);
```

**No fim do `loop()`**, depois dos três relógios — três linhas, sem `delay()`:

```cpp
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  } else if (!mqttClient.connected()) {
    conectarMQTT();
  } else {
    mqttClient.loop();
  }
```

Sem Wi-Fi, tenta Wi-Fi. Com Wi-Fi e sem MQTT, tenta MQTT. Com os dois, `mqttClient.loop()`
mantém a conexão viva.

**Depois do `Serial.printf`**, monte e publique o JSON:

```cpp
    JsonDocument doc;
    doc["device"] = MQTT_CLIENT_ID;
    doc["temp"] = ambiente.temp;
    doc["umid"] = ambiente.umid;
    doc["dist"] = distancia;
    doc["accel_x"] = accel.accelX;
    doc["accel_y"] = accel.accelY;
    doc["accel_z"] = accel.accelZ;
    doc["movimentacao"] = movimentacaoMax;

    String payload;
    serializeJson(doc, payload);
    if (mqttClient.connected()) {
      bool ok = mqttClient.publish(MQTT_PUB_TOPIC, payload.c_str());
      Serial.println(ok ? "[MQTT] Publicado" : "[MQTT] Falha ao publicar");
    }
```

Sem `return` aqui: o bloco de conexão está logo abaixo e precisa rodar.

**Funcionou?**

- [ ] `[MQTT] Conectado` aparece uma vez, logo no começo
- [ ] `[MQTT] Publicado` a cada 1 s, entre as linhas do CSV
- [ ] `mosquitto_sub -h localhost -t 'FIAPIoT/nexolog/equipe01/dados'` mostra o JSON

| Deu errado | Onde olhar |
|---|---|
| `[MQTT] Falha: -2` | broker no ar? No Wokwi o host é `host.wokwi.internal`; na placa, o IP da Ethernet do notebook |
| Demora 5 s para conectar ao ligar | a variável guarda a **próxima** tentativa, não a última: confira o sinal do `<` |
| Conecta e cai sozinho | dois ESP32 com o mesmo `MQTT_CLIENT_ID` |
| `Publicado`, mas nada no `mosquitto_sub` | tópico diferente entre firmware e assinante — confira `equipe01` |
| Publica e para depois de uns segundos | falta o `mqttClient.loop()` |

---

## Iteração 4 — Arrumar o `loop()`

O `loop()` está com duas responsabilidades misturadas: medir/publicar e cuidar da
conexão. Nada de novo aqui — só mover código. Compile antes e depois: a saída no
Serial tem que ser idêntica.

**a) Mais um protótipo**, junto dos outros:

```cpp
bool enviarDadosColetados();
```

**b) Recorte** tudo que está dentro do `if (millis() - tempoAnterior >= ...)`, do
`Serial.printf` até o `publish`, e cole no fim do arquivo dentro desta função —
trocando o `if (mqttClient.connected())` por dois `return`:

```cpp
bool enviarDadosColetados() {
  Serial.printf("%.1f,%.1f,%.1f,%.2f\r\n",
                ambiente.temp, ambiente.umid, distancia, movimentacaoMax);

  JsonDocument doc;
  doc["device"] = MQTT_CLIENT_ID;
  doc["temp"] = ambiente.temp;
  doc["umid"] = ambiente.umid;
  doc["dist"] = distancia;
  doc["accel_x"] = accel.accelX;
  doc["accel_y"] = accel.accelY;
  doc["accel_z"] = accel.accelZ;
  doc["movimentacao"] = movimentacaoMax;

  String payload;
  serializeJson(doc, payload);
  if (!mqttClient.connected()) {
    return false;
  }
  bool ok = mqttClient.publish(MQTT_PUB_TOPIC, payload.c_str());
  Serial.println(ok ? "[MQTT] Publicado" : "[MQTT] Falha ao publicar");
  return ok;
}
```

Agora o `return false` é seguro: ele sai de `enviarDadosColetados()`, não do `loop()`.

**c) No lugar do que você recortou**, uma linha:

```cpp
  // O HC-SR04 e lido junto do envio: a distancia nao muda em milissegundos.
  if (millis() - tempoAnterior >= INTERVALO_COLETA) {
    tempoAnterior = millis();
    distancia = ESP32Sensors::Distancia::medirDistancia();
    enviarDadosColetados();
    movimentacaoMax = NAN;   // recomeca a procurar o pico
  }
```

Repare no `loop()` inteiro agora: três caixas de relógio e três linhas de conexão. Cada
sensor no seu ritmo, e nenhuma medição solta. Medir é assunto do `loop()`; formatar e
publicar é da função.

**Funcionou?**

- [ ] Serial idêntico ao da iteração 3
- [ ] `loop()` cabe na tela

| Deu errado | Onde olhar |
|---|---|
| `'enviarDadosColetados' was not declared in this scope` | faltou o protótipo do (a) |
| `dist` sempre `nan` | o `medirDistancia()` foi junto para dentro da função de envio |
| Parou de reconectar | as três linhas de conexão foram recortadas junto |

Esse é o `src/app14-NexoLog-PUBonly.ino` pronto. Compare.

---

Rodando as quatro: siga para [o fluxo no Node-RED](../Plataformas_config/NodeRED/CONSTRUIR-O-FLUXO.md).
