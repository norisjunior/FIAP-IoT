# Physical Computing, Embedded AI, Robotics & Cognitive IoT

# App21 - AIoT: Random Forest Embarcado com m2cgen

Evolução do **app19**: a inferência de ocupação que antes era feita em um serviço externo via MQTT agora roda **100% dentro do ESP32**, sem Wi-Fi e sem cloud.

## O que muda do app19 para o app21

| app19 | app21 |
|-------|-------|
| ESP32 coleta sensores e publica JSON via MQTT | ESP32 coleta, normaliza e classifica localmente |
| Serviço Python externo executa o modelo ML | Modelo Random Forest embarcado no `.ino` |
| Requer Wi-Fi, broker MQTT e servidor rodando | Funciona offline, sem dependências externas |

## Passo a passo

### 1. Gerar os arquivos do modelo (Google Colab)

Siga o notebook de aula:
[https://colab.research.google.com/drive/1fEBFePpqnozpdJ1Gk3MvBMTESLUD02AI?usp=sharing](https://colab.research.google.com/drive/1fEBFePpqnozpdJ1Gk3MvBMTESLUD02AI?usp=sharing)

O Colab vai gerar dois arquivos — faça o download de ambos:
- `AIoTRandomForest_mc2gen.hpp` — modelo Random Forest em C++
- `AIoTOccupancyRFScaler.hpp` — parâmetros do StandardScaler

### 2. Copiar os arquivos gerados para o projeto

Cole os dois arquivos na pasta `src/`:

```
app21-AIoT-RF-Occupancy_mc2gen/
└── src/
    ├── iot-app21_AIoT_mc2gen.ino
    ├── AIoTRandomForest_mc2gen.hpp   ← gerado no Colab
    ├── AIoTOccupancyRFScaler.hpp     ← gerado no Colab
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

Substitua o conteúdo do `.ino` pelo código abaixo. Os comentários indicam o que foi removido do app19 e o que foi adicionado.

```cpp
/* ==== INCLUDES ===================================================== */
// REMOVIDO do app19: #include <WiFi.h>
// REMOVIDO do app19: #include <WiFiClient.h>
// REMOVIDO do app19: #include <PubSubClient.h>
// REMOVIDO do app19: #include <ArduinoJson.h>
#include "ESP32SensorsAmbiente.hpp"
#include "ESP32SensorsLED.hpp"
#include "ESP32SensorsLDR.hpp"
#include "ESP32SensorsCO2.hpp"
#include "ESP32SensorsHumidityRatio.hpp"
// ADICIONADO: headers gerados pelo Colab
#include "AIoTOccupancyRFScaler.hpp"     // StandardScaler para normalização
#include "AIoTRandomForest_mc2gen.hpp"   // Modelo Random Forest mc2gen

/* ==== Configurações de Hardware =================================================== */
const uint8_t DHT_PIN   = 26;
const uint8_t DHT_MODEL = DHT22;
const uint8_t LED_PIN   = 21;
const uint8_t LDR_PIN   = 35;
const uint8_t CO2_PIN   = 34;
const uint8_t HR_PIN    = 32;

// REMOVIDO do app19: constantes e variáveis de Wi-Fi e MQTT

/* ==== CONSTANTES =================================== */
static unsigned long lastMs = 0;
const unsigned long INTERVAL = 5000; // 5s entre inferências (app19 usava 15s)

/* ==== ESTATÍSTICAS DO MODELO =================================== */
// ADICIONADO: struct para acumular estatísticas de inferência
struct ModelStats {
  unsigned long totalInferencias = 0;
  unsigned long salaOcupada = 0;
  unsigned long salaVazia = 0;
} stats;


/* ==== FUNÇÃO PRINCIPAL: COLETA + INFERÊNCIA LOCAL =================================== */
// SUBSTITUIU: buildAndPublishJSON() do app19 que enviava dados via MQTT
bool coletaDados_e_realizaInferencia() {
  // 1. COLETA DE DADOS DOS SENSORES (igual ao app19)
  ESP32Sensors::Ambiente::AMBIENTE  amb = ESP32Sensors::Ambiente::medirAmbiente();
  ESP32Sensors::LDR::DADOS_LDR      luz = ESP32Sensors::LDR::ler();
  ESP32Sensors::CO2::DADOS_CO2      co2 = ESP32Sensors::CO2::ler();
  ESP32Sensors::HR::DADOS_HR        hr  = ESP32Sensors::HR::ler();

  if (!amb.valido || !luz.valido || !co2.valido) {
    Serial.println("[ERRO] Leituras inválidas dos sensores!");
    return false;
  }

  // 2. EXIBIR DADOS BRUTOS
  Serial.println("\n========== INFERÊNCIA LOCAL ML ==========");
  Serial.println("[SENSORES] Dados coletados:");
  Serial.printf("  Temperature: %.2f °C", amb.temp); Serial.println("");
  Serial.printf("  Humidity: %.2f %%", amb.umid); Serial.println("");
  Serial.printf("  Light: %.1f lux", luz.lux); Serial.println("");
  Serial.printf("  CO2: %.1f ppm", co2.ppm); Serial.println("");
  Serial.printf("  HumidityRatio: %.6f", hr.valor); Serial.println("");

  // 3. PREPARAR ARRAY COM OS DADOS BRUTOS
  float dadosBrutos[5] = {
    amb.temp,    // Temperature
    amb.umid,    // Humidity
    luz.lux,     // Light
    co2.ppm,     // CO2
    hr.valor     // HumidityRatio
  };

  // 4. ADICIONADO: NORMALIZAÇÃO com StandardScaler gerado no Colab
  // (no app19 os dados iam crus para o serviço externo)
  float dadosPadronizados[5];
  Scaler::std(dadosBrutos, dadosPadronizados);

  Serial.println("\n[NORMALIZAÇÃO] Dados processados:");
  Serial.printf("  Temp: %.2f | Hum: %.2f | Light: %.2f | CO2: %.3f | HR: %.3f",
                dadosPadronizados[0], dadosPadronizados[1],
                dadosPadronizados[2], dadosPadronizados[3],
                dadosPadronizados[4]); Serial.println("");

  // 5. ADICIONADO: INFERÊNCIA LOCAL com Random Forest gerado no Colab
  // (no app19 este passo era feito pelo serviço Python externo)
  double inputModelo[5];
  for(int i = 0; i < 5; i++) {
    inputModelo[i] = (double)dadosPadronizados[i];
  }

  double probabilidades[2];
  RandomForest::score(inputModelo, probabilidades);
  // probabilidades[0] = P(vazia), probabilidades[1] = P(ocupada)

  // 6. DECISÃO: maior probabilidade define a classe
  int predicao = (probabilidades[1] > probabilidades[0]) ? 1 : 0;
  const char* statusSala = (predicao == 1) ? "OCUPADA" : "VAZIA";

  // 7. ATUALIZAR ESTATÍSTICAS E ACIONAR LED
  stats.totalInferencias++;
  if (predicao == 1) {
    stats.salaOcupada++;
    ESP32Sensors::LED::on();   // LED acende: sala ocupada
  } else {
    stats.salaVazia++;
    ESP32Sensors::LED::off();  // LED apaga: sala vazia
  }

  // 8. EXIBIR RESULTADO
  Serial.println("\n[RESULTADO ML]");
  Serial.printf("  --> Sala: %s", statusSala); Serial.println("");
  Serial.printf("  --> Probabilidades: Vazia=%.1f%% | Ocupada=%.1f%%",
                probabilidades[0] * 100, probabilidades[1] * 100); Serial.println("");
  Serial.printf("  --> LED: %s", (predicao == 1) ? "LIGADO" : "DESLIGADO"); Serial.println("\n");

  Serial.printf("[STATS] Total: %lu | Ocupada: %lu (%.1f%%) | Vazia: %lu (%.1f%%)\n",
                stats.totalInferencias,
                stats.salaOcupada, (float)stats.salaOcupada/stats.totalInferencias * 100,
                stats.salaVazia,   (float)stats.salaVazia/stats.totalInferencias   * 100);
  Serial.println("==========================================\n");

  return true;
}

/* ==== SETUP / LOOP ================================================ */
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("ESP32 - AIoT - ML EMBARCADO (100% LOCAL)");
  Serial.println("Modelo: Random Forest | Biblioteca: MC2GEN");

  // REMOVIDO do app19: conectarWiFi() e conectarMQTT()

  ESP32Sensors::Ambiente::inicializar(DHT_PIN, DHT_MODEL);
  ESP32Sensors::LED::inicializar(LED_PIN);
  ESP32Sensors::LDR::inicializar(LDR_PIN);
  ESP32Sensors::CO2::inicializar(CO2_PIN);
  ESP32Sensors::HR::inicializar(HR_PIN);
  Serial.println("Sensores inicializados. Sistema pronto.\n");

  delay(2000);
}

void loop() {
  // REMOVIDO do app19: verificação e manutenção da conexão MQTT

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
[SENSORES] Temperature: 24.50 °C | Humidity: 45.20 % | ...

[RESULTADO ML]
  --> Sala: OCUPADA
  --> Probabilidades: Vazia=15.0% | Ocupada=85.0%
  --> LED: LIGADO
==========================================
```

Ajuste os sliders do **LDR** e do **potenciômetro (CO2)** para ver a classificação mudar.

## Troubleshooting

**Erro de compilação no `.hpp` do modelo:** reduza `n_estimators` (tente 10–20) ou `max_depth` (tente 5–10) no Colab e regere os arquivos.

**Classificação sempre igual:** verifique se os parâmetros `means` e `scales` no `AIoTOccupancyRFScaler.hpp` batem com os gerados pelo Colab — não misture scalers de execuções diferentes.
