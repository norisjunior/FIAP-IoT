# Construir o firmware da torre, a partir do da aula passada

O ponto de partida é o firmware que decide e publica, da aula passada
(`app20-Inferencia-Borda/device-MQTT-devkit/src/validacao-device.cpp`). Tudo o que ele já faz
continua igual: a leitura a 100 Hz, a janela de 1 s, as 8 features, o scaler, os LEDs, o
buzzer e a publicação da janela em `FIAPIoT/motor/borda/<nome>`. Por isso a nuvem continua
conferindo, com os mesmos fluxos.

Mudam duas coisas:

1. **o modelo:** sai a floresta (`if`/`else`), entra a rede neural (TensorFlow Lite);
2. **a ordem para a bomba:** a classe vira uma ordem para outra máquina, o pivô.

---

## Mudança 1 — A rede no lugar da floresta

### A biblioteca

No `platformio.ini`, uma linha a mais: o interpretador do TFLite para microcontroladores.

```ini
lib_deps =
    https://github.com/LiquidCGS/FastIMU.git#1.3.0
    knolleary/PubSubClient @ ^2.8
    bblanchon/ArduinoJson @ ^7.4.1
    johnosbb/MicroTFLite @ ^1.0.4
```

A primeira compilação demora uns 5 minutos: são centenas de arquivos do TFLite.

### Os dois arquivos do modelo

Em `src/`, os dois `.hpp` do `.zip` do notebook:

- `ModeloMotorScaler.hpp`: o mesmo formato da aula passada (8 médias, 8 desvios);
- `ModeloMotorRede.hpp`: no lugar do `ModeloMotorRF.hpp`. São os bytes do `.tflite`, um
  vetor chamado `modeloMotorRede`.

Apague o `ModeloMotorRF.hpp`: com os dois no `src/`, nada quebra, mas a floresta fica
ocupando flash sem ninguém usar.

### O topo do arquivo

Saem o `#include "ModeloMotorRF.hpp"` e a linha `Eloquent::ML::Port::RandomForest modeloRF;`.
Entram:

```cpp
#include "ModeloMotorScaler.hpp"   // Scaler::standardize()
#include "ModeloMotorRede.hpp"     // a rede neural, em bytes (formato TFLite)
#include <MicroTFLite.h>           // o interpretador TFLite da placa

/* ---- A memória de trabalho da rede (tensor arena) ---- */
#define ARENA_SIZE (8 * 1024)
uint8_t tensorArena[ARENA_SIZE];
```

A **arena** é a RAM onde o interpretador guarda as contas da rede enquanto ela roda: a
entrada, os 16 neurônios do meio e a saída. 8 KB sobram para esta rede; o Serial mostra quanto
ela usou de fato.

`NOMES_CLASSES[4]` não muda: a rede devolve 4 probabilidades na mesma ordem alfabética da
floresta (0 = anomalia, 1 = inclinado_frente, 2 = inclinado_tras, 3 = operando). O notebook
imprime a linha pronta.

### No `setup()`, carregar a rede

Depois do `Serial.println("MPU iniciado");`:

```cpp
  // A rede neural: carrega o modelo na memória de trabalho
  if (!ModelInit(modeloMotorRede, tensorArena, ARENA_SIZE)) {
    Serial.println("Erro: a rede neural nao carregou");
    while (1);
  }
  ModelPrintMetadata();   // mostra quanto da arena a rede usou
```

### A inferência

Na `classificarJanela()`, a linha `int classe = modeloRF.predict(padronizado);` vira três
passos: as 8 entradas, a rede roda, e fica a classe com a maior probabilidade.

```cpp
int classificarJanela(const float features[8]) {
  float padronizado[8];
  float probabilidade[4];

  uint32_t t0 = micros();
  Scaler::standardize(features, padronizado);
  for (int i = 0; i < 8; i++) {
    ModelSetInput(padronizado[i], i);
  }
  if (!ModelRunInference()) {
    Serial.println("Erro: a inferencia falhou");
    return -1;
  }
  int classe = 0;
  for (int c = 0; c < 4; c++) {
    probabilidade[c] = ModelGetOutput(c);
    if (probabilidade[c] > probabilidade[classe]) classe = c;
  }
  uint32_t duracao = micros() - t0;
  ...
  Serial.printf("  PREDIÇÃO:  %d -> %s (%.0f%%)\r\n",
                classe, NOMES_CLASSES[classe], probabilidade[classe] * 100);
  Serial.printf("  Inferencia: %lu us\r\n", duracao);
```

A diferença para a floresta: a floresta devolve **votos** e a mais votada; a rede devolve
uma **probabilidade** por classe (as quatro somam 100%), e a placa fica com a maior.

**Confira:** compile e grave. No começo, o Serial mostra `Arena Used Bytes`; depois, uma
classe por segundo, com a probabilidade e o tempo da inferência. Os LEDs seguem o motor, como
na aula passada.

---

## Mudança 2 — A ordem para a bomba

O pivô só irriga com a torre operando. Inclinada (atolada, desalinhada) ou vibrando fora do
normal, a bomba tem que parar.

### O tópico

Logo abaixo do `MQTT_PUB_TOPIC`:

```cpp
#define MQTT_TOPIC_BOMBA "FIAPIoT/pivo/" MQTT_CLIENT_ID "/cmd/local"    // a ordem para a bomba do pivô
```

`local` é a origem da ordem: a placa. A nuvem publica em `cmd/cloud`, a pessoa em
`cmd/human`, e a bomba assina os três.

### A ordem anterior

Junto das outras variáveis globais:

```cpp
/* ---- A ordem para a bomba ---- */
int ordemAnterior = -1;            // -1 = ainda não mandou; 0 = desligar; 1 = ligar
uint32_t ultimaOrdem = 0;
const uint32_t REPETIR_ORDEM_MS = 30000;   // repete a mesma ordem a cada 30 s
```

### A função

```cpp
/* ---- A ordem para a bomba: só a torre operando irriga ---- */
void mandarNaBomba(int classe) {
  if (classe < 0) return;                    // a inferência falhou: não manda nada
  int ordem = (classe == 3) ? 1 : 0;         // 3 = operando

  // Só fala com a bomba quando muda de ideia (e repete a cada 30 s)
  if (ordem == ordemAnterior && millis() - ultimaOrdem < REPETIR_ORDEM_MS) return;

  JsonDocument doc;
  doc["dispositivo"] = MQTT_CLIENT_ID;
  doc["bomba"]       = (ordem == 1);
  doc["motivo"]      = NOMES_CLASSES[classe];

  String buffer;
  serializeJson(doc, buffer);

  if (mqttClient.publish(MQTT_TOPIC_BOMBA, buffer.c_str())) {
    if (ordem != ordemAnterior) {
      Serial.printf(">> Ordem para a bomba: %s (%s)\r\n",
                    ordem == 1 ? "LIGAR" : "DESLIGAR", NOMES_CLASSES[classe]);
    }
    ordemAnterior = ordem;
    ultimaOrdem   = millis();
  }
}
```

**Por que repetir a cada 30 s.** A bomba guarda a ordem da placa por 90 s. Se a placa
desligar ou sumir da rede, a ordem vence e a bomba para sozinha: sem notícia da torre, não se
irriga. E uma bomba que acabou de ligar recebe a ordem em até 30 s.

**Por que só quando muda.** Uma ordem por segundo, com a mesma decisão, só ocuparia o broker.

Declare a função junto dos outros protótipos:

```cpp
void mandarNaBomba(int classe);
```

### No `loop()`

No fecho da janela, depois de publicar:

```cpp
      int classe = classificarJanela(features);
      acionarSaida(classe);
      publicarJanela(features, classe);
      mandarNaBomba(classe);
```

A ordem continua a mesma da aula passada: decidir e agir primeiro, falar com a rede depois.

**Confira**, sem bomba nenhuma:

```bash
mosquitto_sub -h <broker> -t "FIAPIoT/pivo/#" -v
```

Passe o motor de `operando` para uma inclinação e de volta: uma linha `"bomba":false` e depois
uma `"bomba":true`, com o motivo.

---

## O que não muda

- `MQTT_CLIENT_ID`: o nome da placa, o mesmo do dataset e dos gatilhos do n8n.
- O JSON em `FIAPIoT/motor/borda/<nome>`, com a `predicao_borda`: a nuvem confere a rede da
  placa com a rede da API, nos mesmos fluxos da aula passada.
- O Wi-Fi por até 10 s e o MQTT com uma tentativa a cada 5 s: sem rede, a torre continua
  decidindo e acendendo; só a ordem para a bomba e a conferência ficam para depois.

## Wokwi

Ative o bloco `(A)` no topo do `.cpp`, troque `#define MPU_TYPE` para `MPU6050` e comente
`mpu.calibrateAccelGyro(&calib);`. A inclinação sai pelo controle do MPU6050; a `anomalia`,
que é vibração, não sai (com o controle parado, as 100 amostras são iguais).
