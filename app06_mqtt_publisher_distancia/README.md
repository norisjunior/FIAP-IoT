# Aplicação 06 - Dispositivo do Professor

Lado do professor da demonstração de MQTT em sala. O HC-SR04 mede a distância e o
ESP32 **publica o número** em `fiap/iot/2026b/prof/dist`, com retained.

Ele não decide nada sobre LEDs, buzzers ou limiares — quem decide é cada dispositivo
assinante, com o próprio limiar. É esse o ponto da aula: **o publisher não sabe quem
está ouvindo, quantos são, nem que regra cada um aplica.**

| Peça | Conteúdo |
|---|---|
| [src/](src/) | ESP32 + HC-SR04 (TRIG 25, ECHO 26) |
| [Servidor/](Servidor/) | dashboard Node-RED e script Python que recebem os botões |
| [ROTEIRO-AULA.md](ROTEIRO-AULA.md) | checklist de rede e a demonstração em 5 atos |

O dispositivo do grupo está na [Aplicação 07](../app07_mqtt_subscriber_atuador/).

> O pino ECHO devolve 5 V e o ESP32 é de 3,3 V. Use um divisor 1 kΩ/2 kΩ no ECHO, ou
> alimente o sensor em 3,3 V — perde alcance, sobra para a demonstração.

A leitura do sensor fica em [ESP32SensorsDistancia.hpp](src/ESP32SensorsDistancia.hpp):
`inicializar()` e `medirDistancia()`, sem biblioteca externa. O `pulseIn` tem timeout de
30 ms, então uma leitura sem eco devolve `0` em vez de travar o `loop()` por 1 segundo —
e `0` mantém o LED dos grupos apagado, porque o teste lá é `dist > 0 && dist <= MEU_LIMIAR`.

## Gravar

```bash
pio run -t upload
pio device monitor
```

Antes da aula, leia o [ROTEIRO-AULA.md](ROTEIRO-AULA.md): o principal ponto de falha
desta demonstração não é o código, é a rede.
