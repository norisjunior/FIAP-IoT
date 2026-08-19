# Aplicação 07 - Dispositivo do Grupo

Assina o tópico do professor e aciona LED/buzzer; publica no tópico do grupo quando o
botão é apertado. Os dois sentidos do MQTT em um único sketch.

O dispositivo do professor está na [Aplicação 06](../app06_mqtt_publisher_distancia/).

## Montagem

| Componente | GPIO | Ligação |
|---|---|---|
| LED + resistor 220 Ω | 2 | GPIO → resistor → ânodo; catodo → GND |
| Buzzer **ativo** | 4 | (+) → GPIO; (−) → GND |
| Botão | 19 | uma perna → GPIO; a outra → GND |

## O que editar

Só o bloco `EDITE AQUI` em [src/app07-subscriber-atuador.ino](src/app07-subscriber-atuador.ino):
`MEU_GRUPO`, `MEU_LIMIAR`, `BROKER_IP`, SSID e senha.

| Grupo | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 |
|---|---|---|---|---|---|---|---|---|---|---|
| `MEU_LIMIAR` (cm) | 120 | 100 | 85 | 70 | 60 | 50 | 40 | 30 | 20 | 12 |

No 2º round da demonstração, descomente o `digitalWrite(BUZZER, ligado)` e regrave.

## Tópicos

| Tópico | Papel |
|---|---|
| `fiap/iot/2026b/prof/#` | assina — tudo do professor (coringa `#`) |
| `fiap/iot/2026b/grupo/<NN>/cmd` | assina — endereçado só a este grupo |
| `fiap/iot/2026b/grupo/<NN>/botao` | publica — o botão do grupo |

## Gravar

```bash
pio run -t upload
pio device monitor
```

O monitor mostra cada mensagem recebida (`[RX]`) e publicada (`[TX]`).

| Sintoma | Causa provável |
|---|---|
| Trava em `Conectando ao WiFi...` | rede em 5 GHz, WPA3, ou SSID/senha errados |
| `falhou. Tentando de novo` | `BROKER_IP` errado, firewall do notebook, ou isolamento de AP no roteador |
| Conecta e cai em loop | dois grupos com o mesmo `MEU_GRUPO` |
| LED acende sozinho ao ligar | é o **retained**: o professor já está perto do sensor |
