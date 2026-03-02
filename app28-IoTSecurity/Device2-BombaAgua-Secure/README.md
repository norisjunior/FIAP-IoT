# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 24 - Device 2 - Controle da Bomba d'Água

Este repositório contém o firmware para um ESP32 que controla uma bomba d'água simulada (servo + LED) via MQTT. O dispositivo recebe comandos JSON, identifica a origem e liga/desliga a bomba atuando sobre um servo e um LED.

**O que a aplicação faz**
- Conecta ao Wi‑Fi e a um broker MQTT.
- Inscreve-se em `FIAPIoT/smartagro/cmd/+` e processa comandos de controle.
- Recebe JSON com `bomba` (boolean) e `dispositivo` (string).
- Liga/desliga o servo e acende/apaga o LED conforme `bomba: true|false`.

**Fluxo resumido**
1. Inicializa módulos (`ESP32Sensors`).
2. Conecta ao Wi‑Fi e ao broker MQTT (padrão: `host.wokwi.internal:1883`).
3. Escuta `FIAPIoT/smartagro/cmd/+`.
4. Ao receber JSON válido com `bomba`, aplica comando: ligar/desligar.

**Tópicos MQTT e exemplo**
- Subscrição: `FIAPIoT/smartagro/cmd/+` (ex.: `.../cmd/local`, `.../cmd/cloud`, `.../cmd/human`).
- Exemplo de payload:

```json
{ "dispositivo": "FIAPIoTapp24B", "bomba": true }
```

**Pinos usados**
- LED: GPIO 27
- Servo: GPIO 13

