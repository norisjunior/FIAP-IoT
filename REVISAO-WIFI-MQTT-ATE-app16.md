# Revisão de WiFi e MQTT até o app16

Aplicação exclusiva em `C:\Projects\FIAP-IoT-eval`. Apps17 em diante não foram alterados nesta etapa.

## Padrão aplicado

- WiFi com canal automático: `WiFi.begin(WIFI_SSID, WIFI_SENHA);`.
- No loop: se WiFi não estiver conectado, chamar `conectarWiFi()`.
- Mensagem de conexão e IP dentro de `conectarWiFi()`, somente depois de conectar.
- Para MQTT: com WiFi conectado, chamar `conectarMQTT()` quando necessário; conectado, chamar `mqttClient.loop()`.
- Assinatura explícita somente nos assinantes: apps07, 08 e 15. O `subscribe` fica dentro do `if` de conexão bem-sucedida e se repete após reconectar.
- Controle de intervalo com `uint32_t`; intervalo de 5 s do app14, mantendo o padrão aprovado nos pilotos.
- PubSubClient 2.8, `setKeepAlive(120)` e `setSocketTimeout(2)`.
- O modelo WiFi mantém os TODOs de MQTT para a atividade.

A função WiFi aguarda a rede em um `while` com `delay(500)`. Enquanto espera, a coleta e o atendimento HTTP pausam. O controle de 5 s limita a entrada na função; não interrompe essa espera. O IP é mostrado a cada conexão iniciada pela função.

MQTT faz uma tentativa por intervalo, mas `connect()` é síncrono. O timeout de 2 s trata a resposta MQTT, não toda a conexão TCP. Não há armazenamento ou reenvio offline.

Apps03, 05, 12 e 13 aguardam o intervalo inicial e conectam antes de iniciar o serviço. Apps14 e 15 mantêm a espera inicial que já tinham. Os pilotos e o modelo usam o controle no loop para a primeira tentativa.

## Escopo e compilação

| App | Resultado |
|---|---|
| 01, 02, 09, 10 e 11 | Sem alteração de WiFi/MQTT |
| 03 | WiFi atualizado; compilação SUCCESS |
| 04 | Piloto preservado; alternativa Flix no .txt alinhada; firmware ativo compilou SUCCESS |
| 05 | WiFi atualizado; compilação SUCCESS |
| 06 | Publicador piloto preservado; compilação já validada na revisão piloto |
| 07 | Assinante piloto preservado; compilação já validada na revisão piloto |
| 08 | WiFi/MQTT atualizados, subscribe explícito; compilação SUCCESS |
| 12 | WiFi atualizado, reconexão Firebase existente mantida; compilação SUCCESS |
| 13 | WiFi atualizado; compilação SUCCESS |
| 14 | WiFi/MQTT atualizados, somente publicação; compilação SUCCESS |
| 15 | WiFi/MQTT atualizados, subscribe explícito; compilação SUCCESS |
| 16 | Orientação atualizada; usa o firmware do app15, sem projeto de firmware próprio |
| app_WiFi_modelo | WiFi atualizado, exercício MQTT preservado; compilação SUCCESS |

Foram compilados nove firmwares nesta etapa: apps03, 04, 05, 08, 12, 13, 14, 15 e modelo WiFi. Não foi feita gravação nem execução em placa/simulação.

## Identificação MQTT

`MQTT_CLIENT_ID` identifica a conexão: use o RA, app e dispositivo, por exemplo `a12345678-app14-01`. Substitua o RA de exemplo e mantenha IDs diferentes quando houver mais de um dispositivo.

Nos apps14/15, `DISPOSITIVO_ID = "NexoLogEquipe01"` preserva o valor do campo `device` no JSON. Tópicos, campos, pinos, intervalos de coleta e callbacks foram preservados. No app08, o broker foi alinhado a `host.wokwi.internal`, como nos pilotos; na placa física, configure o endereço do seu broker.

## Documentação

READMEs dos projetos alterados e roteiros de construção dos apps14/15 foram alinhados. O app16 agora usa o nome `MQTT_BROKER` para configurar o Raspberry. O [guia MQTT do modelo](app_WiFi_modelo/README-MQTT-Passo-a-passo.md) mostra o subscribe diretamente no assinante.

## Trechos para revisar e atualizar os slides

As linhas abaixo correspondem aos arquivos atuais. Os trechos “antes” são do código versionado em HEAD, anterior aos pilotos; “depois” corresponde ao código final.

### app04

Arquivo: [app04_ESP_transmite_Server/app04_transmite_JSON/src/iot-aula02-app04.ino](app04_ESP_transmite_Server/app04_transmite_JSON/src/iot-aula02-app04.ino)

Antes: conectarWiFi não existia com esse nome; o controle estava no setup/loop.

**Depois — conectarWiFi, linha 37**

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

**Antes — loop, linha 47**

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

**Depois — loop, linha 69**

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

### app06

Arquivo: [app06_mqtt_publisher_distancia/src/app06-publisher-distancia.ino](app06_mqtt_publisher_distancia/src/app06-publisher-distancia.ino)

**Antes — conectarWiFi, linha 31**

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

**Depois — conectarWiFi, linha 35**

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

**Antes — conectarMQTT, linha 44**

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

**Depois — conectarMQTT, linha 56**

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

**Antes — loop, linha 67**

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

**Depois — loop, linha 79**

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

Arquivo: [app07_mqtt_subscriber_atuador/src/app07-subscriber-atuador.ino](app07_mqtt_subscriber_atuador/src/app07-subscriber-atuador.ino)

**Antes — conectarWiFi, linha 27**

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

**Depois — conectarWiFi, linha 31**

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

**Antes — conectarMQTT, linha 48**

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

**Depois — conectarMQTT, linha 60**

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

**Antes — loop, linha 73**

```cpp
void loop() {
  if (!mqtt.connected()) {
    conectarMQTT();
  }

  mqtt.loop();  // recebe as mensagens do broker
  delay(100);
}
```

**Depois — loop, linha 85**

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

### app08

Arquivo: [app08_mqtt_subscriber_atuador-LED_e_BUZ/src/app07-subscriber-atuador.ino](app08_mqtt_subscriber_atuador-LED_e_BUZ/src/app07-subscriber-atuador.ino)

**Antes — conectarWiFi, linha 29**

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

**Depois — conectarWiFi, linha 36**

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

**Antes — conectarMQTT, linha 62**

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

**Depois — conectarMQTT, linha 77**

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

**Antes — loop, linha 92**

```cpp
void loop() {
  if (!mqtt.connected()) {
    conectarMQTT();
  }

  mqtt.loop();  // recebe as mensagens do broker
  delay(100);
}
```

**Depois — loop, linha 107**

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

## Itens fora desta alteração

- A alternativa FlixPeripheral do app04 é um arquivo .txt inativo; não foi compilada como firmware nem migrada de biblioteca.
- Bibliotecas e leitura dos sensores dos demais apps permanecem como estavam. A migração de sensores dos pilotos está documentada no [relatório anterior](REVISAO-PILOTO-app04-app06-app07.md).
- O app12 mantém o intervalo de coleta de 30000 ms; o comentário antigo que cita 10 segundos não foi usado para mudar o comportamento.
