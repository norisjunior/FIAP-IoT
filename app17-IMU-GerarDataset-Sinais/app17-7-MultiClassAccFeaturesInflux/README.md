# app17-7-MultiClassAccFeaturesInflux

Firmware ESP32 que calcula features de aceleração no próprio dispositivo, em janela fixa de
100 amostras (1 s @ 100 Hz), e publica 1 mensagem MQTT (JSON) por janela. Evolução multiclasse
do `app17-6` (binário: normal vs anomalia). Quatro classes, sempre com o motor ligado:

- **operando**, **inclinado_frente**, **inclinado_tras**, **anomalia**.

## Pinos

| Função | GPIO |
|---|---|
| MPU SDA | 22 |
| MPU SCL | 23 |
| Botão COLETA (inicia/para a coleta) | 21 |
| Botão CLASSE (avança a sequência, só com a coleta parada) | 18 |
| LED externo (pisca N vezes = índice da classe atual, 1..4) | 4 |
| LED onboard (aceso = coleta em andamento) | 2 |

## Uso

1. **Wokwi:** ative o bloco `(A)` no topo do `.cpp`, use `#define MPU_TYPE MPU6050` e comente
   `mpu.calibrateAccelGyro(&calib);`. As classes de inclinação não têm equivalente fiel no
   simulador — serve para testar botões, LED e publicação MQTT.
2. **ESP32 físico:** ative o bloco `(B)`, ajuste WiFi + IP do broker MQTT e **troque o
   `DEFINA_UM_NOME` do `MQTT_CLIENT_ID` por um nome só seu**: é o `device` que separa as suas
   janelas das dos colegas no broker e no banco. Dois ESP32 com o mesmo nome no mesmo broker se
   derrubam.
3. Calibrar com o motor na posição inicial/de uso, sem movê-lo.
4. Coleta parada: botão 18 escolhe a classe. Botão 21 inicia a coleta, que para sozinha ao
   completar 30 janelas. Repita para as 4 classes; recomendado 3 rodadas completas.

## O protocolo de coleta (o dataset)

Uma **rodada** é uma volta pelas 4 classes; a **classe** é uma condição mantida estável
durante as 30 janelas. O botão numera; quem posiciona o sensor é quem sabe o rótulo.

| Classe | Como posicionar | O que se espera |
|---|---|---|
| `operando` | parado e nivelado | `mean_az` perto de 1 g; `std_*` perto de 0 |
| `inclinado_frente` | inclinado uns 30° para a frente, apoiado | parte da gravidade migra para um eixo (`mean_ay` ou `mean_ax`, conforme a montagem) |
| `inclinado_tras` | o mesmo, para trás | o mesmo eixo, com o sinal trocado |
| `anomalia` | chacoalhar com a mão, sem parar, as 30 janelas | `std_*` e `p2p_mag` sobem |

1. **Posicione primeiro, aperte depois.** A coleta começa no aperto do 21; a janela 1 já conta.
2. Mantenha a condição até o Serial avisar `>>> 30 janelas coletadas`.
3. Aperte o 18 (a coleta já parou), reposicione, aperte o 21. Depois de `anomalia`, o 18
   volta para `operando` e a rodada sobe.
4. **3 rodadas completas:** 4 classes × 3 rodadas × 30 janelas = 360 janelas, uns 6 min de
   coleta mais o tempo de reposicionar. O treino separa por rodada: a última rodada inteira
   fica de fora. Com uma rodada só não há o que separar.

## Node-RED: o broker da sala e o seu banco

O fluxo [`FluxoNodeRED-MultiClass/flow_multiclasse_influx.json`](FluxoNodeRED-MultiClass/flow_multiclasse_influx.json)
assina `FIAPIoT/motor/multiclasse` e grava no measurement `vibracao_multiclasse`:

| | O quê |
|---|---|
| tags | `label`, `rodada`, `device` |
| fields | as 8 features, `janela`, `ts_ms` |
| broker | **o da sala**, um só para a turma (o IP está no nó `Broker da sala`; confira). O client id fica vazio: o Node-RED sorteia um, e 30 Node-RED não se derrubam |
| banco | o **seu** InfluxDB Cloud: edite URL, org e token no nó de saída; o bucket vem como `IoTSensores` |
| `MEU_DEVICE` | no nó **Monta ponto InfluxDB**: escreva o `MQTT_CLIENT_ID` da sua placa, e o fluxo grava só as suas janelas (as dos colegas aparecem como `ignorado: ...` no status). Vazio, grava todas: é como o professor grava a turma inteira |

O horário do ponto é o da chegada no Node-RED; `ts_ms` é só referência.

**Na sala (decisão de 12/11):** os grupos conectam no Wi-Fi e no broker do professor, e o
Node-RED do professor tem **um fluxo por grupo** (`MEU_DEVICE` = a placa do grupo; o InfluxDB
do grupo). O grupo entrega o nome da placa e URL, org, bucket e token do seu InfluxDB. O
[`gerar_fluxos_turma.py`](FluxoNodeRED-MultiClass/gerar_fluxos_turma.py) monta todas as abas
de uma vez a partir de um `turma.csv` (modelo em `turma.exemplo.csv`) e, com `--enviar`, grava
no Node-RED pela API admin, com os tokens (a importação pelo menu não traz credenciais). Quem
tem a plataforma no próprio notebook importa o fluxo, escreve o `MEU_DEVICE` e aponta o broker
para o IP da sala. O `turma.csv` tem tokens: não vai para o repositório (está no `.gitignore`).

## Payload

Tópico `FIAPIoT/motor/multiclasse` — 1 JSON por janela:

```json
{
  "device": "DEFINA_UM_NOME",
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

`rótulos`: `operando` / `inclinado_frente` / `inclinado_tras` / `anomalia`. `rodada` = volta
completa pela sequência de classes. `ts_ms` = tempo do ESP32 em ms desde o boot (`millis()`).

Os valores estão em **`g`**: o FastIMU devolve aceleração em `g` e o firmware publica o número
cru. Nivelado, `mean_az` fica perto de `1.0`; inclinado 25°, parte da gravidade migra para
`mean_ax` (`sen 25° ≈ 0,42`).

## Treino

[`colab/treinamento_multiclasse.ipynb`](colab/treinamento_multiclasse.ipynb) lê estas janelas do
InfluxDB (measurement `vibracao_multiclasse`) e gera o `modelo_motor_multiclasse.pkl`, que a
**app seguinte de inferência usando IMU** carrega para responder no MQTT.

## Outras versões deste app

Mesmas features, tópico e measurement — os dados entram neste mesmo treino. São placas
prontas do professor, fora do conteúdo da aula: servem ao aluno que não conseguir montar a
própria.

- `app17-8-MultiClassAccFeaturesInfluxDevkitv1MOTOR`: rig com um mini motor com hélice, ligado
  pelo próprio ESP32 por ponte H enquanto a coleta roda.
- `app17-10-MultiClassAccFeaturesInfluxESP32C6`: para a placa ESP32-C6-DevKitC-1.
- `app17-11-MultiClassAccFeaturesInfluxESP32C3SERVO`: num ESP32-C3 Super Mini Plus com um
  microservo que vai e volta enquanto a coleta roda.
