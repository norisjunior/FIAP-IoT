# app17-11-MultiClassAccFeaturesInfluxESP32C3SERVO — 4 classes do motor (ESP32-C3 Super Mini, microservo)

**Placa pronta, fora do conteúdo da aula.** Serve ao aluno que não conseguir montar a própria.
É o mesmo coletor multiclasse que os alunos usam, numa montagem compacta do professor: um ESP32-C3 Super Mini Plus com um microservo que vai e volta enquanto a
coleta roda. Os alunos não têm o servo; coletam com a versão sem motor deste app.

Firmware ESP32 que calcula 8 features de aceleração no próprio dispositivo, em janela fixa de
**100 amostras (1 s @ 100 Hz)**, e publica 1 mensagem MQTT (JSON) por janela — além de imprimir
1 linha CSV por janela no Monitor Serial. Quatro classes, **todas com o servo em movimento**:

- **operando**, **inclinado_frente**, **inclinado_tras**, **anomalia**.

A cada volta completa pela sequência o contador `rodada` incrementa — é o grupo do
`LeaveOneGroupOut` na validação.

> **Hardware físico.** MPU6500 e um microservo sem carga no eixo, ligados ao ESP32-C3 Super
> Mini Plus; o servo é alimentado pelo 5V da própria placa. O `diagram.json` reproduz a
> montagem — MPU, botões e servo — para testar a lógica e o MQTT no simulador.

## O que muda em relação à versão com motor DC

Features, janela, tópico MQTT, payload e measurement são **os mesmos** — os dados entram no
mesmo treino. Muda só o que é da montagem:

| | Versão com motor DC | Esta versão |
|---|---|---|
| Placa | ESP32 DevKit v1 | ESP32-C3 Super Mini Plus (USB nativa) |
| Motor | mini motor com hélice, por ponte H (GPIO 4 e 19) | microservo no GPIO 0, no 5V da placa |
| Durante a coleta | o motor gira | o servo vai e volta entre 60° e 120° |
| LED de coleta | LED onboard (GPIO 2), aceso na coleta | LED RGB da placa (GPIO 8), uma cor por estado |
| Botões COLETA / CLASSE | 21 / 18 | 3 / 4 |
| MPU SDA / SCL | 22 / 23 | 20 / 21 |

## Pinos

| Função | GPIO |
|---|---|
| MPU SDA | 20 |
| MPU SCL | 21 |
| Botão **COLETA** (inicia/para a coleta) | 3 |
| Botão **CLASSE** (avança a sequência, só com a coleta parada) | 4 |
| LED RGB da placa (cor do estado, ver abaixo) | 8 |
| Servo (sinal) | 0 |

A classe atual sai no Monitor Serial. Sensor ativo no código: **MPU6500**
(`#define MPU_TYPE`); para o GY-521 troque para `MPU6050`.

## LED RGB

| Estado | Cor |
|---|---|
| Esperando a coleta | branco |
| Coletando `operando` | verde |
| Coletando `inclinado_frente` | amarelo |
| Coletando `inclinado_tras` | roxo |
| Coletando `anomalia` | vermelho |

As cores estão em `COR_ESPERA` e `COR_CLASSE`, na mesma ordem da `SEQUENCIA`.

## Servo

O servo se move exatamente enquanto a coleta roda: começa com o botão **COLETA** e volta ao
repouso (90°) ao parar pelo botão **e** na parada automática ao completar as 30 janelas.
Durante a coleta, fica meio segundo em 60° e meio segundo em 120° — um vai-e-volta por janela.
Os ângulos e o tempo de cada lado estão em `SERVO_ANG_A`, `SERVO_ANG_B` e `SERVO_MEIO_MS`.

Na calibração o servo ainda não recebe sinal e fica parado.

## Build

Usa a plataforma padrão (`espressif32@6.12.0`) com `board = esp32-c3-devkitm-1`. A placa não
tem chip USB-serial: grave e abra o Monitor Serial pela própria USB-C. O
`ARDUINO_USB_CDC_ON_BOOT=1` do `platformio.ini` faz o `Serial` sair por ela. Se a gravação não
começar, segure **BOOT**, aperte **RESET** e solte **BOOT**.

## Uso

1. **Wokwi:** ative o bloco `(A)` no topo do `.cpp`, troque `#define MPU_TYPE` para `MPU6050`
   (o simulador só tem esse) e comente `mpu.calibrateAccelGyro(&calib);` — sem isso o ESP32
   aborta com `Guru Meditation Error: IntegerDivideByZero`, porque o Wokwi não tem a FIFO do
   MPU. O Monitor Serial do simulador vem pela USB do C3 (`serialInterface` no `diagram.json`).
2. **ESP32-C3 físico:** ative o bloco `(B)` e ajuste WiFi + IP do broker MQTT.
3. Calibrar com o servo parado e o sensor na posição inicial/de uso.
4. Coleta parada: botão 4 escolhe a classe (confira no Monitor Serial e reposicione o
   gabarito). Botão 3 inicia a coleta — o servo começa a ir e voltar e a coleta para sozinha ao
   completar 30 janelas. Repita para as 4 classes; recomendado 3 rodadas completas.
5. Para a classe `anomalia`, provoque a perturbação na montagem (como a fita na hélice da
   versão com motor DC) e desfaça antes de voltar a `operando` na rodada seguinte.

## Payload

Tópico `FIAPIoT/motor/multiclasse` — 1 JSON por janela:

```json
{
  "device": "IoTDevMultiClasse001",
  "label": "inclinado_frente",
  "rodada": 1,
  "janela": 7,
  "ts_ms": 85123,
  "mean_ax": 0.328, "mean_ay": -0.008, "mean_az": 0.929,
  "std_ax": 0.036,  "std_ay": 0.030,  "std_az": 0.042,
  "std_mag": 0.046,
  "p2p_mag": 0.215
}
```

`rodada` = volta completa pela sequência de classes. `ts_ms` = tempo do ESP32 em ms desde o boot (`millis()`).

O mesmo conteúdo sai como CSV no Monitor Serial, com o cabeçalho impresso no boot:

```
ts_ms,label,rodada,janela,mean_ax,mean_ay,mean_az,std_ax,std_ay,std_az,std_mag,p2p_mag
```

Os valores estão em **`g`**: o FastIMU devolve aceleração em `g` e o firmware publica o número
cru. Nivelado, `mean_az` fica perto de `1.0`; inclinado 25°, parte da gravidade migra para
`mean_ax` (`sen 25° ≈ 0,42`).

## Node-RED e treino

O fluxo em `FluxoNodeRED-MultiClass/` é o mesmo da versão sem motor (mesmos IDs de nó): se ele
já estiver importado, não precisa importar de novo. Ele grava no measurement
`vibracao_multiclasse`.

O treino também é o mesmo: o notebook `colab/treinamento_multiclasse.ipynb` da versão sem motor
lê essas janelas do InfluxDB e gera o `modelo_motor_multiclasse.pkl`.
