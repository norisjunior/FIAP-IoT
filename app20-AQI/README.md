# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 20 - Sistema de Monitoramento de Qualidade do Ar (AQI)

Esta aplicação monitora a qualidade do ar através de múltiplos sensores analógicos, coletando dados de diferentes poluentes e transmitindo via MQTT.

### Sensores e Pinos

**Sensores de Material Particulado:**
- PM2.5 - Pino GPIO 4
- PM10 - Pino GPIO 5

**Sensores de Óxidos de Nitrogênio:**
- NO (Monóxido de Nitrogênio) - Pino GPIO 6
- NO2 (Dióxido de Nitrogênio) - Pino GPIO 7
- NOx (Óxidos de Nitrogênio) - Pino GPIO 8

**Sensores de Gases:**
- NH3 (Amônia) - Pino GPIO 3
- CO (Monóxido de Carbono) - Pino GPIO 9
- SO2 (Dióxido de Enxofre) - Pino GPIO 1
- O3 (Ozônio) - Pino GPIO 2

**Sensores de COVs (Compostos Orgânicos Voláteis):**
- Benzene - Pino GPIO 10
- Toluene - Pino GPIO 11
- Xylene - Pino GPIO 12

Os dados são coletados a cada 10 segundos e publicados no tópico MQTT `FIAPIoT/aqi/dados` em formato JSON.
