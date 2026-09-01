# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 25 - 2 - Detecção de Gestos em Tempo Real com ESP32 e Edge Impulse

Sistema de reconhecimento de gestos usando TinyML no ESP32 (usando a plataforma Edge Impulse). Detecta 4 gestos diferentes em tempo real: cima-baixo, círculo, lateral-parabaixo e descanso-parabaixo.

## Hardware Necessário

- ESP32 DevKit
- MPU6050 (acelerômetro e giroscópio)
- 3 LEDs (vermelho, verde, azul)

## Conexões

**MPU6050:**
- VCC → 3V3
- GND → GND
- SDA → GPIO 22
- SCL → GPIO 23
- AD0 → GND (endereço I2C 0x68)

**LEDs:**
- LED Vermelho → GPIO 21
- LED Verde → GPIO 19
- LED Azul → GPIO 18

## Instalação da Biblioteca Edge Impulse

1. **Crie a pasta `lib/` na raiz do projeto**
2. **Baixe sua biblioteca do Edge Impulse** (Arduino library em .zip)
3. **Extraia dentro de `lib/`** - deve ficar: `lib/MagicWand_ESP32_inferencing/`

Estrutura correta:
```
app25-2-VarinhaMLDetectaMov-ESP32/
├── lib/
│   └── MagicWand_ESP32_inferencing/
│       ├── src/
│       ├── library.properties
│       └── ...
├── src/
│   ├── ESP32Sensors.hpp
│   ├── ESP32SensorsAccelGyro.hpp
│   └── iot-varinha-ML-app25-coleta.ino
└── platformio.ini
```

## Conceitos Fundamentais do Código

### 1. Inclusão da Biblioteca

```cpp
#include <MagicWand_ESP32_inferencing.h>
```

Esta biblioteca foi gerada pelo Edge Impulse e contém:
- Modelo treinado (rede neural)
- Funções de pré-processamento (FFT)
- Classificador de gestos

**ATENÇÂO:** MagicWand_ESP32 foi o nome do projeto criado pelo professor. Caso seu projeto no Edge Impulse tenha outro nome, será gerado outro nome de biblioteca. USE O NOME CORRETO!

### 2. Frequência de Amostragem

```cpp
#define FREQUENCY_HZ        50
#define INTERVAL_MS         (1000 / FREQUENCY_HZ)
```

- **50 Hz** = 50 amostras por segundo
- **20 ms** entre cada leitura do sensor
- Deve ser igual à frequência usada no treinamento

### 3. Buffer de Amostras

```cpp
static float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
```

**Tamanho do buffer:**
- Janela: 1200ms (configurada no Edge Impulse)
- Frequência: 50 Hz
- Amostras necessárias: 1200ms ÷ 20ms = **60 amostras**
- Eixos: 6 (ax, ay, az, gx, gy, gz)
- **Total: 60 × 6 = 360 valores**

### 4. Preenchimento do Buffer

```cpp
// A cada 20ms, adiciona 6 valores ao buffer
if (feature_ix + 6 <= EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
    features[feature_ix++] = dados.accel.acceleration.x;  // ax
    features[feature_ix++] = dados.accel.acceleration.y;  // ay
    features[feature_ix++] = dados.accel.acceleration.z;  // az
    features[feature_ix++] = dados.gyro.gyro.x;           // gx
    features[feature_ix++] = dados.gyro.gyro.y;           // gy
    features[feature_ix++] = dados.gyro.gyro.z;           // gz
}
```

**Processo:**
```
Tempo 0ms:    amostras [0-5]    → primeiros 6 valores
Tempo 20ms:   amostras [6-11]   → próximos 6 valores
Tempo 40ms:   amostras [12-17]  → próximos 6 valores
...
Tempo 1180ms: amostras [354-359] → últimos 6 valores
Tempo 1200ms: Buffer completo (360 valores) → INFERÊNCIA
```

### 5. Transformada de Fourier (FFT)

```cpp
signal_t signal;
numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
```

A função `signal_from_buffer()` prepara o buffer para processamento. Internamente, o Edge Impulse:
- Aplica **FFT de 32 pontos**
- Extrai características espectrais
- Normaliza os dados

### 6. Inferência (Classificação)

```cpp
ei_impulse_result_t result;
EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
```

A função `run_classifier()`:
- Processa o sinal pela rede neural
- Retorna probabilidades para cada gesto
- Armazena tempos de processamento

### 7. Análise dos Resultados

```cpp
// Probabilidades de cada classe
for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    float prob = result.classification[ix].value;      // Probabilidade (0.0 a 1.0)
    const char *label = result.classification[ix].label; // Nome do gesto
    ei_printf("  %s: %.3f\n", label, prob);
}
```

**Exemplo de saída:**
```
  cima-baixo: 0.023
  circulo: 0.891
  lateral-parabaixo: 0.056
  descanso-parabaixo: 0.030
```

### 8. Melhor Classe

```cpp
// Encontrar gesto com maior probabilidade
float best_val = 0.0f;
size_t best_ix = 0;

for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    if (result.classification[ix].value > best_val) {
        best_val = result.classification[ix].value;
        best_ix = ix;
    }
}

const char *gesto = result.classification[best_ix].label;
```

### 9. Acionamento dos LEDs

```cpp
// Threshold de 60% para evitar falsos positivos
if (best_val >= 0.6f) {
    if (strcmp(gesto, "cima-baixo") == 0) {
        led_cima_baixo();      // LED vermelho
    } else if (strcmp(gesto, "circulo") == 0) {
        led_circulo();         // LED verde
    } else if (strcmp(gesto, "lateral-parabaixo") == 0) {
        led_lateral_parabaixo(); // LED azul
    } else if (strcmp(gesto, "descanso-parabaixo") == 0) {
        led_descanso_parabaixo(); // LEDs branco (todos)
    }
}
```

## Como Funciona (Resumo)

1. **Coleta contínua** a 50Hz (20ms entre amostras)
2. **Buffer acumula 60 amostras** (1200ms de dados)
3. **FFT extrai características** espectrais
4. **Rede neural classifica** o gesto
5. **LED acende** se probabilidade > 60%
6. **Buffer reseta** e processo recomeça

## Compilação e Upload

```bash
pio run --target upload
pio device monitor
```

## Saída no Monitor Serial

```
=== ESP32 + MPU6050 + Edge Impulse ===
Pronto para detectar gestos!
Tamanho do buffer: 360

Timing -> DSP: 45 ms, Class: 12 ms
  cima-baixo: 0.023
  circulo: 0.891
  lateral-parabaixo: 0.056
  descanso-parabaixo: 0.030
=> Gesto detectado: circulo (89.10%)
```

## Parâmetros do Modelo

- **Janela:** 1200ms
- **Stride:** 200ms (não usado em inferência contínua)
- **FFT:** 32 pontos
- **Threshold:** 60% de confiança

## Simulação

Compatível com Wokwi Simulator - use o arquivo `diagram.json` incluído.

## Bibliotecas

- Adafruit MPU6050
- Adafruit Unified Sensor
- Adafruit BusIO
- Edge Impulse SDK (em `lib/`)
