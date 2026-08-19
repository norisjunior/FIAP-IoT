# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 10 - Armazenamento em Cartão SD

Esta aplicação demonstra o uso de cartão SD para armazenar dados de sensores. O sistema coleta periodicamente dados de temperatura e umidade, salvando-os em arquivo CSV no cartão SD externo via interface SPI.

### Sensores e Atuadores

**Sensores:**
- DHT22 (Temperatura e Umidade) - Pino GPIO 26
- Botão 1 (Visualizar) - Pino GPIO 25 (com pull-up interno)
- Botão 2 (Zerar) - Pino GPIO 16 (com pull-up interno)

**Atuadores:**
- LED - Pino GPIO 27 (Indica gravação de dados)

**Interface SD Card (SPI):**
- CS (Chip Select) - Pino GPIO 5
- SCK (Clock) - Pino GPIO 18
- MISO (Data Out) - Pino GPIO 19
- MOSI (Data In) - Pino GPIO 23

### Funcionamento

O sistema coleta dados de temperatura, umidade e índice de calor a cada 5 segundos, armazenando em arquivo CSV (/dados.csv) no cartão SD:

**Recursos:**
- Gravação automática de medições em arquivo CSV no SD card
- Botão 1: Imprime conteúdo do arquivo no monitor serial
- Botão 2: Zera todas as medições (apaga e recria o arquivo)
- LED pisca durante gravação de dados
- Exibe tamanho do cartão SD na inicialização

**Estrutura do arquivo:**
```
temp_C,umid_%,indice_calor_C
25.30,60.50,26.15
24.80,62.10,25.90
...
```

O sistema usa o barramento SPI (VSPI) padrão do ESP32 para comunicação com o cartão SD, permitindo armazenamento de grandes volumes de dados com persistência mesmo após desligar o dispositivo.