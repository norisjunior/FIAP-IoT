# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 12 - Integração com Firebase Realtime Database

Esta aplicação demonstra a integração do ESP32 com o Firebase Realtime Database do Google. O sistema coleta dados de sensores ambientais e de distância, enviando-os para a nuvem via WiFi para armazenamento e visualização em tempo real.

### Sensores e Atuadores

**Sensores:**
- DHT22 (Temperatura e Umidade) - Pino GPIO 26
- HC-SR04 (Sensor Ultrassônico de Distância) - Pinos:
  - TRIG: GPIO 17
  - ECHO: GPIO 16

**Atuadores:**
- LED - Pino GPIO 27 (Indicador de status)

### Funcionamento

O sistema coleta dados dos sensores a cada 30 segundos e envia para o Firebase Realtime Database:

**Dados enviados (nó /iotAula):**
- distancia_cm: Distância medida em centímetros
- distancia_alarme: Boolean indicando se distância < 100cm
- temperatura: Temperatura em °C
- umidade: Umidade em %
- indice_calor: Índice de calor calculado em °C

**Configuração necessária:**
- API Key do projeto Firebase
- URL do Realtime Database
- Email e senha de autenticação do usuário Firebase

O sistema estabelece conexão WiFi, autentica no Firebase usando email/senha, e mantém sincronização automática dos dados. Cada atualização sobrescreve os valores anteriores no mesmo nó, mantendo sempre o estado mais recente dos sensores disponível na nuvem.