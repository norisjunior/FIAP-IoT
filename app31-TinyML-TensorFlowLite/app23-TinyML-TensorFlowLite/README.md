# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 23 - TinyML: Rede Neural com TensorFlow Lite no ESP32

Sistema de **irrigação inteligente** com rede neural embarcada no ESP32. O modelo TFLite decide se a bomba d'água deve ser ligada com base em dois sensores: **secura do solo** e **temperatura**.

---

## O que é TinyML com TensorFlow Lite

**TensorFlow Lite for Microcontrollers (TFLite Micro)** permite executar redes neurais diretamente no microcontrolador, sem nuvem, sem internet. O modelo é treinado no Colab, exportado como arquivo `.tflite`, convertido para um array C (`modelo.h`) e compilado junto com o firmware.

Diferente dos apps anteriores (m2cgen, micromlgen), aqui o modelo é uma **rede neural real** (camadas densas, ativação sigmoid).

---

## Sensores e Atuadores

| Componente | Pino | Função |
|---|---|---|
| DHT22 | GPIO 23 | Temperatura (°C) |
| Potenciômetro | GPIO 34 | Simula secura do solo (0–1023) |
| LED | GPIO 21 | Indica bomba ligada |
| Servo | GPIO 18 | Simula abertura da válvula |

---

## Pipeline TinyML

```
Google Colab                         ESP32
─────────────                        ─────
1. Treinar rede neural (Keras)
2. Exportar → modelo.tflite
3. Converter → modelo_irrig_inteligente.h  ──►  compilar junto com o firmware
                                                 │
                                                 ▼
                                          ModelInit()        ← carrega modelo na RAM
                                          ModelSetInput()    ← alimenta os sensores
                                          ModelRunInference() ← executa a rede neural
                                          ModelGetOutput()   ← lê a probabilidade
```

---

## Core do Código (o que importa ver)

```cpp
// 1. Carregar modelo na tensor arena (RAM reservada para o TFLite)
ModelInit(modelo_irrig_inteligente_tflite, tensorArena, ARENA_SIZE);

// 2. Alimentar entradas (mesma ordem do treino)
ModelSetInput(dry.valor, 0);  // secura do solo: 0–1023
ModelSetInput(amb.temp,  1);  // temperatura: °C

// 3. Executar inferência (a rede neural roda aqui)
ModelRunInference();

// 4. Ler saída: probabilidade de irrigar (0.0 a 1.0)
float probabilidade = ModelGetOutput(0);
bool deveIrrigar    = (probabilidade >= 0.5f);

// 5. Acionar atuadores conforme a decisão do modelo
if (deveIrrigar) { LED::on();  Servo::ligar();    }
else             { LED::off(); Servo::desligar(); }
```

---

## O momento-chave da tomada de decisão com ML

Este é o ponto que justifica usar ML em vez de if/else simples:

```
Secura do solo = 495 (valor intermediário)
  → Temp 25°C  →  BOMBA DESLIGADA  (solo não está tão seco, temperatura normal)
  → Temp 30°C  →  BOMBA LIGADA     (combinação de calor + secura exige irrigação)
```

Para valores extremos de secura, qualquer lógica simples acerta. Mas na zona cinzenta — quando **duas features combinadas** determinam a decisão - só o modelo treinado com dados reais sabe a resposta certa.

---

## Estrutura do Projeto

```
app23-TinyML-TensorFlowLite/
├── src/
│   ├── iot-app23_TinyML-TFLITE.ino       # código principal
│   ├── modelo_irrig_inteligente.h         # rede neural em C (gerada no Colab)
│   ├── ESP32SensorsAmbiente.hpp           # DHT22
│   ├── ESP32SensorsDryness.hpp            # potenciômetro (solo)
│   ├── ESP32SensorsLED.hpp                # LED
│   └── ESP32SensorsServo.hpp              # servo (válvula)
├── platformio.ini
└── README.md
```

**Biblioteca usada:** `johnosbb/MicroTFLite` — wrapper simplificado do TFLite Micro para Arduino/ESP32.


### Inicialização do Modelo

- Inclusão das bibliotecas:
```C
#include "modelo_irrig_inteligente.h"

// TensorFlow Lite via MicroTFLite
#include <MicroTFLite.h>
```
- RAM alocada para a aplicação IoT

```C
/* ==== TensorFlow Lite - Variáveis Globais ================================= */
#define ARENA_SIZE (8 * 1024)
uint8_t tensorArena[ARENA_SIZE];
```

- Inicialização do modelo
```C
    if (!ModelInit(modelo_irrig_inteligente_tflite, tensorArena, ARENA_SIZE)) {
      ...
    }

```

- Input do modelo (features - sensores)
```C
    ModelSetInput(dry.valor, 0);  // 0..1023 (já mapeado no .hpp)
    ModelSetInput(amb.temp, 1);   // °C
```

- Inferência
```C
    if (!ModelRunInference()) {
      ...
    }

    float probabilidade = ModelGetOutput(0);
    bool deveIrrigar    = (probabilidade >= 0.5f);
```

### Tomada de decisão baseada em ML embarcado (TinyML)
```C
    // 5. LIGAR OU NÃO LIGAR A BOMBA D'ÁGUA
    if (deveIrrigar) {
      ...
    } else {
      ...
    }
```



---

## Saída Serial esperada

```
==========================================
  ESP32 TinyML - IRRIGAÇÃO INTELIGENTE
==========================================

========== INFERÊNCIA TinyML ==========
Dryness: 495.0 | Temp: 30.0°C
Prob. Irrigar: 82.3% -> BOMBA->LIGADA
========================================
```

---

## Como executar no Wokwi

1. Abrir o projeto no Wokwi
2. Compilar e iniciar a simulação
3. Ajustar o **slider do potenciômetro** (secura do solo)
4. Observar como a mesma secura com temperaturas diferentes gera decisões opostas
