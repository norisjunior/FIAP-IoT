# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 21 - AIoT: Random Forest Embarcado com m2cgen

Esta aplicação demonstra **Inteligência Artificial de Coisas (AIoT)** com Machine Learning 100% embarcado no ESP32. Um modelo Random Forest para detecção de ocupação é treinado no Google Colab, exportado usando **m2cgen** (Model to Code Generator), e executado localmente no microcontrolador **sem conectividade** (offline).

## Conceito: TinyML e Edge AI

**TinyML** é a técnica de executar modelos de Machine Learning diretamente em dispositivos com recursos limitados (microcontroladores), permitindo:

- ✅ **Inferência local**: Decisões em tempo real sem latência de rede
- ✅ **Privacidade**: Dados não saem do dispositivo
- ✅ **Autonomia**: Funciona offline, sem dependência de cloud
- ✅ **Baixo consumo**: Ideal para dispositivos IoT alimentados por bateria
- ✅ **Custo reduzido**: Sem custos de transmissão de dados ou APIs cloud

## m2cgen vs micromlgen

Esta aplicação usa **m2cgen** (Model to Code Generator):

**m2cgen:**
- Converte modelos scikit-learn para código C/C++ puro
- Gera função `score()` que retorna **probabilidades** para cada classe
- Suporta múltiplos modelos: Random Forest, Decision Tree, SVM, etc.
- Código gerado é otimizado e legível
- Repositório: [BayesWitnesses/m2cgen](https://github.com/BayesWitnesses/m2cgen)

**Comparação com micromlgen** (usado no app22):
- m2cgen retorna **probabilidades**, micromlgen retorna apenas a **classe**
- m2cgen suporta mais modelos além de Random Forest
- micromlgen é específico para modelos scikit-learn com foco em microcontroladores

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
│     m2cgen          │
│  4. Gerar .hpp      │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│  AIoTRandomForest   │
│  _mc2gen.hpp        │  ◄── Código C++ gerado
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

**3. Inferência (Random Forest m2cgen):**
```cpp
double probabilidades[2];
RandomForest::score(inputModelo, probabilidades);
// probabilidades[0] = 0.15  (15% vazia)
// probabilidades[1] = 0.85  (85% ocupada)
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

> **Importante:** O treinamento e exportação do modelo é feito no Google Colab apresentado em sala de aula.

**Resultado:** Dois arquivos gerados:
- `AIoTRandomForest_mc2gen.hpp` - Modelo Random Forest em C++
- `AIoTOccupancyRFScaler.hpp` - Parâmetros do StandardScaler

### 2. Preparar o Código ESP32

Os arquivos gerados no Colab devem ser:
- `src/AIoTRandomForest_mc2gen.hpp`
- `src/AIoTOccupancyRFScaler.hpp`

**Estrutura do projeto:**
```
app21-AIoT-RF-Occupancy_mc2gen/
├── src/
│   ├── iot-app21_AIoT_mc2gen.ino       # Código principal
│   ├── AIoTRandomForest_mc2gen.hpp     # Modelo (gerado no Colab)
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
   - `app21-AIoT-RF-Occupancy_mc2gen`

2. **Compilar e executar:**
   - O ESP32 iniciará e executará inferências a cada 5 segundos

3. **Monitorar Serial Monitor:**

```
==========================================
ESP32 - AIoT - ML EMBARCADO (100% LOCAL)
Modelo: Random Forest - Ocupação de Sala
Modo: SEM CONECTIVIDADE (offline)
Biblioteca: MC2GEN
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
  --> Probabilidades: Vazia=15.0% | Ocupada=85.0%
  --> LED: LIGADO

[STATS] Total: 1 | Ocupada: 1 (100.0%) | Vazia: 0 (0.0%)
==========================================
```

4. **Ajustar sensores:**
   - Use o slider do **LDR** para simular luminosidade
   - Use o slider do **Potenciômetro** para simular CO2
   - DHT22 (temperatura/umidade) varia automaticamente no Wokwi
   - Observe o LED acender/apagar conforme a classificação

## Estrutura do Código

### Arquivo Principal (iot-app21_AIoT_mc2gen.ino)

```cpp
// 1. COLETA DE DADOS
ESP32Sensors::Ambiente::AMBIENTE amb = ESP32Sensors::Ambiente::medirAmbiente();
ESP32Sensors::LDR::DADOS_LDR luz = ESP32Sensors::LDR::ler();
ESP32Sensors::CO2::DADOS_CO2 co2 = ESP32Sensors::CO2::ler();

// 2. NORMALIZAÇÃO
float dadosBrutos[5] = {amb.temp, amb.umid, luz.lux, co2.ppm, hr};
float dadosPadronizados[5];
Scaler::std(dadosBrutos, dadosPadronizados);

// 3. INFERÊNCIA
double inputModelo[5];
for(int i = 0; i < 5; i++) inputModelo[i] = (double)dadosPadronizados[i];

double probabilidades[2];
RandomForest::score(inputModelo, probabilidades);

// 4. CLASSIFICAÇÃO
int predicao = (probabilidades[1] > probabilidades[0]) ? 1 : 0;

// 5. ATUAÇÃO
if (predicao == 1) ESP32Sensors::LED::on();
else ESP32Sensors::LED::off();
```

## Vantagens do m2cgen

✅ **Probabilidades disponíveis**: Diferente do micromlgen, retorna confiança da predição
✅ **Código legível**: Gerado em C puro, fácil de entender e debugar
✅ **Otimizado**: Performance adequada para microcontroladores
✅ **Flexível**: Suporta múltiplos tipos de modelos
✅ **Open source**: Comunidade ativa e bem documentado

## Diferenças: App21 (m2cgen) vs App22 (micromlgen)

| Aspecto | App21 (m2cgen) | App22 (micromlgen) |
|---------|----------------|---------------------|
| **Biblioteca** | m2cgen | micromlgen |
| **Método de inferência** | `RandomForest::score()` | `modeloRF.predict()` |
| **Retorno** | Probabilidades [0.15, 0.85] | Classe direta (0 ou 1) |
| **Arquivo gerado** | AIoTRandomForest_mc2gen.hpp | AIoTRandomForest_micromlgen.hpp |
| **Informação disponível** | Confiança da predição | Apenas decisão final |
| **Uso de memória** | Semelhante | Semelhante |
| **Performance** | Semelhante | Semelhante |

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

1. **Comparar com App22**: Execute também o app22 (micromlgen) e compare resultados
