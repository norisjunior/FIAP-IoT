# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 17 - Serialização CBOR para Edge Computing

Esta aplicação demonstra o uso de CBOR (Concise Binary Object Representation) para comunicação eficiente em sistemas IoT. O ESP32 coleta dados de sensores, serializa em formato binário compacto CBOR, armazena localmente em SD Card e envia em lotes via MQTT, otimizando largura de banda.

### Sensores e Atuadores

**Sensores:**
- DHT22 (Temperatura e Umidade)
- MPU6050 (Acelerômetro/Giroscópio) - I2C

**Atuadores:**
- LED (Indicador de status do motor)

**Armazenamento:**
- SD Card via SPI (persistência local de dados CBOR)

### Funcionamento

O sistema implementa estratégia de armazenamento local e envio em lote para economia de recursos:

**Coleta de Dados:**
- Intervalo: 5 segundos entre medições
- Dados armazenados localmente em formato CBOR binário
- Máximo: 12 medições por lote (60 segundos)

**Envio via MQTT:**
- Intervalo: 60 segundos (lote completo)
- Tópico: `FIAPIoT/aula10/noris/dados/cbor`
- Formato: CBOR codificado em Base64 para transmissão
- Comandos recebidos em: `FIAPIoT/aula10/noris/motor/cmd`
- Status publicado em: `FIAPIoT/aula10/noris/status`

**Vantagens do CBOR:**
- Redução de 30-50% no tamanho dos dados vs JSON
- Parsing mais rápido (formato binário)
- Suporte nativo a tipos de dados binários
- Ideal para dispositivos com recursos limitados

**Arquitetura Edge Computing:**
- Armazenamento local robusto em SD Card
- Operação offline durante desconexão de rede
- Envio em lote para otimizar bateria e banda
- Recuperação automática de dados não enviados

O sistema codifica dados CBOR em Base64 para compatibilidade com protocolos texto como MQTT, mantendo os benefícios da compactação binária.