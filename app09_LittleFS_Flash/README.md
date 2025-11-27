# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 09 - Armazenamento em Flash usando LittleFS

Esta aplicação demonstra o uso do sistema de arquivos LittleFS para armazenar dados de sensores na memória flash interna do ESP32. O sistema coleta periodicamente dados de temperatura e umidade, salvando-os em arquivo CSV na flash.

### Sensores e Atuadores

**Sensores:**
- DHT22 (Temperatura e Umidade) - Pino GPIO 26
- Botão Push - Pino GPIO 25 (com pull-up interno)

**Atuadores:**
- LED - Pino GPIO 27 (Indica gravação de dados)

### Funcionamento

O sistema coleta dados de temperatura, umidade e índice de calor a cada 5 segundos, armazenando em arquivo CSV (/dados.csv) na memória flash:

**Recursos:**
- Gravação automática de medições em arquivo CSV
- Cabeçalho: "temp_C,umid_%,indice_calor_C"
- LED pisca durante gravação de dados
- Botão permite zerar todas as medições (apaga e recria o arquivo)
- Exibe conteúdo completo do arquivo no monitor serial após cada gravação

**Estrutura do arquivo:**
```
temp_C,umid_%,indice_calor_C
25.30,60.50,26.15
24.80,62.10,25.90
...
```

O LittleFS é montado automaticamente na inicialização, criando o arquivo se não existir. O sistema mantém um único arquivo de log sem rotação, permitindo acumulação contínua de dados até que o usuário pressione o botão para zerar.