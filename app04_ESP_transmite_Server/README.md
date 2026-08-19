# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 04 - ESP32 Transmite Dados para Servidor Python

Esta aplicação demonstra comunicação cliente-servidor entre ESP32 e aplicação Python. O ESP32 coleta dados de sensores ambientais e de movimento, transmitindo-os via socket TCP para um servidor Python que armazena os dados em arquivo CSV.

### Estrutura do Projeto

```
app04_ESP_transmite_Server/
├── app04_transmite_JSON/     # Código do ESP32 (Wokwi + PlatformIO)
│   └── src/
│       └── iot-aula02-app04.ino
└── server_socket/            # Servidor Python
    └── ESP32_data_server.py
```

### Sensores e Atuadores (ESP32)

**Sensores:**
- DHT22 (Temperatura e Umidade) - Pino GPIO 27
- MPU6050 (Acelerômetro) - Pinos I2C:
  - SDA: GPIO 18
  - SCL: GPIO 19

**Atuadores:**
- LED Vermelho - Pino GPIO 14

### Funcionamento

#### 1. Servidor Python (server_socket/)

O servidor Python escuta conexões TCP na porta 5000, recebe dados JSON do ESP32 e armazena em arquivo CSV.

**Iniciar o servidor:**

```bash
cd server_socket
python ESP32_data_server.py
```

**Dependências necessárias:**
```bash
pip install pytz
```

O servidor:
- Escuta na porta 5000
- Recebe dados JSON do ESP32
- Cria/atualiza arquivo `dados.csv` com timestamp (fuso horário de São Paulo)
- Exibe dados recebidos no console
- Para com Ctrl+C

**Estrutura do CSV gerado:**
```csv
timestamp,dispositivo,temp,umid,hic,motor_x,motor_y,motor_z
2025-11-27T10:30:45.123456-03:00,ESP32Noris,25.30,60.50,26.15,0.98,0.05,-9.81
```

#### 2. Código ESP32 (app04_transmite_JSON/)

O ESP32 conecta-se ao servidor Python via WiFi e envia dados a cada 5 segundos.

**Dados enviados (formato JSON):**
```json
{
  "device": "ESP32Noris",
  "temp": 25.30,
  "umid": 60.50,
  "hic": 26.15,
  "motor_accel_x": 0.98,
  "motor_accel_y": 0.05,
  "motor_accel_z": -9.81
}
```

**Configuração de rede:**
- SSID: "Wokwi-GUEST"
- Host: host.wokwi.internal
- Porta: 5000

### Como Usar

1. **Inicie o servidor Python primeiro:**
   ```bash
   cd server_socket
   python ESP32_data_server.py
   ```

2. **Execute o código ESP32 no Wokwi ou dispositivo físico**
   - Compile e carregue o código em `app04_transmite_JSON/`
   - O ESP32 conectará ao WiFi e começará a enviar dados

3. **Monitore os dados:**
   - Console do servidor Python mostrará dados recebidos em tempo real
   - Arquivo `dados.csv` será atualizado automaticamente

4. **Pare o servidor:**
   - Pressione Ctrl+C no terminal do servidor Python

### Casos de Uso

- Monitoramento remoto de temperatura e umidade
- Detecção de vibração/movimento em motores industriais
- Coleta de dados históricos para análise
- Prototipagem de sistemas IoT cliente-servidor
- Integração ESP32 com aplicações Python/Data Science

### Observações

- O servidor deve estar rodando antes de iniciar o ESP32
- Certifique-se de que a porta 5000 não está sendo usada por outro processo
- Para uso em rede local (não Wokwi), altere o `host` no código ESP32 para o IP do computador rodando o servidor Python
