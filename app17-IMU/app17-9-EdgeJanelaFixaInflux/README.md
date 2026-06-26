# app17-9-EdgeJanelaFixaInflux — Forma 1: features no edge (janela fixa)

Firmware do ESP32 que calcula as **features no próprio dispositivo** (Edge Analytics) usando
**janela fixa de 100 amostras (1 s)** a 100 Hz, e publica **1 mensagem MQTT por janela**.
É a evolução direta do `app17-8-MotorML` — mesma base, mesmas libs, mesmo estilo.

> **Forma 1 (edge):** o ESP32 envia só o resumo da janela (features). É eficiente, mas
> **perde o sinal bruto** — não dá para re-janelar ou testar outras features depois. Quando
> você quiser o raw, use a **Forma 2** (`app17-10-RawSerialCSV`).

## Montagem (mínima, sem motor)

Só **ESP32 + MPU6050 sobre a mesa**, 2 botões e 1 LED. **Não há motor.** O "ativo monitorado"
é a própria mesa (Aula 14, slide 2):

- **NORMAL** = MPU6050 **parado** sobre a mesa.
- **ANOMALIA** = **tapas na mesa** enquanto a coleta acontece.

| Função | GPIO |
|---|---|
| MPU SDA | 19 |
| MPU SCL | 18 |
| Botão **COLETA** (inicia/para o envio) | 26 |
| Botão **ANOMALIA** (seleciona NORMAL/ANOMALIA, só com a coleta parada) | 25 |
| LED (aceso = coletando ANOMALIA) | 27 |

> O `diagram.json` traz os **2 botões** e o **LED** com os pinos corretos (o diagram antigo do
> repo estava sem botões e com pinos trocados; o motor saiu de vez).

## O que ele faz (item da Sprint)

- Atende a **Forma 1 / Sprint 4 – item 1 (base analítica)**: features extraídas **por janela**.
- A **visualização** e o **modelo + feature importance** ficam nos notebooks `1.3` e `1.5`,
  que leem do InfluxDB.

## Como rodar

1. **Suba a IoT-platform** da disciplina no WSL (ver [`IoT-platform/README.md`](../../IoT-platform/README.md)).
2. **Compile e simule** (Wokwi pela extensão do VS Code, ou grave no ESP32 físico).
   - Wokwi: já vem com WiFi `Wokwi-GUEST` + broker `host.wokwi.internal` (padrão no `.cpp`).
   - Físico: descomente o bloco **(B)** no topo do `.cpp` e ajuste WiFi + IP da máquina.
3. **Importe e dê deploy** no fluxo Node-RED
   [`analytics/forma1_nodered/flow_features_influx.json`](../analytics/forma1_nodered/),
   configurando o nó InfluxDB nuvem (url/org/token/bucket).
4. Abra os notebooks **1.3** (visualização) e **1.5** (Random Forest + export micromlgen).

### Protocolo de coleta (Aula 14, slide 22)

1. Com a coleta **parada**, escolha a condição no **botão 25** (NORMAL ou ANOMALIA).
2. **Botão 26** inicia a coleta. Mantenha a condição **estável** o tempo todo:
   - NORMAL → não encoste no MPU/mesa.
   - ANOMALIA → **dê tapas na mesa** continuamente enquanto coleta.
3. Colete bastante (alvo da Sprint: ~1 min/condição) e pare no **botão 26**.
4. Faça **rodadas separadas** e nomeie a tag `coleta` de forma incremental:
   `normal_01`, `normal_02`, `anomalia_01`, `anomalia_02`… — é isso que permite o split
   treino/teste por coleta (sem vazamento).

> No simulador, "tapas na mesa" = **chacoalhar o MPU6050** no painel do Wokwi.

## Timestamp (hora da Internet via NTP)

No boot, o firmware **sincroniza o relógio por NTP** (UTC) e passa a enviar
`ts_epoch_ms` = **base NTP + `millis()`** decorrido. Assim cada janela carrega a **hora real**,
sem precisar de RTC dedicado (em sala discutimos a importância de sincronizar relógios).
Se o NTP falhar (ex.: físico sem Internet), `ts_epoch_ms` vira o uptime e a ponte usa o horário
de recepção como fallback.

## Payload publicado

Tópico `FIAPIoT/motor/features` — **1 JSON por janela** (label vem do botão 25):

```json
{
  "device": "IoTDeviceNorisEdgeJanelaFixa001",
  "label": "ligado_anomalia",
  "ts_epoch_ms": 1749760205123,
  "mean_ax": 0.021, "mean_ay": -0.015, "mean_az": 9.812,
  "std_ax": 1.402,  "std_ay": 1.187,  "std_az": 0.945,
  "rms_ax": 1.403,  "rms_ay": 1.190,  "rms_az": 9.857,
  "rms_mag": 10.12
}
```

- `ts_epoch_ms` = epoch em **ms, UTC** (NTP + millis). A ponte usa esse valor como tempo do ponto
  no InfluxDB. Mesmo assim há latência de rede entre **medir** e **gravar** (Aula 14, slide 23).
- Os notebooks usam **7 features**: `mean_ax/ay/az`, `std_ax/ay/az`, `rms_mag`. As demais (`rms_ax/ay/az`)
  vão no payload por herança do app17-8 e ficam disponíveis caso você queira experimentar.
- **Rótulos:** `ligado_normal` / `ligado_anomalia`. Sem motor, "ligado" significa "coletando".
  Enquanto a coleta está parada, o dispositivo não envia janelas (estado `parado`).
- A tag gravada no InfluxDB é o **`label`** (do botão 25). O fluxo Node-RED apenas o repassa
  como tag; não há tag `coleta`/rodada numerada por enquanto.

## Limitações

- **Edge perde o raw:** só as features são transmitidas/armazenadas.
- **Anomalia simulada:** tapas na mesa (ou chacoalho no Wokwi) **não substituem** um ensaio de
  vibração industrial; servem para aprender o pipeline.
- **Timestamp:** mesmo com NTP, há latência rede ESP32→PC; o instante de medição e o de gravação
  não são idênticos.
- **`label` por botão:** troque a condição só com a coleta **parada**, para não misturar janelas.
