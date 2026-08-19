# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 14 - CPS (Cyber-Physical Systems) e Automação com Low-Code/No-Code

### app14_Sentinela

Aplicação IoT embarcada para ESP32 que monitora o ambiente de uma sala por meio de sensores de temperatura/umidade, distância e presença. Os dados coletados são publicados via MQTT e podem ser consumidos por plataformas de automação como Node-RED ou n8n.

---

### Sensores e Atuadores (ESP32)

| Componente | Descrição | Pino GPIO |
|---|---|---|
| DHT22 | Temperatura e Umidade | 26 |
| HC-SR04 (TRIG) | Distância ultrassônica | 17 |
| HC-SR04 (ECHO) | Distância ultrassônica | 16 |
| PIR | Detecção de movimento/presença | 25 |
| LED Vermelho | Indicador de transmissão ativa | 27 |

---

### Funcionamento

Após conectar-se ao WiFi e ao broker MQTT, o ESP32 inicia um ciclo de leitura a cada **2,5 segundos**:

1. Lê temperatura, umidade e índice de calor (DHT22)
2. Mede a distância ao objeto mais próximo (HC-SR04)
3. Detecta presença ou movimento (PIR)
4. Valida os dados (descarta ciclo se a leitura do DHT for inválida)
5. Serializa os dados em JSON e publica no tópico MQTT
6. Acende o LED durante a transmissão como indicador visual

Em caso de falha de WiFi ou MQTT, o firmware tenta reconexão automática antes de retomar os ciclos de envio.

---

### Publicação MQTT

**Broker:** `host.wokwi.internal:1883`  
**Tópico:** `FIAPIoT/sala1`  
**Device ID:** `NorisESP32IoT2025001`  
**Intervalo:** 2,5 segundos

**Payload JSON publicado:**
```json
{
  "temp": 25.3,
  "umid": 60.5,
  "ic": 26.1,
  "dist": 45.2,
  "mov": true
}
```

| Campo | Tipo | Descrição |
|---|---|---|
| `temp` | float | Temperatura em °C |
| `umid` | float | Umidade relativa em % |
| `ic` | float | Índice de calor (Heat Index) em °C |
| `dist` | float | Distância medida em cm |
| `mov` | bool | `true` se movimento detectado |

---

### Integração com Plataforma IoT

Os dados publicados no tópico `FIAPIoT/sala1` podem ser consumidos por qualquer cliente MQTT. Este projeto prevê integração com:

- **Node-RED** — criação de dashboards, fluxos visuais e alertas
- **n8n** — automação de workflows, notificações e lógica condicional

A configuração das plataformas (flows, workflows, Docker) está nos respectivos subdiretórios do projeto.
