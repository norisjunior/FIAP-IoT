# app16-0b-SPS30Sozinho

Só o SPS30, numa segunda ESP32-C3 Super Mini, sem o SGP30 e o BME280. Serve para confirmar
que o sensor e as leituras estão certos com nada mais no barramento.

**Pronto quando:** aparece `tipo 00080000`, nenhum `ALARME`, e o vapor faz o `pm` e o `nc`
subirem.

## Ligação

| SPS30 (posição no conector) | Vai para (ESP32-C3) |
|---|---|
| 1 VDD | **5V** |
| 2 SDA | GPIO 4 |
| 3 SCL | GPIO 3 |
| 4 SEL | G (escolhe o modo I2C; solto, ele fala UART e não responde) |
| 5 GND | G |

- **Alimentação no 5V, pull-up no 3.3:** o SPS30 é alimentado pelo 5V. O pull-up é outra
  coisa: um resistor em cada fio de dados (SDA e SCL), que puxa a linha para 1 quando ninguém
  a puxa para 0. Na caixa, as placas do SGP30 e do BME280 já trazem esses resistores; aqui
  elas não estão.
- **Sem resistores, teste primeiro:** o `Wire.begin` liga o pull-up interno do ESP32. Ele é
  fraco, e a linha demora a voltar para 1. Por isso este app usa o I2C a **10 kHz**, e não
  a 100 kHz como a caixa: cada bit dura mais, e a subida lenta deixa de atrapalhar. A coleta
  continua a 1 Hz, e cada leitura leva uns 60 ms. Se mesmo assim aparecer `SPS30 FALHOU` ou
  `ERRO de leitura`, ponha 10 kΩ do GPIO 4 ao 3.3 e outro de 10 kΩ do GPIO 3 ao 3.3.
- **Resistor no 3.3, nunca no 5V:** os pinos do ESP32 não aguentam 5 V. O SPS30 aceita
  dados em 3,3 V.
- **Fios do SPS30:** a cor não é padrão. Identifique cada fio pela posição no conector.
- **GPIO 4 e 3** ficam na mesma fileira do 5V, G e 3.3: tudo sai de um lado da placa.

## O que aparece

No boot, uma linha só:

```
SPS30 ok | tipo 00080000 | serie XXXXXXXXXXXXXXXX | firmware 2.3
```

Ela sai uma vez só. Para vê-la, reinicie a placa pelo monitor (Ctrl+T, Ctrl+D, duas vezes),
como explica a seção "Monitor serial desde o boot" do README da pasta de cima. O ventilador do SPS30
só gira depois dessa linha: parado, ele está esperando o comando de medir.

Se o SPS30 não inicia, o programa tenta de novo a cada 2 s e diz por quê:

| Mensagem | O que conferir |
|---|---|
| `SPS30 FALHOU: 0x69 nao responde` | SEL no G, VDD no 5V, SDA no 4, SCL no 3, nessa ordem. Depois de mexer na ligação, tire e recoloque a USB: o RST reinicia só o ESP32, e o SPS30 continua ligado |
| `SPS30 FALHOU: 0x69 responde, mas a leitura falha` | faltam os pull-ups de 10 kΩ |

Depois, 1 vez por segundo, no Teleplot:

| Variável | O que é |
|---|---|
| `pm1_0`, `pm2_5`, `pm4_0`, `pm10` | massa de partículas até 1; 2,5; 4 e 10 µm (µg/m³) |
| `nc0_5`, `nc1_0`, `nc2_5`, `nc4_0`, `nc10` | contagem de partículas até 0,5; 1; 2,5; 4 e 10 µm (por cm³) |
| `tamanho` | tamanho típico das partículas (µm) |

E, só quando acontece:

| Mensagem | O que é |
|---|---|
| `ALARME: rotacao do ventilador fora da faixa` | ventilador rápido ou devagar demais |
| `ALARME: falha no laser` | corrente do laser fora da faixa |
| `ALARME: ventilador parado` | ventilador ligado, mas sem girar |
| `ERRO de leitura` | a resposta do SPS30 não chegou inteira: pull-ups, cabo ou solda |

- Os alarmes vêm do registrador de status do SPS30, que só existe a partir do firmware 2.2 (a
  linha do boot mostra a versão).
- Um alarme que aparece uma vez e some não é defeito. Defeito é alarme repetido.
- **LED RGB da placa:** verde quando a leitura chegou e não há alarme; vermelho quando o
  SPS30 não responde, a leitura falha ou há alarme. Na Super Mini sem RGB (LED azul no
  GPIO 8), ignore o LED: o Serial diz o mesmo.

## Como saber se a leitura está certa

1. **Identificação:** `00080000` é o código do SPS30. Se ele aparece, o I2C funciona. Cada
   resposta do sensor traz um CRC, e a biblioteca descarta resposta corrompida.
2. **Ventilador:** dá para ouvir um zumbido baixo, e nenhum `ALARME` aparece.
3. **Ordem:** sempre `pm1_0 ≤ pm2_5 ≤ pm4_0 ≤ pm10` e
   `nc0_5 ≤ nc1_0 ≤ nc2_5 ≤ nc4_0 ≤ nc10`, porque cada faixa inclui as menores (o PM2,5
   contém o PM1).
4. **Ar parado:** valores baixos e estáveis, que mudam pouco de um segundo para o outro.
5. **Vapor:** uma chaleira recém-fervida, de lado e um pouco abaixo do sensor, nunca com o
   sensor em cima (a água condensa dentro dele). `pm` e `nc` sobem em poucos segundos e
   voltam ao nível de antes em menos de 1 minuto, porque as gotas evaporam.
6. **Fumaça:** um fósforo apagado, a alguns palmos. O `nc0_5` dispara e o `tamanho` fica
   abaixo de 1 µm. A fumaça tem partículas mais finas que as gotas do vapor.
7. **De volta à caixa:** recoloque o SPS30 na caixa e rode a leitura isolada no mesmo ar. O
   `pm2_5` e o `nc0_5` devem ficar parecidos com os daqui. Se aqui dá certo e na caixa não, o
   problema está no barramento compartilhado (pull-ups, cabo, solda), e não no sensor.
