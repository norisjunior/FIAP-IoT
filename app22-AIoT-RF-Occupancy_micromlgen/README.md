# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 22 - AIoT: Random Forest Embarcado com micromlgen

Esta aplicação demonstra **Inteligência Artificial de Coisas (AIoT)** com Machine Learning 100% embarcado no ESP32. Um modelo Random Forest para detecção de ocupação é treinado no Google Colab, exportado usando **micromlgen** (Eloquent Arduino library), e executado localmente no microcontrolador **sem conectividade** (offline).

## Conceito: TinyML e Edge AI

**TinyML** é a técnica de executar modelos de Machine Learning diretamente em dispositivos com recursos limitados (microcontroladores), permitindo:

- ✅ **Inferência local**: Decisões em tempo real sem latência de rede
- ✅ **Privacidade**: Dados não saem do dispositivo
- ✅ **Autonomia**: Funciona offline, sem dependência de cloud
- ✅ **Baixo consumo**: Ideal para dispositivos IoT alimentados por bateria
- ✅ **Custo reduzido**: Sem custos de transmissão de dados ou APIs cloud

## micromlgen vs m2cgen

Esta aplicação usa **micromlgen** (biblioteca Eloquent para Arduino):

**micromlgen:**
- Biblioteca Python que converte modelos scikit-learn para C++ Arduino
- Gera classes C++ com método `predict()` que retorna a **classe predita**
- Otimizado especificamente para microcontroladores
- Foco em simplicidade e uso eficiente de memória
- Repositório: [eloquentarduino/micromlgen](https://github.com/eloquentarduino/micromlgen)

**Comparação com m2cgen** (usado no app21):
- micromlgen retorna apenas a **classe**, m2cgen retorna **probabilidades**
- micromlgen é específico para Arduino/microcontroladores
- m2cgen suporta mais linguagens de destino (C, Python, Java, etc.)
- micromlgen tem sintaxe mais orientada a objetos

## Sensores e Atuadores

**Sensores:**
- DHT22 (Temperatura e Umidade) - Pino GPIO 26
- LDR (Sensor de Luminosidade) - Pino GPIO 35
- Potenciômetro (Simulador de CO2) - Pino GPIO 34

**Atuadores:**
- LED Vermelho - Pino GPIO 21 (Acende quando sala está ocupada)

**Features do Modelo:**
- Temperature (°C)
- Humidity (%)
- Light (Lux)
- CO2 (ppm)
- HumidityRatio (calculado)

## Funcionamento

### Pipeline Completo

```
┌─────────────────────┐
│  Google Colab       │
│  ────────────       │
│  1. Carregar dataset│
│  2. Treinar Random  │
│     Forest (sklearn)│
│  3. Exportar com    │
│     micromlgen      │
│  4. Gerar .hpp      │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│  AIoTRandomForest   │
│  _micromlgen.hpp    │  ◄── Código C++ gerado
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│     ESP32           │
│  ─────────          │
│  1. Coletar dados   │
│     dos sensores    │
│  2. Normalizar      │
│     (StandardScaler)│
│  3. Inferência local│
│     (Random Forest) │
│  4. Classificação:  │
│     • Vazia (0)     │
│     • Ocupada (1)   │
│  5. Controlar LED   │
└─────────────────────┘
```

### Processo de Inferência

**1. Coleta de Dados:**
```
Temperature: 24.5 °C
Humidity: 45.2 %
Light: 320.5 lux
CO2: 680.3 ppm
HumidityRatio: 0.004521
```

**2. Normalização (StandardScaler):**
```
Dados padronizados para mean=0, std=1
```

**3. Inferência (Random Forest micromlgen):**
```cpp
Eloquent::ML::Port::RandomForest modeloRF;
int predicao = modeloRF.predict(dadosPadronizados);
// predicao = 1 (OCUPADA)
```

**4. Decisão:**
```
Classe predita: 1 (OCUPADA)
LED: LIGADO
```

**5. Estatísticas Acumuladas:**
```
Total: 120 inferências
Ocupada: 85 vezes (70.8%)
Vazia: 35 vezes (29.2%)
```

## Como Usar

### 1. Treinar e Exportar o Modelo (Google Colab)

> **Importante:** O treinamento é feito no Google Colab apresentado em sala de aula.

**Passo a passo no Colab:**

```python
# 1. Instalar micromlgen
!pip install micromlgen

# 2. Treinar Random Forest
from sklearn.ensemble import RandomForestClassifier
from sklearn.preprocessing import StandardScaler

# Carregar dataset de ocupação
# Features: Temperature, Humidity, Light, CO2, HumidityRatio
# Target: Occupancy (0=vazia, 1=ocupada)

scaler = StandardScaler()
X_scaled = scaler.fit_transform(X)

rf_model = RandomForestClassifier(
    n_estimators=20,  # Reduzir para caber na memória do ESP32
    max_depth=10,
    random_state=42
)
rf_model.fit(X_scaled, y)

# 3. Exportar com micromlgen
from micromlgen import port

# Gerar código C++ para Arduino
codigo_cpp = port(rf_model)

# Salvar como .hpp
with open('AIoTRandomForest_micromlgen.hpp', 'w') as f:
    f.write('#pragma once\n\n')
    f.write(codigo_cpp)

# 4. Exportar StandardScaler
print("Mean:", scaler.mean_)
print("Scale:", scaler.scale_)
# Copiar valores para AIoTOccupancyRFScaler.hpp
```

**Resultado:** Dois arquivos gerados:
- `AIoTRandomForest_micromlgen.hpp` - Modelo Random Forest em C++
- `AIoTOccupancyRFScaler.hpp` - Parâmetros do StandardScaler

### 2. Preparar o Código ESP32

Os arquivos gerados no Colab devem substituir:
- `src/AIoTRandomForest_micromlgen.hpp`
- `src/AIoTOccupancyRFScaler.hpp`

**Estrutura do projeto:**
```
app22-AIoT-RF-Occupancy_micromlgen/
├── src/
│   ├── iot-app22_AIoT_micromlgen.ino   # Código principal
│   ├── AIoTRandomForest_micromlgen.hpp # Modelo (gerado no Colab)
│   ├── AIoTOccupancyRFScaler.hpp       # Scaler (gerado no Colab)
│   ├── ESP32Sensors.hpp                # Biblioteca de sensores
│   ├── ESP32SensorsAmbiente.hpp        # DHT22
│   ├── ESP32SensorsLDR.hpp             # LDR
│   ├── ESP32SensorsCO2.hpp             # Potenciômetro (CO2)
│   └── ESP32SensorsLED.hpp             # LED
├── diagram.json                        # Diagrama Wokwi
└── README.md
```

### 3. Executar no Wokwi

1. **Abrir projeto no Wokwi:**
   - Arquivo: `app22-AIoT-RF-Occupancy_micromlgen/src/iot-app22_AIoT_micromlgen.ino`

2. **Compilar e executar:**
   - O ESP32 iniciará e executará inferências a cada 5 segundos

3. **Monitorar Serial Monitor:**

```
==========================================
ESP32 - AIoT - ML EMBARCADO (100% LOCAL)
Modelo: Random Forest - Ocupação de Sala
Modo: SEM CONECTIVIDADE (offline)
Biblioteca: MICROMLGEN
==========================================
Sensores inicializados!
Sistema pronto para inferências locais.

========== INFERÊNCIA LOCAL ML ==========
[SENSORES] Dados coletados:
  Temperature: 24.50 °C
  Humidity: 45.20 %
  Light: 320.5 lux
  CO2: 680.3 ppm
  HumidityRatio: 0.004521

[NORMALIZAÇÃO] Dados processados:
  Temp: 0.45 | Hum: -0.32 | Light: 1.23 | CO2: -0.87 | HR: 0.12

[RESULTADO ML]
  --> Sala: OCUPADA
  --> Classe predita: 1
  --> LED: LIGADO

[STATS] Total: 1 | Ocupada: 1 (100.0%) | Vazia: 0 (0.0%)
==========================================
```

**Observação:** Diferente do app21 (m2cgen), o micromlgen **não retorna probabilidades**, apenas a classe predita (0 ou 1).

4. **Ajustar sensores:**
   - Use o slider do **LDR** para simular luminosidade
   - Use o slider do **Potenciômetro** para simular CO2
   - DHT22 (temperatura/umidade) varia automaticamente no Wokwi
   - Observe o LED acender/apagar conforme a classificação

## Estrutura do Código

### Arquivo Principal (iot-app22_AIoT_micromlgen.ino)

```cpp
// DECLARAÇÃO: Criar instância do modelo
Eloquent::ML::Port::RandomForest modeloRF;

// 1. COLETA DE DADOS
ESP32Sensors::Ambiente::AMBIENTE amb = ESP32Sensors::Ambiente::medirAmbiente();
ESP32Sensors::LDR::DADOS_LDR luz = ESP32Sensors::LDR::ler();
ESP32Sensors::CO2::DADOS_CO2 co2 = ESP32Sensors::CO2::ler();

// 2. NORMALIZAÇÃO
float dadosBrutos[5] = {amb.temp, amb.umid, luz.lux, co2.ppm, hr};
float dadosPadronizados[5];
Scaler::std(dadosBrutos, dadosPadronizados);

// 3. INFERÊNCIA (retorna diretamente a classe)
int predicao = modeloRF.predict(dadosPadronizados);

// 4. ATUAÇÃO
if (predicao == 1) ESP32Sensors::LED::on();
else ESP32Sensors::LED::off();
```

## Vantagens do micromlgen

✅ **Simplicidade**: API orientada a objetos, fácil de usar
✅ **Otimizado para Arduino**: Desenvolvido especificamente para microcontroladores
✅ **Memória eficiente**: Código compacto e otimizado
✅ **Documentação Arduino**: Exemplos focados em makers
✅ **Ativo**: Comunidade Eloquent Arduino

## Diferenças: App22 (micromlgen) vs App21 (m2cgen)

| Aspecto | App22 (micromlgen) | App21 (m2cgen) |
|---------|---------------------|----------------|
| **Biblioteca** | micromlgen | m2cgen |
| **Método de inferência** | `modeloRF.predict()` | `RandomForest::score()` |
| **Retorno** | Classe direta (0 ou 1) | Probabilidades [0.15, 0.85] |
| **Arquivo gerado** | AIoTRandomForest_micromlgen.hpp | AIoTRandomForest_mc2gen.hpp |
| **Informação disponível** | Apenas decisão final | Confiança da predição |
| **Sintaxe** | Orientada a objetos | Funções namespace |
| **Uso de memória** | Semelhante | Semelhante |
| **Performance** | Semelhante | Semelhante |
| **Documentação** | Focada em Arduino | Mais genérica |

**Quando usar micromlgen (app22):**
- Quando precisar apenas da decisão final (classe)
- Quando preferir sintaxe orientada a objetos

**Quando usar m2cgen (app21):**
- Quando precisar das probabilidades (confiança)
- Quando quiser código mais "bare metal" (namespace)
- Quando precisar portar para outras plataformas além de Arduino

## Casos de Uso

**1. Automação Residencial:**
- Ligar/desligar luzes e ar-condicionado baseado em ocupação
- Economia de energia com decisões locais instantâneas

**2. Edifícios Inteligentes:**
- Gestão de climatização por ambiente
- Redução de custos operacionais

**3. Segurança:**
- Detecção de presença não autorizada
- Alertas em tempo real sem dependência de internet

**4. Indústria 4.0:**
- Monitoramento de estações de trabalho
- Otimização de layout fabril

## Troubleshooting

**Modelo não compila:**
- Verificar se arquivo .hpp foi gerado corretamente no Colab
- Reduzir `n_estimators` do Random Forest (tentar 10-20 árvores)
- Reduzir `max_depth` (tentar 5-10)

**Erro: 'Eloquent' não foi declarado:**
- Verificar se o código gerado pelo micromlgen foi copiado corretamente
- O arquivo .hpp deve conter `namespace Eloquent { ... }`

**Inferências inconsistentes:**
- Verificar se StandardScaler foi exportado corretamente
- Conferir valores de `mean_` e `scale_` no Scaler.hpp
- Validar leituras dos sensores (valores devem estar no range esperado)

**LED não acende:**
- Verificar conexão do LED no GPIO 21
- Testar manualmente: `ESP32Sensors::LED::on();`
- Verificar se predição está retornando 1

**Memória insuficiente:**
- Reduzir número de árvores no Random Forest
- Reduzir profundidade máxima
- Simplificar modelo (menos features)

## Próximos Passos

1. **Comparar com App21**: Execute também o app21 (m2cgen) e compare as diferenças
