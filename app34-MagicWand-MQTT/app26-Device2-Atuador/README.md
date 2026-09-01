# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 26 - Device 2 - Atuadores MQTT (Bomba + Buzzer)

Sistema embarcado que recebe comandos MQTT e controla dois atuadores independentes:
- **Bomba d'água** (servo motor) - Atuador IPSO 3342
- **Buzzer/Irrigação** (alarme sonoro) - Atuador IPSO 3338

## O que faz

Este dispositivo **assina** tópicos MQTT usando wildcard e **executa comandos** recebidos de diferentes origens (Device 1 com varinha mágica, nuvem, ou interface humana). Cada comando especifica qual atuador deve ser ligado/desligado através de payload JSON padronizado.

## Hardware Necessário

- ESP32 DevKit
- Servo motor (simula bomba d'água)
- Buzzer passivo
- LED (feedback visual)

## Conexões

**Servo Motor (Bomba d'água):**
- VCC → 5V
- GND → GND
- PWM → GPIO 13

**Buzzer:**
- Positivo → GPIO 18
- Negativo → GND

**LED (Indicador):**
- Anodo → GPIO 27
- Catodo → GND

## Configuração MQTT

**Broker:**
- Host: `host.wokwi.internal` (ou IP do broker)
- Porta: `1883`
- Client ID: `FIAPIoTapp26B`

**Tópico de subscrição (com wildcard):**
```
FIAPIoT/smartagro/cmd/+
```

Aceita comandos de:
- `FIAPIoT/smartagro/cmd/local` (Device 1 - Varinha mágica)
- `FIAPIoT/smartagro/cmd/cloud` (Servidor/Automação)
- `FIAPIoT/smartagro/cmd/human` (Interface manual)

## Formato do JSON Esperado

```json
{
  "deviceId": "FIAPIoTapp26Dev1",
  "atuador": "3342",
  "comando": true
}
```

**Campos obrigatórios:**
- `atuador` (string): Código IPSO do atuador
  - `"3342"`: Bomba d'água (On/Off Switch)
  - `"3338"`: Buzzer/Irrigação
- `comando` (boolean):
  - `true`: Liga o atuador
  - `false`: Desliga o atuador

**Campos opcionais:**
- `deviceId` (string): Identificação do dispositivo que enviou o comando

## Fluxo de Funcionamento

```
┌─────────────────────────┐
│   Inicialização         │
│  - WiFi                 │
│  - MQTT Broker          │
│  - Atuadores (teste)    │
└────────┬────────────────┘
         │
         ▼
┌─────────────────────────┐
│  Aguarda comando MQTT   │◄───────┐
│  (subscriber)           │        │
└────────┬────────────────┘        │
         │                         │
         │ Comando recebido        │
         ▼                         │
┌─────────────────────────┐        │
│  Parse JSON             │        │
│  Valida campos          │        │
└────────┬────────────────┘        │
         │                         │
         ▼                         │
┌─────────────────────────┐        │
│  Identifica atuador     │        │
│  3342 → Bomba           │        │
│  3338 → Buzzer          │        │
└────────┬────────────────┘        │
         │                         │
         ▼                         │
┌─────────────────────────┐        │
│  Executa comando        │        │
│  - Liga/Desliga servo   │        │
│  - Liga/Desliga buzzer  │        │
└────────┬────────────────┘        │
         │                         │
         └─────────────────────────┘
```

## Mapeamento Atuadores

| Atuador IPSO | Descrição | Hardware | Ação |
|--------------|-----------|----------|------|
| 3342 | On/Off Switch | Servo motor + LED | Liga/desliga bomba d'água |
| 3338 | Buzzer | Buzzer passivo | Ativa/desativa alarme sonoro |

## Lógica de Controle

### Bomba d'água (3342)
- **Liga** (`comando: true`): Servo → 90°, LED acende
- **Desliga** (`comando: false`): Servo → 0°, LED apaga
- **Prevenção**: Ignora comandos duplicados (evita desgaste do servo)

### Buzzer (3338)
- **Liga** (`comando: true`): PWM de 500 Hz no canal 15
- **Desliga** (`comando: false`): PWM desativado
- **Prevenção**: Ignora comandos duplicados
