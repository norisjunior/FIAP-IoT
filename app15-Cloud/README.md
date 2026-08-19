# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 15 - Cloud Computing com MQTT

### app15-Cloud

Aplicação IoT embarcada para ESP32 que simula o monitoramento remoto de um motor industrial por meio de comunicação bidirecional via MQTT. O dispositivo coleta dados de temperatura/umidade (DHT22) e aceleração (MPU6050), publica os leituras na nuvem a cada 3 segundos e responde a comandos de ligar/desligar recebidos do broker, com o LED indicando o estado operacional do motor.

---

### Sensores e Atuadores (ESP32)

| Componente | Descrição | Pino GPIO |
|---|---|---|
| DHT22 | Temperatura e Umidade | 26 |
| MPU6050 (SDA) | Acelerômetro via I2C | 18 |
| MPU6050 (SCL) | Acelerômetro via I2C | 19 |
| LED | Indicador de status do motor | 27 |

---

### Tópicos MQTT

**Broker:** `host.wokwi.internal:1883`  
**Client ID:** `IoTDeviceNoris001`  
**Intervalo de envio:** 3 segundos

| Direção | Tópico | Descrição |
|---|---|---|
| Publicação | `FIAPIoT/aula08/noris/motor/dados` | Dados dos sensores em JSON |
| Subscrição | `FIAPIoT/aula08/noris/motor/cmd` | Comandos `ON` / `OFF` |

**Payload JSON publicado:**
```json
{
  "device": "IoTDeviceNoris001",
  "temp": 25.3,
  "umid": 60.5,
  "ic": 26.1,
  "accel_x": 0.12,
  "accel_y": -0.05,
  "accel_z": 9.81,
  "motor_status": true
}
```

---

### Dashboards Node-RED

Dois dashboards estão disponíveis em `Plataformas_config/NodeRED`:

**1. Dashboard de Controle (`Fluxo_1_dashboard_graphs_e_cmd.json`)**  
Visualiza em tempo real os dados dos sensores (temperatura, umidade, aceleração) e disponibiliza botões para enviar comandos `ON`/`OFF` ao dispositivo via MQTT.

**2. Dashboard com InfluxDB (`Fluxo_2_envio_InfluxDB.json`)**  
Recebe os dados do tópico MQTT e os persiste no InfluxDB para armazenamento histórico, permitindo consultas e análises temporais dos dados do motor.

---

### Plataforma IoT

Os serviços necessários (MQTT Broker, Node-RED, InfluxDB) são provisionados via Docker. Siga as instruções em `IoT-platform/README.md` para inicializar o ambiente.

| Serviço | Endereço | Credenciais |
|---|---|---|
| MQTT Broker | `localhost:1883` | — |
| Node-RED | `http://localhost:1880` | admin / FIAPIoT |
| InfluxDB | `http://localhost:8086` | admin / FIAP@123 |
| Grafana (local) | `http://localhost:3000` | admin / admin |
