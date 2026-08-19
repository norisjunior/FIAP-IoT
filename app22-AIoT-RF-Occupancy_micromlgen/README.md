# Physical Computing, Embedded AI, Robotics & Cognitive IoT

# App22 - AIoT: Random Forest Embarcado com micromlgen

Evolução do **app19**: mesma ideia do app21 (inferência local no ESP32), mas usando a biblioteca **micromlgen** em vez de m2cgen. A diferença principal é que micromlgen retorna diretamente a **classe predita**, sem probabilidades.

## O que muda do app19 para o app22

| app19 | app22 |
|-------|-------|
| ESP32 publica JSON via MQTT | ESP32 classifica localmente, sem rede |
| Serviço Python externo executa o modelo | Modelo Random Forest embarcado no `.ino` |
| Requer Wi-Fi, broker MQTT e servidor | Funciona offline, sem dependências externas |

## app21 (m2cgen) vs app22 (micromlgen)

| | app21 | app22 |
|-|-------|-------|
| **Biblioteca** | m2cgen | micromlgen |
| **Retorno da inferência** | `probabilidades[0]`, `probabilidades[1]` | classe direta (`0` ou `1`) |
| **Chamada** | `RandomForest::score(input, prob)` | `modeloRF.predict(input)` |
| **Instância** | não precisa | `Eloquent::ML::Port::RandomForest modeloRF` |

## Passo a passo

### 1. Gerar os arquivos do modelo (Google Colab)

Siga o notebook de aula:
[https://colab.research.google.com/drive/1fEBFePpqnozpdJ1Gk3MvBMTESLUD02AI?usp=sharing](https://colab.research.google.com/drive/1fEBFePpqnozpdJ1Gk3MvBMTESLUD02AI?usp=sharing)

O Colab vai gerar dois arquivos — faça o download de ambos:
- `AIoTRandomForest_micromlgen.hpp` — modelo Random Forest em C++ (Eloquent)
- `AIoTOccupancyRFScaler.hpp` — parâmetros do StandardScaler

### 2. Copiar os arquivos gerados para o projeto

Cole os dois arquivos na pasta `src/`:

```
app22-AIoT-RF-Occupancy_micromlgen/
└── src/
    ├── iot-app22_AIoT_micromlgen.ino
    ├── AIoTRandomForest_micromlgen.hpp  ← gerado no Colab
    ├── AIoTOccupancyRFScaler.hpp        ← gerado no Colab
    ├── ESP32SensorsAmbiente.hpp
    ├── ESP32SensorsLDR.hpp
    ├── ESP32SensorsCO2.hpp
    ├── ESP32SensorsLED.hpp
    └── ESP32SensorsHumidityRatio.hpp
```

### 3. Atualizar o platformio.ini

Remova a dependência do `PubSubClient` (não há mais MQTT):

```ini
[env:esp32]
platform = espressif32
framework = arduino
board = esp32dev

lib_deps =
    adafruit/DHT sensor library @ ^1.4.6
    bblanchon/ArduinoJson @ ^7.4.1
```

### 4. Atualizar o .ino

Faça as alterações abaixo no arquivo `.ino` do app19, bloco a bloco.

---

**Bloco 1 — INCLUDES:** remova os de Wi-Fi/MQTT e adicione os dois headers gerados.

```cpp
/* ==== INCLUDES ===================================================== */
// REMOVIDO: #include <WiFi.h>
// REMOVIDO: #include <WiFiClient.h>
// REMOVIDO: #include <PubSubClient.h>
// REMOVIDO: #include <ArduinoJson.h>
#include "ESP32SensorsAmbiente.hpp"
#include "ESP32SensorsLED.hpp"
#include "ESP32SensorsLDR.hpp"
#include "ESP32SensorsCO2.hpp"
#include "ESP32SensorsHumidityRatio.hpp"
// ADICIONADO: headers gerados pelo Colab
#include "AIoTOccupancyRFScaler.hpp"          // StandardScaler
#include "AIoTRandomForest_micromlgen.hpp"    // Modelo Random Forest micromlgen
```

---

**Bloco 2 — Constantes e variáveis globais:** remova tudo de Wi-Fi/MQTT e adicione a instância do modelo e as estatísticas.

```cpp
/* ==== Configurações de Hardware =================================================== */
const uint8_t DHT_PIN   = 26;
const uint8_t DHT_MODEL = DHT22;  // app19 usava DHT11 — atualizar para DHT22
const uint8_t LED_PIN   = 21;
const uint8_t LDR_PIN   = 35;
const uint8_t CO2_PIN   = 34;
const uint8_t HR_PIN    = 32;

// REMOVIDO: constantes e variáveis de Wi-Fi e MQTT

/* ==== CONSTANTES =================================== */
static unsigned long lastMs = 0;
const unsigned long INTERVAL = 5000; // 5s entre inferências

// ADICIONADO: estatísticas acumuladas
struct ModelStats {
  unsigned long totalInferencias = 0;
  unsigned long salaOcupada = 0;
  unsigned long salaVazia = 0;
} stats;

// ADICIONADO: instância do modelo micromlgen (Eloquent)
Eloquent::ML::Port::RandomForest modeloRF;
```

---

**Bloco 3 — Função de inferência:** substitua `buildAndPublishJSON()` por esta função. O fluxo é: coletar → normalizar → inferir localmente → acionar LED.

```cpp
/* ==== FUNÇÃO PRINCIPAL: COLETA + INFERÊNCIA LOCAL =================================== */
// SUBSTITUIU: buildAndPublishJSON() do app19
bool coletaDados_e_realizaInferencia() {
  // 1. Coleta de dados (igual ao app19)
  ESP32Sensors::Ambiente::AMBIENTE  amb = ESP32Sensors::Ambiente::medirAmbiente();
  ESP32Sensors::LDR::DADOS_LDR      luz = ESP32Sensors::LDR::ler();
  ESP32Sensors::CO2::DADOS_CO2      co2 = ESP32Sensors::CO2::ler();
  ESP32Sensors::HR::DADOS_HR        hr  = ESP32Sensors::HR::ler();

  if (!amb.valido || !luz.valido || !co2.valido) {
    Serial.println("[ERRO] Leituras inválidas dos sensores!");
    return false;
  }

  Serial.println("\n========== INFERÊNCIA LOCAL ML ==========");
  Serial.printf("  Temperature: %.2f °C\n",  amb.temp);
  Serial.printf("  Humidity: %.2f %%\n",     amb.umid);
  Serial.printf("  Light: %.1f lux\n",       luz.lux);
  Serial.printf("  CO2: %.1f ppm\n",         co2.ppm);
  Serial.printf("  HumidityRatio: %.6f\n",   hr.valor);

  // 2. ADICIONADO: normalização com StandardScaler gerado no Colab
  float dadosBrutos[5] = { amb.temp, amb.umid, luz.lux, co2.ppm, hr.valor };
  float dadosPadronizados[5];
  Scaler::std(dadosBrutos, dadosPadronizados);

  // 3. ADICIONADO: inferência local com micromlgen
  //    predict() recebe float* e retorna a classe diretamente (0=vazia, 1=ocupada)
  int predicao = modeloRF.predict(dadosPadronizados);

  const char* statusSala = (predicao == 1) ? "OCUPADA" : "VAZIA";

  stats.totalInferencias++;
  if (predicao == 1) { stats.salaOcupada++; ESP32Sensors::LED::on(); }
  else               { stats.salaVazia++;   ESP32Sensors::LED::off(); }

  Serial.println("\n[RESULTADO ML]");
  Serial.printf("  --> Sala: %s\n", statusSala);
  // DIFERENÇA do app21: micromlgen não retorna probabilidades, só a classe
  Serial.printf("  --> Classe predita: %d\n", predicao);
  Serial.printf("  --> LED: %s\n\n", (predicao == 1) ? "LIGADO" : "DESLIGADO");

  Serial.printf("[STATS] Total: %lu | Ocupada: %lu (%.1f%%) | Vazia: %lu (%.1f%%)\n",
                stats.totalInferencias,
                stats.salaOcupada, (float)stats.salaOcupada/stats.totalInferencias * 100,
                stats.salaVazia,   (float)stats.salaVazia  /stats.totalInferencias * 100);
  Serial.println("==========================================\n");

  return true;
}
```

---

**Bloco 4 — setup() e loop():** remova as chamadas de Wi-Fi/MQTT.

```cpp
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("ESP32 - AIoT - ML EMBARCADO (100% LOCAL)");
  Serial.println("Modelo: Random Forest | Biblioteca: MICROMLGEN");

  // REMOVIDO: conectarWiFi() e conectarMQTT()

  ESP32Sensors::Ambiente::inicializar(DHT_PIN, DHT_MODEL);
  ESP32Sensors::LED::inicializar(LED_PIN);
  ESP32Sensors::LDR::inicializar(LDR_PIN);
  ESP32Sensors::CO2::inicializar(CO2_PIN);
  ESP32Sensors::HR::inicializar(HR_PIN);
  Serial.println("Sensores inicializados. Sistema pronto.\n");

  delay(2000);
}

void loop() {
  // REMOVIDO: verificação e manutenção da conexão MQTT

  unsigned long now = millis();
  if (now - lastMs >= INTERVAL) {
    coletaDados_e_realizaInferencia();
    lastMs = now;
  }
  delay(100);
}
```

### 5. Compilar e executar no Wokwi

Compile e observe o Serial Monitor. A cada 5 segundos:

```
========== INFERÊNCIA LOCAL ML ==========
  Temperature: 24.50 °C
  Humidity: 45.20 %
  Light: 320.5 lux
  CO2: 680.3 ppm
  HumidityRatio: 0.004521

[RESULTADO ML]
  --> Sala: OCUPADA
  --> Classe predita: 1
  --> LED: LIGADO

[STATS] Total: 1 | Ocupada: 1 (100.0%) | Vazia: 0 (0.0%)
==========================================
```

Ajuste os sliders do **LDR** e do **potenciômetro (CO2)** para ver a classificação mudar.

## Troubleshooting

**Erro `'Eloquent' was not declared`:** o arquivo `AIoTRandomForest_micromlgen.hpp` não foi gerado pelo micromlgen ou está incompleto — deve conter `namespace Eloquent { ... }`.

**Erro de compilação no `.hpp` do modelo:** reduza `n_estimators` (tente 10–20) ou `max_depth` (tente 5–10) no Colab e regere os arquivos.

**Classificação sempre igual:** verifique se os parâmetros `means` e `scales` no `AIoTOccupancyRFScaler.hpp` batem com os gerados pelo Colab — não misture scalers de execuções diferentes.
