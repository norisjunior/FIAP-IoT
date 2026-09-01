# app17-7-MultiClassAccFeaturesInflux (ESP32-C6)

Firmware **ESP32-C6** que calcula 12 features de aceleração no próprio dispositivo, em janela
fixa de **1000 amostras (2 s @ 500 Hz)**, e publica 1 mensagem MQTT (JSON) por janela — além
de imprimir 1 linha CSV por janela no Monitor Serial. Cinco classes cíclicas, selecionadas
por botão:

```
desligado → operando → inclinado_frente → inclinado_tras → anomalia → (repete)
```

A cada volta completa pela sequência o contador `rodada` incrementa — é o grupo do
`LeaveOneGroupOut` na validação. As classes de inclinação (`inclinado_frente` /
`inclinado_tras`) e a `anomalia` exigem o motor e o gabarito 3D reais; o `diagram.json` do
Wokwi cobre só MPU + botões + LED, para compilar e testar a lógica.

> Mesma lógica do app17-7 original — só muda o alvo (ESP32-C6) e os pinos, agora iguais
> aos do **app17-6**.

## Build

O ESP32-C6 exige o core Arduino-ESP32 3.x. O `platformio.ini` usa o platform da comunidade
[`pioarduino`](https://github.com/pioarduino/platform-espressif32) (`board =
esp32-c6-devkitc-1`); o platform oficial `platformio/espressif32` ainda não suporta o C6 no
framework Arduino. Grave/monitore pela porta **UART** da placa (ponte USB-serial). Para usar
a porta **USB** nativa do C6, ative `-D ARDUINO_USB_CDC_ON_BOOT=1` no `platformio.ini`.

## Pinos (iguais aos do app17-6)

| Função | GPIO |
|---|---|
| MPU SDA | 5 |
| MPU SCL | 4 |
| Botão **COLETA** (inicia/para a coleta) | 12 |
| Botão **CLASSE** (avança para a próxima classe, só com a coleta parada) | 13 |
| LED externo (aceso = coleta em andamento) | 7 |
| LED onboard = LED RGB WS2812 da DevKitC-1 (`RGB_BUILTIN`) | 8 |

Sensor ativo no código: **MPU6050** (`#define MPU_TYPE`). Para o MPU6500, troque a linha no
topo do `.cpp`.

## Uso

1. **Wokwi:** comente `mpu.calibrateAccelGyro(&calib);` (trava no simulador, FIFO ausente).
2. **ESP32-C6 físico:** ative o bloco `(B)` no topo do `.cpp` e ajuste WiFi + IP do broker MQTT.
3. Compile e rode. Com a coleta parada, o botão 13 avança a classe (imprime a classe atual no
   Monitor Serial). O botão 12 inicia/para a coleta da classe atual — enquanto roda, sai 1
   linha CSV + 1 publish MQTT a cada 2 s.

## Payload

Tópico `FIAPIoT/motor/multiclasse` — 1 JSON por janela:

```json
{
  "device": "IoTDevMultiClasse001",
  "classe": "operando",
  "rodada": 1,
  "janela": 12,
  "fs_real": 499.8,
  "mean_ax": 0.021, "mean_ay": -0.015, "mean_az": 9.812,
  "std_ax": 0.042,  "std_ay": 0.037,  "std_az": 0.051,
  "rms_mag": 9.813, "std_mag": 0.048, "p2p_mag": 0.310,
  "crest_mag": 3.10, "kurt_mag": 0.41, "zcr_mag": 0.12
}
```

CSV no Monitor Serial, mesmo cabeçalho impresso no boot:

```
classe,rodada,janela,fs_real,mean_ax,mean_ay,mean_az,std_ax,std_ay,std_az,rms_mag,std_mag,p2p_mag,crest_mag,kurt_mag,zcr_mag
```

`fs_real` é a taxa de amostragem efetiva medida na própria janela (amostras / tempo decorrido)
— confira se fica perto de 500 Hz.
