# app17-11-MotorMultiClasse — 4 classes do motor (ESP32 DevKit v1, motor real)

Firmware ESP32 que calcula 8 features de aceleração no próprio dispositivo, em janela fixa de
**100 amostras (1 s @ 100 Hz)**, e publica 1 mensagem MQTT (JSON) por janela — além de imprimir
1 linha CSV por janela no Monitor Serial. Evolução multiclasse do `app17-6` (binário: normal vs
anomalia). Quatro classes, **todas com o motor girando**:

- **operando**, **inclinado_frente**, **inclinado_tras**, **anomalia**.

A cada volta completa pela sequência o contador `rodada` incrementa — é o grupo do
`LeaveOneGroupOut` na validação.

> **Hardware físico.** MPU6500 acoplado a um mini motor 3 V com hélice, acionado por ponte H
> pelo próprio ESP32, montado num gabarito 3D com as posições de inclinação. O motor e as
> classes de inclinação **não têm equivalente fiel no Wokwi** — o `diagram.json` cobre só
> MPU + botões, para compilar e testar a lógica/MQTT no simulador.
>
> Mesmas features e mesma estrutura das outras versões deste app (`...ESP32C6`); o que muda
> aqui é o alvo (DevKit v1), o acionamento do motor e a ausência de LED externo.

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

1. **Wokwi:** ative o bloco `(A)` no topo do `.cpp` e comente `mpu.calibrateAccelGyro(&calib);`
   (trava no simulador, FIFO ausente).
2. **ESP32 físico:** ative o bloco `(B)` e ajuste WiFi + IP do broker MQTT.
3. Calibrar com o motor na posição inicial/de uso, **desligado e parado**.
4. Coleta parada: botão 18 escolhe a classe (confira no Monitor Serial e reposicione o
   gabarito). Botão 21 inicia a coleta — o motor liga e a coleta para sozinha ao completar
   30 janelas. Repita para as 4 classes; recomendado 3 rodadas completas.
5. Para a classe `anomalia`, cole a fita/massa na hélice; remova antes de voltar a `operando`
   na rodada seguinte.

## Payload

Tópico `FIAPIoT/motor/multiclasse`:

```json
{
  "device": "IoTDevMultiClasse001",
  "label": "inclinado_frente",
  "rodada": 1,
  "janela": 7,
  "ts_epoch_ms": 1749760205123,
  "mean_ax": 0.328, "mean_ay": -0.008, "mean_az": 0.929,
  "std_ax": 0.036,  "std_ay": 0.030,  "std_az": 0.042,
  "std_mag": 0.046,
  "p2p_mag": 0.215
}
```

`rodada` = volta completa pela sequência de classes. `ts_epoch_ms` = epoch em ms (UTC), via
NTP + `millis()`.

O mesmo conteúdo sai como CSV no Monitor Serial, com o cabeçalho impresso no boot:

```
ts_epoch_ms,label,rodada,janela,mean_ax,mean_ay,mean_az,std_ax,std_ay,std_az,std_mag,p2p_mag
```

Os valores estão em **`g`**: o FastIMU devolve aceleração em `g` e o firmware publica o número
cru. Nivelado, `mean_az` fica perto de `1.0`; inclinado 25°, parte da gravidade migra para
`mean_ax` (`sen 25° ≈ 0,42`).

## Treino

[`analytics/notebooks/2.6_multiclasse_motor.ipynb`](../analytics/notebooks/2.6_multiclasse_motor.ipynb)
lê estas janelas do InfluxDB (measurement `vibracao_multiclasse`) e gera o
`modelo_motor_multiclasse.pkl`.
