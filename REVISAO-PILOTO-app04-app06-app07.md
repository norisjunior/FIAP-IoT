# Revisão piloto — app04, app06 e app07

Somente estes três projetos foram alterados, em `C:/Projects/FIAP-IoT-eval`, para revisão do professor antes de ampliar o padrão. Não houve commit, simulação ou gravação. O guia MQTT do projeto modelo e os demais apps aguardam essa revisão.

## O que mudou

| App | Mudança | Compilação |
|---|---|---|
| app04 | FastIMU no mesmo `ESP32Sensors::Accel`; struct `ACCEL { x, y, z }` por valor; unidades em m/s²; WiFi comum; DHT e versões fixadas | SUCCESS com MPU6050 e com MPU6500 |
| app06 | HC-SR04 via `pulseIn` em header; payload inteiro e ausência de eco = 400 preservados; rede MQTT comum | SUCCESS |
| app07 | Rede MQTT comum; assinatura renovada após reconexão; callback original preservado | SUCCESS |

Foi mantido o estilo com funções explícitas, configurações no início do `.ino` e módulos locais em `.hpp`. Não foram acrescentados `malloc`, `new`, classes próprias para conexão, tarefas ou filas. A API FastIMU exige `getAccel(&leitura)`; esse detalhe fica no header, e a aplicação recebe uma struct por valor. Foram mantidos o nome `Accel` e o arquivo `ESP32SensorsAccel.hpp`, ajustando a proposta inicial para ficar mais próximo da forma de codificar do professor.

## Configurações que o aluno modifica

- `MPU_TYPE`: `MPU6050` no Wokwi; `MPU6500` no kit com esse modelo.
- `WIFI_SSID`, `WIFI_SENHA`: Wokwi usa rede aberta; rede física usa suas credenciais. `WiFi.begin` recebe apenas SSID e senha, com canal automático.
- `MQTT_BROKER`: os pilotos usam `host.wokwi.internal`, com broker local acessível pelo gateway do Wokwi. Na rede do laboratório, usar `172.16.10.101`.
- `MQTT_CLIENT_ID`: trocar o RA de exemplo e manter IDs distintos por aluno/dispositivo.
- No assinante, acrescentar `mqttClient.subscribe(TOPICO);` explicitamente após conectar ao MQTT. O publicador não faz subscribe.

Os pinos, o tópico `fiap/iot/distancia`, as chaves do JSON do app04 e os intervalos de aquisição/publicação foram conferidos contra o HEAD anterior. Os três diagramas foram preservados. Nos tempos e durações do piloto, `uint32_t` substituiu `unsigned long` e `uint64_t`; pinos e portas mantêm seus tipos existentes. As funções WiFi são textualmente idênticas nos três apps; MQTT segue a mesma estrutura nos dois apps, com uma chamada explícita de subscribe acrescentada no app07.

## Limites desta revisão

- Compilação não verifica leituras físicas nem conectividade. Não foram executados Wokwi, broker ou servidor Python.
- PubSubClient faz conexão síncrona. As tentativas foram espaçadas por `millis()`, mas uma tentativa pode atrasar o `loop()`. O timeout MQTT de 2 s não limita toda a conexão TCP.
- MQTT e o intervalo de chamada WiFi seguem o app14, com `uint32_t`, canal automático e sem `delay` inicial no setup. A primeira tentativa ocorre após o intervalo inicial de 5 s.
- Ao iniciar uma conexão, `conectarWiFi()` aguarda `WL_CONNECTED` com `while` e `delay(500)`. Só depois imprime `[WiFi] Conectado` e `WiFi.localIP()`, mostrando o IP obtido em cada conexão iniciada, inclusive reconexões. Não imprime mensagens durante a espera.
- O loop apenas verifica se WiFi está desconectado e chama a função; não tem flag nem impressão do IP. Enquanto espera a conexão, o processamento do loop fica pausado, conforme a simplificação didática escolhida pelo professor.
- App04 mantém seu intervalo de 5 s entre ciclos; a conexão TCP é tentada somente com WiFi conectado.
- App06 preserva a conversão inteira `/ 57` da NewPing. O header mostra a fórmula didática `duracao * 0.034 / 2` e também fornece a duração para essa compatibilidade. O pulso TRIG passou de 12 us (NewPing) para 10 us; a aquisição/timeout do `pulseIn` não é temporalmente idêntica à da NewPing.
- Os exemplos não acrescentam armazenamento/reenvio offline.

## Compilações executadas

App04 foi compilado após mover o aviso/IP para `conectarWiFi()`. App06 e app07 foram recompilados após retirar a chave de configuração de subscribe e manter somente a chamada explícita no assinante:

| Projeto/modelo | RAM | Flash | Resultado |
|---|---:|---:|---|
| app04 / MPU6050 | 45236 bytes | 777853 bytes | SUCCESS |
| app06 | 44824 bytes | 749405 bytes | SUCCESS |
| app07 | 44808 bytes | 754449 bytes | SUCCESS |

O MPU6500 também compilou com sucesso na revisão inicial deste piloto, usando `PLATFORMIO_BUILD_FLAGS=-DMPU_TYPE=MPU6500` temporário. A implementação do acelerômetro não mudou nesta revisão de conexão. O código e o binário atuais ficam com MPU6050. Não houve simulação ou gravação.

## Trechos dos slides — antes e depois

O “antes” abaixo vem de `git show HEAD:arquivo`. O “depois” vem do arquivo atual. Os números indicam linhas reais do arquivo, não deste relatório.

### app04

Arquivo: `app04_ESP_transmite_Server/app04_transmite_JSON/src/iot-aula02-app04.ino`.

#### setup()

Antes — linhas 29 a 45:

```cpp
void setup() {
  Serial.begin(115200);

  //Inicializa todos os sensores
  ESP32Sensors::Ambiente::inicializar(DHT_PIN, DHT_MODEL);
  ESP32Sensors::Accel::inicializar(SDA_PIN, SCL_PIN);
  ESP32Sensors::LED::inicializar(LED_PIN);

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
}
```

Depois — linhas 58 a 67:

```cpp
void setup() {
  Serial.begin(115200);

  //Inicializa todos os sensores
  ESP32Sensors::Ambiente::inicializar(DHT_PIN, DHT_MODEL);
  ESP32Sensors::Accel::inicializar(SDA_PIN, SCL_PIN);
  ESP32Sensors::LED::inicializar(LED_PIN);

  conectarWiFi();
}
```

#### loop()

Antes — linhas 47 a 78:

```cpp
void loop() {
  //Mede temperatura, umidade e índice de calor
  ESP32Sensors::Ambiente::AMBIENTE salaMaquinas = ESP32Sensors::Ambiente::medirAmbiente();

  //Mede trepidação/movimentação do motor (eixos x, y e z)
  sensors_event_t motor = ESP32Sensors::Accel::medirAccel();

  WiFiClient client;
  if (client.connect(host, port)) {

    /* Transmissão dos dados em formato JSON */
    JsonDocument doc;

    doc["device"] = device;
    doc["temp"] = salaMaquinas.temp;
    doc["umid"] = salaMaquinas.umid;
    doc["hic"] = salaMaquinas.ic;
    doc["motor_accel_x"] = motor.acceleration.x;
    doc["motor_accel_y"] = motor.acceleration.y;
    doc["motor_accel_z"] = motor.acceleration.z;

    String output;
    serializeJson(doc, output);
    client.println(output);
    Serial.println("Dados enviados: " + output);
    client.stop();
  } else {
    Serial.println("Falha ao conectar no servidor");
  }

  delay(5000); // Aguarda 5s
}
```

Depois — linhas 69 a 104:

```cpp
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

  //Mede temperatura, umidade e índice de calor
  ESP32Sensors::Ambiente::AMBIENTE salaMaquinas = ESP32Sensors::Ambiente::medirAmbiente();

  //Mede trepidação/movimentação do motor (eixos x, y e z)
  ESP32Sensors::Accel::ACCEL motor = ESP32Sensors::Accel::medirAccel();

  WiFiClient client;
  if (WiFi.status() == WL_CONNECTED && client.connect(host, port)) {

    /* Transmissão dos dados em formato JSON */
    JsonDocument doc;

    doc["device"] = device;
    doc["temp"] = salaMaquinas.temp;
    doc["umid"] = salaMaquinas.umid;
    doc["hic"] = salaMaquinas.ic;
    doc["motor_accel_x"] = motor.x;
    doc["motor_accel_y"] = motor.y;
    doc["motor_accel_z"] = motor.z;

    String output;
    serializeJson(doc, output);
    client.println(output);
    Serial.println("Dados enviados: " + output);
    client.stop();
  } else {
    Serial.println("Falha ao conectar no servidor");
  }

  delay(5000); // Aguarda 5s
}
```

A função WiFi foi extraída do setup e acrescentada nas linhas 37 a 56:

```cpp
void conectarWiFi() {
  // Uma tentativa a cada INTERVALO_RECONEXAO, como no app14.
  if (millis() - ultimaTentativaWiFi < INTERVALO_RECONEXAO) {
    return;
  }
  ultimaTentativaWiFi = millis();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_SENHA);
  WiFi.setSleep(false);

  // Aguarda conectar para mostrar o IP obtido nesta conexão.
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.println("[WiFi] Conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}
```

### app06

Arquivo: `app06_mqtt_publisher_distancia/src/app06-publisher-distancia.ino`.

#### conectarWiFi()

Antes — linhas 31 a 42:

```cpp
void conectarWiFi() {
  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_SENHA);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println(" conectado!");
}
```

Depois — linhas 35 a 54:

```cpp
void conectarWiFi() {
  // Uma tentativa a cada INTERVALO_RECONEXAO, como no app14.
  if (millis() - ultimaTentativaWiFi < INTERVALO_RECONEXAO) {
    return;
  }
  ultimaTentativaWiFi = millis();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_SENHA);
  WiFi.setSleep(false);

  // Aguarda conectar para mostrar o IP obtido nesta conexão.
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.println("[WiFi] Conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}
```

#### conectarMQTT()

Antes — linhas 44 a 55:

```cpp
void conectarMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Conectando ao broker MQTT...");

    if (mqtt.connect(CLIENT_ID)) {
      Serial.println(" conectado!");
    } else {
      Serial.println(" falhou. Nova tentativa em 2 segundos.");
      delay(2000);
    }
  }
}
```

Depois — linhas 56 a 67:

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

#### setup()

Antes — linhas 57 a 65:

```cpp
void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  conectarWiFi();
  mqtt.setServer(BROKER_IP, 1883);
  mqtt.setKeepAlive(120);
}
```

Depois — linhas 69 a 77:

```cpp
void setup() {
  Serial.begin(115200);
  ESP32Sensors::Distancia::inicializar(TRIG_PIN, ECHO_PIN, TEMPO_LIMITE_ECHO);

  conectarWiFi();
  mqttClient.setServer(MQTT_BROKER, MQTT_PORTA);
  mqttClient.setKeepAlive(120);
  mqttClient.setSocketTimeout(2); // Limita a espera pela resposta MQTT, em segundos
}
```

#### loop()

Antes — linhas 67 a 84:

```cpp
void loop() {
  if (!mqtt.connected()) {
    conectarMQTT();
  }
  mqtt.loop();

  if (millis() - tempoAgora >= INTERVALO_COLETA) {

    tempoAgora = millis();
    float distancia = sensorDist.ping_cm();
    if (distancia == 0) distancia = 400;  // sem eco = muito longe
    String mensagem = String(distancia, 0);

    mqtt.publish(TOPICO, mensagem.c_str());
    Serial.println("Distancia publicada: " + mensagem + " cm");
  }

}
```

Depois — linhas 79 a 100:

```cpp
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  } else if (!mqttClient.connected()) {
    conectarMQTT();
  } else {
    mqttClient.loop();
  }

  if (millis() - tempoAgora >= INTERVALO_COLETA) {
    tempoAgora = millis();
    ESP32Sensors::Distancia::DISTANCIA leitura = ESP32Sensors::Distancia::medirDistancia();

    // Mantém a conversão inteira da NewPing usada neste exemplo (57 us por cm).
    float distancia = leitura.duracaoUs / 57;
    if (!leitura.valido || distancia == 0 || distancia > 400) distancia = 400;
    String mensagem = String(distancia, 0);

    mqttClient.publish(TOPICO, mensagem.c_str());
    Serial.println("Distancia publicada: " + mensagem + " cm");
  }
}
```

### app07

Arquivo: `app07_mqtt_subscriber_atuador/src/app07-subscriber-atuador.ino`.

#### conectarWiFi()

Antes — linhas 27 a 38:

```cpp
void conectarWiFi() {
  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_SENHA);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println(" conectado!");
}
```

Depois — linhas 31 a 50:

```cpp
void conectarWiFi() {
  // Uma tentativa a cada INTERVALO_RECONEXAO, como no app14.
  if (millis() - ultimaTentativaWiFi < INTERVALO_RECONEXAO) {
    return;
  }
  ultimaTentativaWiFi = millis();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_SENHA);
  WiFi.setSleep(false);

  // Aguarda conectar para mostrar o IP obtido nesta conexão.
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.println("[WiFi] Conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}
```

#### conectarMQTT()

Antes — linhas 48 a 61:

```cpp
void conectarMQTT() {
  // O endereco MAC deixa o nome de cada ESP32 unico na sala.
  while (!mqtt.connected()) {
    Serial.print("Conectando ao broker MQTT...");

    if (mqtt.connect(CLIENT_ID)) {
      Serial.println(" conectado!");
      mqtt.subscribe(TOPICO);
    } else {
      Serial.println(" falhou. Nova tentativa em 2 segundos.");
    }
  delay(2000);
  }
}
```

Depois — linhas 60 a 72:

```cpp
void conectarMQTT() {
  if (millis() - ultimaTentativaMQTT < INTERVALO_RECONEXAO) {
    return;
  }
  ultimaTentativaMQTT = millis();

  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println("[MQTT] Conectado");
    mqttClient.subscribe(TOPICO);
  } else {
    Serial.printf("[MQTT] Falha: %d\r\n", mqttClient.state());
  }
}
```

#### setup()

Antes — linhas 63 a 71:

```cpp
void setup() {
  Serial.begin(115200);
  pinMode(PINO_LED, OUTPUT);

  conectarWiFi();
  mqtt.setServer(BROKER_IP, 1883);
  mqtt.setKeepAlive(120);
  mqtt.setCallback(receberMensagem);
}
```

Depois — linhas 74 a 83:

```cpp
void setup() {
  Serial.begin(115200);
  pinMode(PINO_LED, OUTPUT);

  conectarWiFi();
  mqttClient.setServer(MQTT_BROKER, MQTT_PORTA);
  mqttClient.setKeepAlive(120);
  mqttClient.setSocketTimeout(2); // Limita a espera pela resposta MQTT, em segundos
  mqttClient.setCallback(receberMensagem);
}
```

#### loop()

Antes — linhas 73 a 80:

```cpp
void loop() {
  if (!mqtt.connected()) {
    conectarMQTT();
  }

  mqtt.loop();  // recebe as mensagens do broker
  delay(100);
}
```

Depois — linhas 85 a 94:

```cpp
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  } else if (!mqttClient.connected()) {
    conectarMQTT();
  } else {
    mqttClient.loop();
  }
  delay(100);
}
```

#### Callback preservado

`receberMensagem()` passou das linhas 40–46 para 52–58, sem mudança no conteúdo:

```cpp
void receberMensagem(char* topico, byte* conteudo, unsigned int tamanho) {
  String mensagem(conteudo, tamanho);
  float distancia = mensagem.toFloat();

  Serial.println("Distancia recebida: " + mensagem + " cm");
  digitalWrite(PINO_LED, distancia <= LIMITE);
}
```

## Headers de sensores para revisão

### App04 — retorno do acelerômetro

Antes:

```cpp
sensors_event_t motor = ESP32Sensors::Accel::medirAccel();
doc["motor_accel_x"] = motor.acceleration.x;
```

Depois:

```cpp
ESP32Sensors::Accel::ACCEL motor = ESP32Sensors::Accel::medirAccel();
doc["motor_accel_x"] = motor.x;
```

O mesmo ajuste foi feito para y e z. Abaixo está o header completo atual:

Arquivo: `app04_ESP_transmite_Server/app04_transmite_JSON/src/ESP32SensorsAccel.hpp`.

```cpp
#pragma once
#include <Wire.h>
#include <FastIMU.h>

// A biblioteca é a mesma para o MPU6050 e para o MPU6500.
#ifndef MPU_TYPE
#define MPU_TYPE MPU6050
#endif

namespace ESP32Sensors {
  namespace Accel {

    struct ACCEL {
      float x;
      float y;
      float z;
    };

    MPU_TYPE mpu;
    calData calib = {0};  // Sem calibração

    void inicializar(uint8_t sda, uint8_t scl) {
      Wire.begin(sda, scl);
      if (mpu.init(calib, 0x68) != 0) {
        Serial.println("Erro: MPU não encontrado. Verifique modelo e conexões.");
        while (1) delay(10);
      }

      // Preserva a configuração do app04: ±16 g, filtro ~21 Hz e taxa de 1 kHz.
      mpu.setAccelRange(16);
      mpu.setGyroRange(500);
      mpu.setGyroLPF(21);  // No MPU6050, o filtro também atua no acelerômetro
      mpu.setAccelLPF(21); // No MPU6500, o acelerômetro tem filtro próprio
      mpu.setAccelODR(1000);
    }

    ACCEL medirAccel() {
      AccelData leitura;
      mpu.update();
      mpu.getAccel(&leitura);  // A API da FastIMU exige o endereço da variável

      // FastIMU entrega em g. O JSON continua em m/s², como antes.
      ACCEL dados;
      dados.x = leitura.accelX * 9.80665f;
      dados.y = leitura.accelY * 9.80665f;
      dados.z = leitura.accelZ * 9.80665f;
      return dados;
    }
  }
}
```

### App06 — aquisição de distância

Antes:

```cpp
float distancia = sensorDist.ping_cm();
if (distancia == 0) distancia = 400;
```

Depois, o header recebe os pinos em `inicializar()` e devolve uma struct em `medirDistancia()`. A conversão compatível e a publicação aparecem no novo loop reproduzido acima. Header completo:

Arquivo: `app06_mqtt_publisher_distancia/src/ESP32SensorsDistancia.hpp`.

```cpp
#pragma once
#include <Arduino.h>

namespace ESP32Sensors {
  namespace Distancia {

    struct DISTANCIA {
      float cm;
      uint32_t duracaoUs;
      bool valido;
    };

    static uint8_t trigPin = 0;
    static uint8_t echoPin = 0;
    static uint32_t tempoLimite = 30000;

    void inicializar(uint8_t tPin, uint8_t ePin, uint32_t limiteUs = 30000) {
      trigPin = tPin;
      echoPin = ePin;
      tempoLimite = limiteUs;
      pinMode(trigPin, OUTPUT);
      pinMode(echoPin, INPUT);
    }

    DISTANCIA medirDistancia() {
      // Etapa 1: garante que o TRIG esteja em nível baixo.
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);

      // Etapa 2: dispara o sensor com um pulso de 10 microssegundos.
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);

      // Etapa 3: mede o tempo do ECHO. Sem pulso, pulseIn devolve zero.
      uint32_t duracao = pulseIn(echoPin, HIGH, tempoLimite);

      // Etapa 4: calcula a distância. Dividimos por 2 por causa da ida e volta.
      DISTANCIA dados;
      dados.cm = duracao * 0.034f / 2;
      dados.duracaoUs = duracao;
      dados.valido = duracao > 0;
      return dados;
    }
  }
}
```

## Problemas fora do escopo — somente registrados

- O servidor Python de app06 assina tópicos diferentes dos firmwares app06/app07.
- O arquivo histórico FlixPeriph do app04 não é compilado e contém GPIO de DHT diferente do exemplo ativo; o README informa qual fonte seguir.
- Os demais achados da Fase 1 continuam registrados naquela entrega: calibração que reaplica defaults, perfis de filtros diferentes nas varinhas, tópico do script atacante e sensores não utilizados nos diagramas de áudio.

## Referências das APIs

- [FastIMU 1.3.0, MPU6050](https://github.com/LiquidCGS/FastIMU/blob/1.3.0/src/sensors/F_MPU6050.hpp)
- [FastIMU 1.3.0, MPU6500](https://github.com/LiquidCGS/FastIMU/blob/1.3.0/src/sensors/F_MPU6500.hpp)
- [FastIMU: unidades e implementação](https://github.com/LiquidCGS/FastIMU/blob/1.3.0/src/sensors/F_MPU6050.cpp)
- [DHT 1.4.6](https://github.com/adafruit/DHT-sensor-library/blob/1.4.6/DHT.h)
- [pulseIn](https://docs.arduino.cc/language-reference/en/functions/advanced-io/pulseIn/)
- [NewPing 1.9.7](https://dl.registry.platformio.org/download/teckel12/library/NewPing/1.9.7/NewPing-1.9.7.tar.gz)
- [PubSubClient 2.8](https://github.com/knolleary/pubsubclient/blob/v2.8/src/PubSubClient.h)
- [PubSubClient: conexão síncrona](https://github.com/knolleary/pubsubclient/blob/v2.8/src/PubSubClient.cpp)
- [WiFi no core 2.0.17](https://github.com/espressif/arduino-esp32/blob/2.0.17/libraries/WiFi/src/WiFiSTA.h)
