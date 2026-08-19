# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 13 - Integração com InfluxDB Cloud

Esta aplicação demonstra a integração do ESP32 com InfluxDB Cloud, um banco de dados de séries temporais (time-series database) otimizado para armazenamento e análise de dados de sensores IoT. O sistema envia medições em tempo real para a nuvem via HTTP.

### Sensores e Atuadores

**Sensores:**
- DHT22 (Temperatura e Umidade) - Pino GPIO 26
- HC-SR04 (Sensor Ultrassônico de Distância) - Pinos:
  - TRIG: GPIO 17
  - ECHO: GPIO 16

**Atuadores:**
- LED - Pino GPIO 27 (Indicador de status)

### Funcionamento

O sistema coleta dados dos sensores a cada 5 segundos e envia para o InfluxDB Cloud usando o protocolo Line Protocol via HTTP POST:

**Dados enviados (measurement: sensores_iot):**
- Tag: dispositivo (identificador do ESP32)
- Fields:
  - distancia_cm: Distância medida em centímetros
  - distancia_alarme: 1 se distância < 100cm, 0 caso contrário
  - temp: Temperatura em °C
  - umid: Umidade em %
  - IC: Índice de calor calculado em °C

**Configuração necessária:**
- URL da API InfluxDB Cloud (região específica)
- Organization ID
- Bucket name
- Token de autenticação com permissões de escrita

**Formato Line Protocol:**
```
sensores_iot,dispositivo=Noris1_ESP32_Aula07 distancia_cm=45.23,distancia_alarme=1,temp=25.30,umid=60.50,IC=26.15
```

O InfluxDB armazena automaticamente timestamps e permite consultas e visualizações avançadas dos dados históricos através do Flux Query Language ou InfluxQL.