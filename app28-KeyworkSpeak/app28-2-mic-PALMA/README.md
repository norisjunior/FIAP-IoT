# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 24 - Device 1 - AIoT: Irrigação Inteligente com TensorFlow Lite

Esta aplicação demonstra **Inteligência Artificial de Coisas (AIoT)** com Machine Learning 100% embarcado no ESP32. Um modelo de rede neural treinado em TensorFlow é compilado para TFLite, exportado e executado localmente no microcontrolador para tomar decisões autônomas de irrigação baseadas em sensores ambientais.

## Conceito: TinyML e EdgeAI para Agricultura Inteligente

**TinyML** é a técnica de executar modelos de Machine Learning diretamente em dispositivos com recursos limitados (microcontroladores), permitindo:

- ✅ **Automação Agrícola**: Decisões sobre irrigação em tempo real
- ✅ **Privacidade**: Dados de sensores não saem do dispositivo
- ✅ **Autonomia**: Funciona offline e pode continuar operando sem conectividade
- ✅ **Baixo consumo**: Ideal para sistemas alimentados por bateria ou energia solar
- ✅ **Custo reduzido**: Sem custos de chamadas de API ou processamento em cloud

## TensorFlow Lite vs m2cgen

Esta aplicação usa **TensorFlow Lite (TFLite)**:

**TFLite:**
- Framework oficial do Google para ML em dispositivos embarcados
- Modelos compilados e otimizados em binário (.tflite)
- Suporta diversos tipos de redes neurais: Dense, Conv, LSTM, etc.
- Integração com bibliotecas como ArduTFLite para ESP32
- Documentação extensa e comunidade grande
- Repositório: [TensorFlow Lite](https://www.tensorflow.org/lite)

**Comparação com m2cgen** (usado no app21):
- TFLite usa modelos compilados e binários, m2cgen gera código C puro
- TFLite suporta redes neurais profundas, m2cgen é melhor para algoritmos clássicos
- TFLite requer interpretador, m2cgen não precisa
- TFLite possui melhor performance para modelos complexos

## Sensores e Atuadores

**Sensores:**
- DHT22 (Temperatura e Umidade) - Pino GPIO 26
- Sensor de Dryness/Umidade do Solo (Potenciômetro) - Pino GPIO 34

**Atuadores:**
- LED Vermelho - Pino GPIO 21 (Acende quando a bomba de irrigação deve estar ligada)

**Features do Modelo:**
- Dryness: Nível de secura do solo (0-1023, mapeado pelo ADC)
- Temperature: Temperatura ambiente (°C)

**Saída do Modelo:**
- Probabilidade de irrigação (0.0 a 1.0)
  - Se prob >= 0.5: **BOMBA LIGADA** (LED aceso)
  - Se prob < 0.5: **BOMBA DESLIGADA** (LED apagado)

## Funcionamento

### Pipeline Completo

```
┌──────────────────────┐
│   Google Colab       │
│   ──────────────     │
│  1. Carregar dataset │
│  2. Criar rede       │
│     neural (TF)      │
│  3. Treinar modelo   │
│  4. Converter para   │
│     TFLite           │
│  5. Gerar .tflite    │
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│ modelo_irrig_        │
│ inteligente.h        │  ◄── Modelo compilado
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│      ESP32           │
│   ──────────────     │
│  1. Ler sensores     │
│     (dryness, temp)  │
│  2. Executar modelo  │
│     TFLite           │
│  3. Receber saída:   │
│     probabilidade    │
│  4. Decisão:         │
│     • Não irriga (0) │
│     • Irriga (1)     │
│  5. Controlar LED    │
│     (bomba)          │
│  6. Enviar via MQTT  │
└──────────────────────┘
```

### Processo de Inferência

**1. Coleta de Dados:**
```
Dryness: 450 (solo com 44% de umidade)
Temperature: 28.5 °C
```

**2. Normalização (aplicada no modelo TFLite):**
```
Valores normalizados conforme treinamento
```

**3. Inferência (Rede Neural TFLite):**
```cpp
modelSetInput(dry.valor, 0);    // Input 0: dryness
modelSetInput(amb.temp, 1);     // Input 1: temperatura
modelRunInference();
float probabilidade = modelGetOutput(0);  // Output: 0 a 1
```

**4. Decisão:**
```
Se probabilidade >= 0.5:
  Classe predita: 1 (IRRIGAR)
  LED: LIGADO
  Ação: Ativa bomba d'água
Senão:
  Classe predita: 0 (NÃO IRRIGAR)
  LED: DESLIGADO
  Ação: Desativa bomba
```

**5. Comunicação:**
```json
Publicado em: FIAPIoT/smartagro/cmd/local
{
  "dispositivo": "FIAPIoTapp24Dev1",
  "bomba": true
}
```

## Como Usar

### 1. Treinar e Exportar o Modelo (Google Colab)

> **Importante:** O treinamento e exportação do modelo é feito no Google Colab (conforme apresentado em sala de aula).

**Processo:**
- Carregar dataset de umidade do solo x irrigação
- Criar rede neural (Dense + Dropout para evitar overfitting)
- Treinar com épocas suficientes
- Converter para TFLite com `tf.lite.TFLiteConverter`
- Exportar como `.tflite` e converter em array C (`.h`)

**Resultado:** Arquivo gerado:
- `modelo_irrig_inteligente.h` - Modelo compilado em C++ (binário do TFLite)

### 2. Preparar o Código ESP32

O arquivo gerado no Colab deve ser colocado em:
- `src/modelo_irrig_inteligente.h`

**Estrutura do projeto:**
```
app24-IrrigInteligente/Device1-TinyML/
├── src/
│   ├── iot-app24_TinyML-TFLITE.ino           # Código principal
│   ├── modelo_irrig_inteligente.h            # Modelo TFLite (gerado no Colab)
│   ├── ESP32Sensors.hpp                      # Biblioteca agregadora de sensores
│   ├── ESP32SensorsAmbiente.hpp              # DHT22 (temperatura/umidade)
│   ├── ESP32SensorsDryness.hpp               # Sensor de umidade do solo
│   └── ESP32SensorsLED.hpp                   # LED indicador/atuador
├── platformio.ini                            # Configuração PlatformIO
├── diagram.json                              # Diagrama Wokwi
├── wokwi.toml                                # Configuração Wokwi
└── README.md
```

### 3. Dependências (PlatformIO)

```ini
lib_deps =
    adafruit/DHT sensor library @ ^1.4.6
    bblanchon/ArduinoJson @ ^7.4.1
    spaziochirale/ArduTFLite @ ^1.0.2
    knolleary/PubSubClient @ ^2.8
```

- **DHT**: Leitura de temperatura/umidade
- **ArduinoJson**: Serialização JSON para MQTT
- **ArduTFLite**: Interpretador TFLite para ESP32
- **PubSubClient**: Cliente MQTT

### 4. Executar no Wokwi

1. **Abrir projeto no Wokwi:**
   - Carregar o arquivo `diagram.json`

2. **Compilar e executar:**
   - O ESP32 iniciará a coleta de dados imediatamente

3. **Monitorar Serial Monitor:**

```
==========================================
  ESP32 TinyML - IRRIGAÇÃO INTELIGENTE
==========================================

Sensores OK!
Carregando modelo TFLite...
Modelo TFLite carregado com sucesso!
Conectando ao Wi-Fi: Wokwi-GUEST
OK! IP: 192.168.x.x
[MQTT] Conectando ao broker host.wokwi.internal
 --> Conectado ao MQTT Broker!

>> Sistema pronto!

========== INFERÊNCIA TinyML ==========
Medição nº 1
Dryness: 450.0 | Temp: 28.5°C
Prob. Irrigar: 78.5% -> BOMBA->LIGADA
========================================

[MQTT] Publicado

========== INFERÊNCIA TinyML ==========
Medição nº 2
Dryness: 320.0 | Temp: 29.1°C
Prob. Irrigar: 92.3% -> BOMBA->LIGADA
========================================

[MQTT] Publicado
```

4. **Ajustar sensores:**
   - Use o slider do **Sensor de Dryness** para simular diferentes níveis de umidade do solo
   - DHT22 (temperatura/umidade) varia automaticamente no Wokwi
   - Observe o LED acender/apagar conforme a decisão do modelo
   - Verifique as mensagens MQTT no Mosquitto mqtt_sub

## Estrutura do Código

### Arquivo Principal (iot-app24_TinyML-TFLITE.ino)

```cpp
// 1. INICIALIZAR MODELO TFLITE
if (!modelInit(modelo_irrig_inteligente_tflite, tensorArena, ARENA_SIZE)) {
    Serial.println("ERRO: Falha ao carregar modelo!");
    while(1);  // Parar se falhar
}

// 2. COLETA DE DADOS DOS SENSORES
ESP32Sensors::Ambiente::AMBIENTE amb = ESP32Sensors::Ambiente::medirAmbiente();
ESP32Sensors::Dryness::DRY       dry = ESP32Sensors::Dryness::ler();

// 3. PREPARAR ENTRADAS PARA O MODELO
modelSetInput(dry.valor, 0);  // Input 0: dryness (0-1023)
modelSetInput(amb.temp, 1);   // Input 1: temperatura (°C)

// 4. EXECUTAR INFERÊNCIA
if (!modelRunInference()) {
    Serial.println("[ERRO] Falha na inferência!");
    return;
}

// 5. LER RESULTADO
float probabilidade = modelGetOutput(0);  // Probabilidade de irrigar (0.0 a 1.0)
deveIrrigar = (probabilidade >= 0.5f);    // Threshold em 50%

// 6. ATUAR NO LED (SIMULA A BOMBA)
if (deveIrrigar) {
    ESP32Sensors::LED::on();
} else {
    ESP32Sensors::LED::off();
}

// 7. ENVIAR DECISÃO VIA MQTT
JsonDocument doc;
doc["dispositivo"] = MQTT_DEVICEID;
doc["bomba"] = deveIrrigar;
String payload;
serializeJson(doc, payload);
mqttClient.publish(MQTT_PUB_TOPIC, payload.c_str());
```

### TFLite Model Functions

As funções do modelo são abstraídas pela biblioteca ArduTFLite:

```cpp
// Inicializar modelo com tensor arena
bool modelInit(const uint8_t* model_data, uint8_t* arena, size_t arena_size)

// Configurar valores de entrada
void modelSetInput(float value, uint8_t input_index)

// Executar a inferência
bool modelRunInference()

// Recuperar saída do modelo
float modelGetOutput(uint8_t output_index)
```

### Fluxo de Execução

```
┌─────────────────────────────┐
│        SETUP()              │
├─────────────────────────────┤
│ 1. Inicializar sensores     │
│ 2. Carregar modelo TFLite   │
│ 3. Conectar WiFi            │
│ 4. Conectar MQTT            │
└────────────┬────────────────┘
             │
             ▼
        ┌─────────────────────────────┐
        │        LOOP() (30s)         │
        ├─────────────────────────────┤
        │ 1. Ler dryness e temp       │
        │ 2. Chamar modelSetInput()   │
        │ 3. Chamar modelRunInference()
        │ 4. Recuperar probabilidade  │
        │ 5. Decidir: irrigar?        │
        │ 6. Ligar/desligar LED       │
        │ 7. Publicar via MQTT        │
        │ 8. Aguardar 30 segundos     │
        └─────────────────────────────┘
```

## Vantagens do TensorFlow Lite

✅ **Framework oficial**: Mantido pelo Google, suporte contínuo
✅ **Modelos complexos**: Suporta redes neurais profundas e convolucional
✅ **Framework flexível**: Importar de Keras, PyTorch, etc.
✅ **Otimização automática**: Quantização, prunning, distilação
✅ **Multiplataforma**: Funciona em Android, iOS, ESP32, Raspberry Pi, etc.
✅ **Performance**: Excelente para modelos não-triviais
✅ **Comunidade**: Documentação extensiva e grande comunidade de desenvolvedores

## Diferenças: App24 (TFLite) vs App21 (m2cgen)

| Aspecto | App24 (TFLite) | App21 (m2cgen) |
|---------|---|---|
| **Framework** | TensorFlow Lite | m2cgen |
| **Tipo de Modelo** | Redes neurais (Dense, Conv, LSTM) | Algoritmos clássicos (RF, SVM, DT) |
| **Formato** | Binário `.tflite` compilado | Código C++ puro |
| **Interpretador** | Requer runtime ArduTFLite | Não precisa (código direto) |
| **Tamanho do modelo** | Maior (binário otimizado) | Menor (código fonte) |
| **Tipo de saída** | Probabilidade contínua (0.0-1.0) | Probabilidades ou classe direta |
| **Complexidade suportada** | Alta (redes profundas) | Média (algoritmos clássicos) |
| **Performance** | Excelente para modelos complexos | Ótima para modelos simples |
| **Memoria**: | ~8KB tensor arena | Depende do modelo |
| **Caso de uso aqui** | Irrigation (2 inputs -> 1 output) | Ocupação de sala (5 inputs -> 2 outputs) |

## Casos de Uso

**1. Agricultura e Fazendas Inteligentes:**
- Irrigação autônoma em plantações
- Economia de água com decisões baseadas em NN
- Sistema totalmente offline, sem dependência de internet

**2. Jardinaria Doméstica e Comercial:**
- Vasos inteligentes que se irrigam automaticamente
- Múltiplos sensores para diferentes plantas
- Dados de decisões enviados via MQTT para logging

**3. Estufa Controlada:**
- Combinação de sensores para otimizar crescimento
- Modelo treinado com histórico da estufa específica
- Redução de perdas e aumento de produtividade

**4. IoT em Propriedades Rurais:**
- Monitoramento remoto via MQTT de múltiplos pontos de irrigação
- Consumo mínimo de energia (modelo rodar localmente)
- Autonomia total do dispositivo, funcionando offline

## Arquitetura de Comunicação

```
┌──────────────┐
│   ESP32      │
│   Device1    │
│  TinyML +    │
│   WiFi       │
│   MQTT       │
└────────┬─────┘
         │
         │  MQTT Publish
         │  (WiFi)
         ▼
┌──────────────────────┐
│  MQTT Broker         │
│  (host.wokwi.internal
│   port 1883)         │
└────────┬─────────────┘
         │
         │  MQTT Subscribe
         │
         ▼
┌──────────────────────┐
│  Outros Sistemas IoT │
│  (Dashboard, Logs,   │
│   Controle Remoto)   │
└──────────────────────┘
```

**Tópico MQTT:**
```
FIAPIoT/smartagro/cmd/local
```

**Payload:**
```json
{
  "dispositivo": "FIAPIoTapp24Dev1",
  "bomba": true
}
```

## Troubleshooting

**Inferências inconsistentes:**
- Verificar range esperado dos sensores (dryness 0-1023, temp 15-35°C)
- Certificar que o modelo foi treinado com os mesmos ranges
- Validar leituras dos sensores via Serial Monitor
- Verificar normalização/escala no Colab (aplicar mesma lógica no ESP32 se necessário)

**LED não acende:**
- Verificar conexão do LED no GPIO 21
- Testar manualmente: `ESP32Sensors::LED::on();`
- Verificar se probabilidade está acima do threshold (0.5)
- Confirmar polaridade do LED (cátodo no GND)

**WiFi/MQTT não conecta:**
- Verificar SSID e senha da rede (app24 usa "Wokwi-GUEST" no simulador)
- Confirmar que broker MQTT está rodando (host.wokwi.internal:1883)
- Aumentar timeout de reconexão se ambiente for instável
- Testar com `mqtt_sub` ou MQTT Explorer para verificar mensagens

