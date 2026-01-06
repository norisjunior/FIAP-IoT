# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 26 - Device 1 - Varinha Mágica com MQTT para Controle de Atuadores

Sistema de reconhecimento de gestos usando TinyML no ESP32 (Edge Impulse) que envia comandos MQTT para controlar atuadores remotos. Detecta 4 gestos diferentes em tempo real e aciona atuadores via protocolo MQTT:
- **Círculo**: Liga bomba d'água (atuador 3342)
- **Cima-baixo**: Desliga bomba d'água (atuador 3342)
- **Lateral-parabaixo**: Liga irrigação (atuador 3338)
- **Descanso-parabaixo**: Desliga irrigação (atuador 3338)

## Hardware Necessário

- ESP32 DevKit
- MPU6050 (acelerômetro e giroscópio)
- 3 LEDs (vermelho, verde, azul) - feedback visual
- Broker MQTT (ex: Mosquitto, HiveMQ)

## Conexões

**MPU6050:**
- VCC → 3V3
- GND → GND
- SDA → GPIO 22
- SCL → GPIO 23
- AD0 → GND (endereço I2C 0x68)

**LEDs (Feedback Visual):**
- LED Vermelho → GPIO 21 (gesto cima-baixo)
- LED Verde → GPIO 19 (gesto círculo)
- LED Azul → GPIO 18 (gesto lateral-parabaixo)

## Configuração do payload

**Formato do payload JSON:**
```json
{
  "deviceId": "FIAPIoTapp26Dev1",
  "atuador": "3342",
  "comando": true
}
```

O atuador segue os parâmetros do IPSO Smart Objects:
- 3342: On/Off Switch (para ligar/desligar a bomba)
- 3338: Buzzer (liga/desliga)

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

## Estrutura do Código

### Fluxo de execução

```
┌─────────────────┐
│  Inicialização  │
│  (WiFi, MQTT,   │
│   Sensores)     │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Loop: 20ms     │◄───────┐
│  Ler MPU6050    │        │
│  (ax,ay,az,     │        │
│   gx,gy,gz)     │        │
└────────┬────────┘        │
         │                 │
         │ Adiciona ao     │
         │ buffer          │
         ▼                 │
┌─────────────────┐        │
│ Buffer completo?│──Não───┤
│   (360 valores) │        │
└────────┬────────┘        │
         │Sim              │
         ▼                 │
┌─────────────────┐        │
│   FFT + ML      │        │
│   Classifica    │        │
└────────┬────────┘        │
         │                 │
         ▼                 │
┌─────────────────┐        │
│ Confiança > 60%?│──Não───┤
└────────┬────────┘        │
         │Sim              │
         ▼                 │
┌─────────────────┐        │
│  Aciona LED +   │        │
│  Envia MQTT     │        │
└────────┬────────┘        │
         │                 │
         └─────────────────┘
```

### Função ```EdgeAI_fft()```
    Realiza o pré-processamento e obtém a predição da classe (classificação do gesto)

    Esta função encapsula o processamento da biblioteca Edge Impulse:
- `signal_from_buffer()`: Prepara os dados do buffer
- `run_classifier()`: Executa FFT + classificação neural
- `result`: Armazena as probabilidades de cada gesto

### Função ```EdgeAI_predict()```
    Encontra Melhor Classe


### Função enviarComando()
    Envio dos dados via MQTT

**Formato do payload publicado:**
```json
{
  "deviceId": "FIAPIoTapp26Dev1",
  "atuador": "3342",
  "comando": true
}
```

## Como Funciona (Resumo)

1. **Setup**: Conecta ao WiFi, MQTT broker e inicializa sensor MPU6050
2. **Loop a 50Hz**: A cada 20ms, lê acelerômetro e giroscópio (6 valores)
3. **Acumula dados**: Coleta 60 amostras = 1200ms de movimento (360 valores no buffer)
4. **Classificação**: Quando buffer completo, executa FFT + rede neural
5. **Seleção**: Encontra gesto com maior probabilidade (reseta variáveis entre inferências)
6. **Validação**: Apenas gestos com confiança ≥ 60% são aceitos
7. **Ação**: Acende LED correspondente + publica comando MQTT
8. **Reset**: Buffer é zerado e ciclo recomeça


## Mapeamento Gestos → Atuadores

| Gesto | LED | Atuador | Comando | Ação |
|-------|-----|---------|---------|------|
| Círculo | Verde | 3342 | `true` | Liga bomba d'água |
| Cima-baixo | Vermelho | 3342 | `false` | Desliga bomba d'água |
| Lateral-parabaixo | Azul | 3338 | `true` | Liga buzzer |
| Descanso-parabaixo | Branco | 3338 | `false` | Desliga buzzer |


## Bibliotecas

- Adafruit MPU6050
- Adafruit Unified Sensor
- Adafruit BusIO
- PubSubClient (MQTT)
- ArduinoJson
- WiFi (ESP32)
- Edge Impulse SDK (em `lib/`)

## Arquitetura do Sistema

```
┌─────────────────────────┐
│   ESP32 + MPU6050       │
│   (Device 1 - Varinha)  │
│                         │
│  1. Detecta Gesto       │
│  2. Classifica (ML)     │
│  3. Publica MQTT        │
└───────────┬─────────────┘
            │
            │ MQTT Publish
            │ Topic: FIAPIoT/smartagro/cmd/local
            ▼
┌─────────────────────────┐
│     Broker MQTT         │
│   (host.wokwi.internal) │
└───────────┬─────────────┘
            │
            │ MQTT Subscribe
            ▼
┌─────────────────────────┐
│   ESP32 Atuadores       │
│   (Device 2)            │
│                         │
│  - Bomba (3342)         │
│  - Irrigação (3338)     │
└─────────────────────────┘
```
