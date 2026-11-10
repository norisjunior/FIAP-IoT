# app17-8-MultiClassAccFeaturesInfluxDevkitv1MOTOR — 4 classes do motor (ESP32 DevKit v1, motor real)

**Demonstração em aula.** É o mesmo coletor multiclasse que os alunos usam, montado no rig do
professor: um mini motor de verdade, com hélice, girando enquanto a coleta roda. Os alunos não
têm o motor; coletam com a versão sem motor deste app.

Firmware ESP32 que calcula 8 features de aceleração no próprio dispositivo, em janela fixa de
**100 amostras (1 s @ 100 Hz)**, e publica 1 mensagem MQTT (JSON) por janela — além de imprimir
1 linha CSV por janela no Monitor Serial. Quatro classes, **todas com o motor girando**:

- **operando**, **inclinado_frente**, **inclinado_tras**, **anomalia**.

A cada volta completa pela sequência o contador `rodada` incrementa — é o grupo do
`LeaveOneGroupOut` na validação.

> **Hardware físico.** MPU6500 acoplado a um mini motor 3 V com hélice, acionado por ponte H
> pelo próprio ESP32, montado num gabarito 3D com as posições de inclinação. O `diagram.json`
> cobre só MPU + botões, para compilar e testar a lógica/MQTT no simulador.

## O que muda em relação à versão sem motor

Features, janela, tópico MQTT, payload e measurement são **os mesmos** — os dados das duas
versões entram no mesmo treino. Muda só o que é do rig:

| | Versão sem motor | Esta versão |
|---|---|---|
| Motor | fora do circuito do ESP32 | o ESP32 liga e desliga via ponte H (GPIO 4 e 19) |
| GPIO 4 | LED externo que pisca o índice da classe | `MOTOR_INA` da ponte H |
| Classe atual | LED externo + Monitor Serial | só Monitor Serial |
| Sensor no código | `MPU6050` | `MPU6500` |

## Pinos

| Função | GPIO |
|---|---|
| MPU SDA | 22 |
| MPU SCL | 23 |
| Botão **COLETA** (inicia/para a coleta) | 21 |
| Botão **CLASSE** (avança a sequência, só com a coleta parada) | 18 |
| LED onboard (aceso = coleta em andamento, motor girando) | 2 |
| Ponte H — MOTOR_INA | 4 |
| Ponte H — MOTOR_INB | 19 |

Não há LED externo neste rig: a classe atual sai no Monitor Serial. Sensor ativo no código:
**MPU6500** (`#define MPU_TYPE`); para o GY-521 troque para `MPU6050`.

## Motor

O motor gira exatamente enquanto a coleta roda: liga junto com o botão **COLETA**, desliga ao
parar pelo botão **e** na parada automática ao completar as 30 janelas. Sentido único
(`INA` alto / `INB` baixo); ao parar, ambos vão a baixo — para por inércia (coast), sem freio,
para não sacudir o gabarito bem na hora em que a janela seguinte começaria.

## Uso

1. **Wokwi:** ative o bloco `(A)` no topo do `.cpp`, troque `#define MPU_TYPE` para `MPU6050`
   (o simulador só tem esse) e comente `mpu.calibrateAccelGyro(&calib);` — sem isso o ESP32
   aborta com `Guru Meditation Error: IntegerDivideByZero`, porque o Wokwi não tem a FIFO do
   MPU. O motor não existe no simulador; serve para testar botões, LED onboard e MQTT.
2. **ESP32 físico:** ative o bloco `(B)` e ajuste WiFi + IP do broker MQTT.
3. Calibrar com o motor na posição inicial/de uso, **desligado e parado**.
4. Coleta parada: botão 18 escolhe a classe (confira no Monitor Serial e reposicione o
   gabarito). Botão 21 inicia a coleta — o motor liga e a coleta para sozinha ao completar
   30 janelas. Repita para as 4 classes; recomendado 3 rodadas completas.
5. Para a classe `anomalia`, cole a fita/massa na hélice; remova antes de voltar a `operando`
   na rodada seguinte.

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
