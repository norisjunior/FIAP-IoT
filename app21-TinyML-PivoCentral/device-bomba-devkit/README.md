# A bomba do pivô — quem manda: a pessoa, depois a nuvem, depois a placa

**Placa do professor.** Uma ESP32 DevKit com um servo (a válvula) e um LED (a bomba). Ela
obedece a três origens, cada uma num tópico, e publica o que fez.

| GPIO | Componente | |
|---|---|---|
| 25 | servo (sinal) | a válvula: 0° fechada, 90° aberta |
| 27 | LED (com resistor) | aceso = bomba ligada |
| 2 | LED onboard | aceso = conectado ao broker |

O servo vai no 5V da placa e no GND. No `setup()`, a válvula abre e fecha uma vez: é o teste
da fiação.

## Os tópicos

`<torre>` é o nome da placa da torre que a bomba obedece agora. Ao ligar, é `PIVO_PADRAO`:
`IoTDevMultiClasse001`, a C3 do professor.

| Tópico | Sentido | O quê |
|---|---|---|
| `FIAPIoT/pivo/<torre>/cmd/local` | assina | a ordem da placa da torre |
| `FIAPIoT/pivo/<torre>/cmd/cloud` | assina | a ordem da nuvem (chuva prevista) |
| `FIAPIoT/pivo/<torre>/cmd/human` | assina | a ordem da pessoa (chat) |
| `FIAPIoT/pivo/<torre>/estado` | publica | ligada ou não, quem manda e o motivo, a cada mudança |
| `FIAPIoT/pivo/bomba/obedecer` | assina | troca a torre: a mensagem é o nome da placa |

A bomba assina `FIAPIoT/pivo/<torre>/cmd/+`, e o fim do tópico diz de quem é a ordem.

## Trocar a torre, sem regravar (a vez de cada grupo)

```bash
mosquitto_pub -h <broker> -t "FIAPIoT/pivo/bomba/obedecer" -m "grupo03"
```

A bomba:

1. publica no `estado` da torre antiga que saiu (`"motivo": "a bomba passou para grupo03"`);
2. deixa de assinar as ordens da torre antiga e assina as de `grupo03`;
3. apaga as ordens guardadas e desliga até a primeira ordem da torre nova (em até 30 s: a
   placa repete a ordem a cada 30 s);
4. publica no `estado` da torre nova (`"motivo": "agora obedece a grupo03"`).

A partir daí, a placa, a nuvem e o chat do grupo 3 (os fluxos com `grupo03` nos tópicos)
mandam nesta bomba. Mensagem vazia volta para `PIVO_PADRAO`; um RESET também. Com `-r`
(mensagem retida), a bomba continua com o grupo depois de um RESET; para apagar a retida:
`mosquitto_pub -h <broker> -t "FIAPIoT/pivo/bomba/obedecer" -r -n`.

## A regra

Cada origem tem a sua última ordem, com a hora em que chegou. Uma vez por segundo (e a cada
ordem nova), a bomba confere quem manda:

| Quem | Vale por | Por quê |
|---|---|---|
| a pessoa | 30 min (`VALIDADE_PESSOA`), ou até devolver | passa por cima de tudo, mas não para sempre |
| a nuvem | 2 h (`VALIDADE_NUVEM`) | a previsão olha 2 h à frente |
| a placa | 90 s (`VALIDADE_PLACA`) | a placa repete a ordem a cada 30 s; se ela some, a bomba para |

Sem ordem válida de ninguém, a bomba desliga.

As ordens:

```json
{"dispositivo": "grupo03", "bomba": true, "motivo": "operando"}
{"dispositivo": "nuvem",   "bomba": false, "motivo": "chuva prevista: 25 mm em 2 h"}
{"dispositivo": "pessoa",  "bomba": true, "motivo": "pedido no chat"}
{"dispositivo": "pessoa",  "devolver": true}
```

`"devolver": true` apaga a ordem daquela origem: o controle volta para quem está abaixo.

## Testar sem torre e sem n8n

```bash
# a placa manda ligar
mosquitto_pub -h <broker> -t "FIAPIoT/pivo/IoTDevMultiClasse001/cmd/local" \
  -m '{"dispositivo":"teste","bomba":true,"motivo":"operando"}'
# a nuvem manda desligar (vale 2 h)
mosquitto_pub -h <broker> -t "FIAPIoT/pivo/IoTDevMultiClasse001/cmd/cloud" \
  -m '{"dispositivo":"nuvem","bomba":false,"motivo":"chuva prevista: 25 mm em 2 h"}'
# a pessoa liga por cima da nuvem, e depois devolve
mosquitto_pub -h <broker> -t "FIAPIoT/pivo/IoTDevMultiClasse001/cmd/human" \
  -m '{"dispositivo":"pessoa","bomba":true,"motivo":"pedido no chat"}'
mosquitto_pub -h <broker> -t "FIAPIoT/pivo/IoTDevMultiClasse001/cmd/human" \
  -m '{"dispositivo":"pessoa","devolver":true}'

# o que a bomba conta
mosquitto_sub -h <broker> -t "FIAPIoT/pivo/+/estado" -v
```

Depois de devolver, a bomba volta a desligar: a ordem da nuvem ainda vale. Para zerar todas as
ordens, aperte RESET na bomba, ou troque a torre.

## Build

```bash
pio run -t upload
pio device monitor
```

`espressif32@6.12.0`, `board = esp32dev`, bibliotecas PubSubClient, ArduinoJson e ESP32Servo.
Flash / RAM: 58,9% / 13,8%.

**Wokwi:** ative o bloco `(A)` no topo do `.cpp`. O `diagram.json` já tem o servo no 25 e o
LED no 27.
