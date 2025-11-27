# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 03 - Servidor Web ESP32 com Sensores

Esta aplicação implementa um servidor web no ESP32, disponibilizando dados de sensores ambientais e de movimento através de rotas HTTP. O ESP32 conecta-se à rede WiFi e responde a requisições web com informações coletadas dos sensores.

### Sensores e Atuadores

**Sensores:**
- DHT22 (Temperatura e Umidade) - Pino GPIO 27
- MPU6050 (Acelerômetro) - Pinos I2C:
  - SDA: GPIO 18
  - SCL: GPIO 19

**Atuadores:**
- LED Vermelho - Pino GPIO 14

### Funcionamento

O ESP32 cria um servidor web na porta 80 com as seguintes rotas:

- **/** - Página inicial com lista de rotas disponíveis
- **/ambiente** - Retorna temperatura (°C) e umidade (%)
- **/accel** - Retorna dados do acelerômetro (eixos x, y, z)

O servidor conecta-se à rede WiFi "Wokwi-GUEST" e permanece aguardando requisições HTTP. Os dados dos sensores são coletados em tempo real quando as rotas são acessadas.

### Acesso

Após inicialização, o servidor pode ser acessado via navegador pelo endereço IP exibido no monitor serial (http://localhost:8180 no ambiente Wokwi).