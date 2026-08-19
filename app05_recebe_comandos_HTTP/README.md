# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 05 - Controle Remoto via HTTP

Esta aplicação implementa um servidor web no ESP32 que recebe comandos HTTP para controlar atuadores. O sistema permite ligar/desligar um LED e reproduzir sons através de um buzzer, tudo via requisições HTTP.

### Sensores e Atuadores

**Atuadores:**
- LED Vermelho - Pino GPIO 26 (Controle on/off via HTTP)
- Buzzer - Pino GPIO 27 (Reproduz sons com frequência e duração configuráveis)

### Funcionamento

O ESP32 cria um servidor web na porta 80 com as seguintes rotas:

- **/** - Página inicial com lista de comandos disponíveis
- **/led/on** - Acende o LED vermelho
- **/led/off** - Apaga o LED vermelho
- **/play** - Toca um som no buzzer (aceita parâmetros query string: freq=frequência, dur=duração)

Exemplo de uso da rota /play:
- `/play?freq=1000&dur=500` - Toca um som de 1000Hz por 500ms

O servidor conecta-se à rede WiFi "Wokwi-GUEST" e permanece aguardando comandos HTTP para controlar os atuadores remotamente.