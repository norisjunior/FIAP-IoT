# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 17 - Serialização CBOR para Edge Computing

Esta aplicação demonstra o uso de CBOR (Concise Binary Object Representation) para comunicação eficiente em sistemas IoT. O ESP32 coleta dados de sensores, serializa em formato binário compacto CBOR, armazena localmente em SD Card e envia em lotes via MQTT, otimizando largura de banda.

## Pré-requisitos

### Inicializar a Plataforma IoT

Esta aplicação requer a plataforma IoT completa rodando. Siga as instruções em `IoT-platform/README.md`:

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

**Serviços disponíveis:**
- MQTT Broker: localhost:1883 (para ESP32: host.wokwi.internal:1883)
- Node-RED: http://localhost:1880 (admin/FIAPIoT)
- n8n: http://localhost:5678
- InfluxDB: http://localhost:8086 (admin/FIAP@123)
- Grafana: http://localhost:3000 (admin/admin)

**Dica:** Use o Node-RED ou n8n para decodificar e visualizar os dados CBOR recebidos via MQTT.

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