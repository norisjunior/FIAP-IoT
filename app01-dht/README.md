# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 01 - Monitoramento de Temperatura e Umidade com DHT22

Esta aplicação realiza o monitoramento de condições ambientais utilizando um sensor DHT22, coletando dados de temperatura e umidade. O sistema calcula o índice de calor e aciona LEDs de acordo com as condições ambientais detectadas.

### Sensores e Atuadores

**Sensores:**
- DHT22 (Temperatura e Umidade) - Pino GPIO 26

**Atuadores:**
- LED Vermelho - Pino GPIO 25 (Indica condições ambientais inadequadas quando índice de calor > 25°C)
- LED Verde - Pino GPIO 27 (Pisca para indicar nova leitura)

### Funcionamento

O sistema realiza leituras a cada 2 segundos, exibindo no monitor serial:
- Temperatura em °C
- Umidade em %
- Índice de calor calculado

Quando o índice de calor ultrapassa 25°C, o LED vermelho é acionado, indicando condições ambientais inadequadas. O LED verde pisca 4 vezes para sinalizar o início de uma nova leitura.
