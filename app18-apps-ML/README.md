# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 18 - Coleta de Dados para Machine Learning com InfluxDB

Esta aplicação demonstra um pipeline completo de coleta de dados IoT para treinamento de modelos de Machine Learning. Dois dispositivos ESP32 com diferentes conjuntos de sensores coletam dados simultaneamente, publicando via MQTT. O Node-RED processa e armazena os dados no InfluxDB, criando um dataset estruturado para posterior análise e treinamento de ML.

## Pré-requisitos

### Inicializar a Plataforma IoT

Esta aplicação **requer obrigatoriamente** a plataforma IoT completa rodando. Siga as instruções em `IoT-platform/README.md`:

1. **Acessar WSL2 Ubuntu:**
   ```bash
   wsl -d ubuntu
   ```

2. **Clonar o repositório (se ainda não clonou):**
   ```bash
   cd ~
   git clone https://github.com/norisjunior/FIAP-IoT
   ```

3. **Iniciar todos os serviços:**
   ```bash
   cd FIAP-IoT/IoT-platform/
   sudo ./start-linux.sh
   ```

Isso iniciará: MQTT Broker, Node-RED, n8n, InfluxDB e Grafana.

**Serviços utilizados nesta aplicação:**
- MQTT Broker: localhost:1883 (para ESP32: host.wokwi.internal:1883)
- Node-RED: http://localhost:1880 (admin/FIAPIoT)
- InfluxDB: http://localhost:8086 (admin/FIAP@123)
  - Organização: fiapiot
  - Bucket: sensores
  - Token: TOKEN_SUPER_SECRETO_1234567890
- Grafana: http://localhost:3000 (admin/admin)

## Estrutura do Projeto

```
app18-apps-ML/
├── app-ML-1-DHT-HC-SR04/        # Device 1: Sensores de distância e ambiente
│   └── src/DeviceML1.ino
├── app-ML-2-DHT-MPU6050/        # Device 2: Sensores de movimento e ambiente
│   └── src/DeviceML2.ino
├── app18-Fluxo-Node-RED-Influx.json  # Flow Node-RED para InfluxDB
└── Info-InfluxDB.txt            # Configurações InfluxDB
```

## Dispositivos e Sensores

### Device ML1 - DHT22 + HC-SR04

**Sensores:**
- DHT22 (Temperatura e Umidade) - Pino GPIO 26
- HC-SR04 (Sensor Ultrassônico de Distância) - Pinos:
  - TRIG: GPIO 17
  - ECHO: GPIO 16

**Atuadores:**
- LED Vermelho - Pino GPIO 27 (Indicador de status do motor)

**Configuração MQTT:**
- Client ID: IoTDeviceNoris001_ML
- Tópico publicação: `FIAPIoT/lab/noris/motor/dist/dados`
- Tópico subscrição: `FIAPIoT/lab/noris/motor/dist/cmd`
- Intervalo de coleta: 5 segundos

### Device ML2 - DHT22 + MPU6050

**Sensores:**
- DHT22 (Temperatura e Umidade) - Pino GPIO 26
- MPU6050 (Acelerômetro/Giroscópio) - Pinos I2C:
  - SDA: GPIO 18
  - SCL: GPIO 19

**Atuadores:**
- LED Vermelho - Pino GPIO 27 (Indicador de status do motor)

**Configuração MQTT:**
- Client ID: IoTDeviceNoris002_ML
- Tópico publicação: `FIAPIoT/lab/noris/motor/mov/dados`
- Tópico subscrição: `FIAPIoT/lab/noris/motor/mov/cmd`
- Intervalo de coleta: 3 segundos

## Pipeline de Dados para Machine Learning

### Arquitetura do Sistema

```
┌─────────────┐      ┌─────────────┐
│ Device ML1  │      │ Device ML2  │
│ DHT+HC-SR04 │      │ DHT+MPU6050 │
└──────┬──────┘      └──────┬──────┘
       │                    │
       │   MQTT (JSON)      │
       └────────┬───────────┘
                │
         ┌──────▼──────┐
         │ MQTT Broker │
         └──────┬──────┘
                │
         ┌──────▼──────┐
         │  Node-RED   │ ◄── app18-Fluxo-Node-RED-Influx.json
         │ (Processamento)
         └──────┬──────┘
                │
         ┌──────▼──────┐
         │  InfluxDB   │ ◄── Time-Series Database
         │  (Storage)  │
         └──────┬──────┘
                │
         ┌──────▼──────┐
         │   Dataset   │ ◄── Export para ML
         │   (CSV)     │
         └─────────────┘
```

## Como Usar

### 1. Configurar Node-RED

1. **Acessar Node-RED:**
   ```
   http://localhost:1880
   ```

2. **Importar o fluxo:**
   - Menu > Import
   - Selecionar arquivo: `app18-Fluxo-Node-RED-Influx.json`
   - Deploy

3. **Verificar configurações:**
   - Nó MQTT: Verificar conexão com broker (mqtt-broker:1883)
   - Nó InfluxDB: Verificar configurações:
     - URL: http://influxdb:8086
     - Org: fiapiot
     - Bucket: sensores
     - Token: TOKEN_SUPER_SECRETO_1234567890

### 2. Executar os Dispositivos ESP32

Execute **ambos** os dispositivos simultaneamente:

1. **Device ML1:**
   - Compile e execute `app-ML-1-DHT-HC-SR04/src/DeviceML1.ino`

2. **Device ML2:**
   - Compile e execute `app-ML-2-DHT-MPU6050/src/DeviceML2.ino`

### 3. Monitorar Coleta de Dados

**No Node-RED:**
- Abra a aba "Debug" para ver os dados sendo processados
- Verifique se os dados estão sendo escritos no InfluxDB

**No InfluxDB:**
1. Acessar: http://localhost:8086
2. Login: admin/FIAP@123
3. Data Explorer > Bucket "sensores"
4. Verificar measurement: `fiapiot_aula09_nearedge`

### 4. Visualizar Dados no Grafana

1. Acessar Grafana: http://localhost:3000
2. Login: admin/admin
3. Criar dashboard conectando ao InfluxDB
4. Visualizar séries temporais dos sensores

### 5. Exportar Dataset para Machine Learning

**Usar Flux Query no InfluxDB:**

```flux
from(bucket: "sensores")
  |> range(start: -24h)
  |> filter(fn: (r) => r._measurement == "fiapiot_aula09_nearedge")
  |> pivot(rowKey:["_time"], columnKey: ["_field"], valueColumn: "_value")
```

**Exportar como CSV:**
- No Data Explorer do InfluxDB, executar a query acima
- Clicar em "CSV" para download
- Dataset estará pronto para análise e treinamento de ML

## Dados Coletados

### Fields no InfluxDB:

**Device ML1 (Distância):**
- temperatura (°C)
- umidade (%)
- indice_calor (°C)
- distancia (cm)
- motor_status (boolean)

**Device ML2 (Movimento):**
- temperatura (°C)
- umidade (%)
- indice_calor (°C)
- accel_x, accel_y, accel_z (m/s²)
- motor_status (boolean)

### Tags:
- device_id (identificador único do dispositivo)
- location (localização)

## Casos de Uso para Machine Learning

### 1. Predição de Falhas em Motores
- **Features:** Temperatura, vibração (acelerômetro), umidade
- **Target:** Detectar anomalias que precedem falhas

### 2. Detecção de Ocupação/Presença
- **Features:** Distância, movimento, temperatura
- **Target:** Classificar ocupação de ambientes

### 3. Manutenção Preditiva
- **Features:** Padrões de vibração, temperatura ao longo do tempo
- **Target:** Prever necessidade de manutenção

### 4. Otimização de Consumo Energético
- **Features:** Padrões de uso (movimento, ocupação), condições ambientais
- **Target:** Otimizar ligar/desligar de equipamentos

## Configurações Avançadas

### InfluxDB Cloud (Alternativa)

Se preferir usar InfluxDB Cloud ao invés do local, configure conforme `Info-InfluxDB.txt`:

```
URL: https://us-east-1-1.aws.cloud2.influxdata.com
ORG: e044ac59f07be199
BUCKET: IoTSensores
MEASUREMENT: fiapiot_aula09_resumo_1minuto
```

### Agregação de Dados

No Node-RED, você pode configurar agregações:
- Média por minuto
- Máximo/Mínimo por hora
- Desvio padrão para detecção de anomalias

## Troubleshooting

**Dados não aparecem no InfluxDB:**
- Verificar se o Node-RED está conectado ao broker MQTT
- Verificar credenciais do InfluxDB no fluxo Node-RED
- Checar logs do Node-RED: Menu > View > Debug messages

**Dispositivos não conectam ao MQTT:**
- Verificar se a plataforma IoT está rodando: `docker ps`
- Verificar configuração do broker no código ESP32

**Queries retornam vazio:**
- Aguardar alguns minutos para coleta de dados
- Ajustar range de tempo na query Flux
- Verificar nome correto do measurement

## Próximos Passos

1. **Coletar dados por período significativo** (horas/dias)
2. **Exportar dataset completo** do InfluxDB
3. **Análise exploratória** dos dados (Python/Pandas)
4. **Feature engineering** conforme caso de uso
5. **Treinamento de modelos** (Scikit-learn, TensorFlow, etc.)
6. **Deploy do modelo** de volta ao ESP32 (TinyML) ou servidor
