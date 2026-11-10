# app-Fumaca-SensorFusion (provisório)

Caixa de fumaça do ID 13: **SGP30** (gás) + **SPS30** (partículas) + **BME280**
(temperatura, umidade e pressão) num **ESP32-C3 Super Mini** (Plus, antena externa).

Pasta provisória. Quando o firmware estiver validado na bancada, este programa vira o
`app21-0-VerificaSensores` (ver `GUIA-CODIGO-SENSOR-FUSION.md`).

## Os programas desta pasta

Grave na ordem. Cada um só faz sentido depois que o anterior funcionou.

| Programa | Onde | Pronto quando |
|---|---|---|
| app21-0, verificador | esta pasta (`src/app.ino`) | mostra 0x58, 0x69 e 0x76; LED verde |
| app21-1 | `app21-1-LeituraIsolada/` | cada sensor no plotter; o álcool mexe só no SGP30 |
| app21-2 | `app21-2-Fusao1Hz/` | cabeçalho + 1 linha por segundo |

Quando tudo funcionar na bancada, a pasta vira `app21-Fumaca-GerarDataset-Escalares`, e o
verificador vai para `app21-0-VerificaSensores/`.

## Ligação

Os 3 sensores dividem o mesmo barramento I2C: os fios SDA de todos se encontram no GPIO4,
e os SCL, no GPIO3.

| Sensor | Pino do sensor | Vai para (ESP32-C3) |
|---|---|---|
| SGP30 | VIN | **3.3** (nunca 5V: os resistores de pull-up da placa puxariam o I2C para 5 V) |
| | GND | G |
| | SDA | GPIO 4 |
| | SCL | GPIO 3 |
| BME280 (versão 3,3 V) | VCC | **3.3** (esta placa não tem regulador: 5 V queima) |
| | GND | G |
| | SDA | GPIO 4 |
| | SCL | GPIO 3 |
| | CSB | 3.3 (escolhe o modo I2C) |
| | SDO | G (endereço 0x76) |
| SPS30 | 1 VDD | **5V** |
| | 2 SDA | GPIO 4 |
| | 3 SCL | GPIO 3 |
| | 4 SEL | G (escolhe o modo I2C; solto, ele fala UART e some do I2C) |
| | 5 GND | G |

- **Pull-up:** as placas do SGP30 e do BME280 já trazem resistores de pull-up para 3,3 V.
  Não é preciso colocar outros.
- **Fios do SPS30:** a cor não é padrão. Identifique cada fio pela posição no conector
  (1 a 5, figura "pin assignment" do datasheet).
- **O cabo do SPS30** pede I2C curto. Se houver erro de leitura, encurte o cabo.

### Por que GPIO 4 e 3

- **GPIO 8:** é o LED RGB da placa.
- **GPIO 9:** é o botão BOOT.
- **GPIO 2:** é pino de boot.
- **4 e 3:** ficam na mesma fileira de 5V, G e 3.3, voltada para os sensores. Tudo o que
  a caixa precisa sai de um lado só da placa.

## Saída esperada

```
Enderecos encontrados: 0x58 0x69 0x76
  SGP30  (0x58) ok
  SPS30  (0x69) ok
  BME280 (0x76) ok
```

O LED RGB da placa fica **verde** quando os 3 respondem e **vermelho** quando falta algum.
A mensagem de cada sensor diz o que conferir.

- **BMP280 no lugar do BME280:** o verificador também lê o ID do chip. O BMP280 é parecido e
  usa o mesmo endereço 0x76, mas não mede umidade; às vezes vem vendido como BME280. Se
  aparecer `0x76 responde, mas o chip nao e BME280 (ID 0x58...)`, a placa é um BMP280 e
  precisa ser trocada: sem umidade, o SGP30 fica sem compensação e o dataset perde a coluna
  `umid`.
