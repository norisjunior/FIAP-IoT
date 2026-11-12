# app16-Fumaca-SensorFusion

Caixa de fumaça: **SGP30** (gás) + **SPS30** (partículas) + **BME280** (temperatura,
umidade e pressão) num **ESP32-C3 Super Mini** (Plus, antena externa).

É material do professor: a demonstração ao vivo dos IDs 13 em diante e o dataset escalar
que a turma recebe pronto. O paralelo é com o dataset de fumaça do Kaggle
(`app16-4-Fusao1Hz-MQTT/colab/dataset_detector_fumaca_kaggle.csv`), que tem as mesmas colunas.
Os dois CSV e os notebooks que vão para a turma estão em `app16-4-Fusao1Hz-MQTT/colab/`.

## Os programas desta pasta

Grave na ordem. Cada um só faz sentido depois que o anterior funcionou.

| Programa | Onde | Pronto quando |
|---|---|---|
| app16-0, verificador | esta pasta (`src/app.ino`) | mostra 0x58, 0x69 e 0x76; LED verde |
| app16-0b, só o SPS30 (opcional) | `app16-0b-SPS30Sozinho/` | em outra placa: tipo 00080000, sem alarme; o vapor sobe `pm` e `nc` |
| app16-1 | `app16-1-LeituraIsolada/` | cada sensor no plotter; o álcool mexe só no SGP30 |
| app16-2 | `app16-2-TesteFenomenos/` | com o BOOT marcando o estímulo: o vapor mexe no SPS30 e na umidade, o álcool só no SGP30, a fumaça da vela nos dois, a chama na temperatura |
| app16-3 | `app16-3-Fusao1Hz/` | o coletor do dataset: o painel mostra as curvas, e o `.csv` ganha uma linha por segundo, com as colunas do Kaggle |
| app16-4 | `app16-4-Fusao1Hz-MQTT/` | o app16-3 com MQTT: o Grafana recebe um ponto por segundo (Node-RED → InfluxDB); o Colab treina fire/not fire e prevê a última leitura |

## Ligação

Os 3 sensores dividem o mesmo barramento I2C: os fios SDA de todos se encontram no GPIO20,
e os SCL, no GPIO21.

| Sensor | Pino do sensor | Vai para (ESP32-C3) |
|---|---|---|
| SGP30 | VIN | **3.3** (nunca 5V: os resistores de pull-up da placa puxariam o I2C para 5 V) |
| | GND | G |
| | SDA | GPIO 20 |
| | SCL | GPIO 21 |
| BME280 (versão 3,3 V) | VCC | **3.3** (esta placa não tem regulador: 5 V queima) |
| | GND | G |
| | SDA | GPIO 20 |
| | SCL | GPIO 21 |
| | CSB | 3.3 (escolhe o modo I2C) |
| | SDO | G (endereço 0x76) |
| SPS30 | 1 VDD | **5V** |
| | 2 SDA | GPIO 20 |
| | 3 SCL | GPIO 21 |
| | 4 SEL | G (escolhe o modo I2C; solto, ele fala UART e some do I2C) |
| | 5 GND | G |

- **Pull-up:** as placas do SGP30 e do BME280 já trazem resistores de pull-up para 3,3 V.
  Não é preciso colocar outros.
- **Fios do SPS30:** a cor não é padrão. Identifique cada fio pela posição no conector
  (1 a 5, figura "pin assignment" do datasheet).
- **O cabo do SPS30** pede I2C curto. Se houver erro de leitura, encurte o cabo.

### Por que GPIO 20 e 21

- **GPIO 8:** é o LED RGB da placa.
- **GPIO 9:** é o botão BOOT.
- **GPIO 2:** é pino de boot.
- **20 e 21:** são o RX e o TX da UART0, livres porque o Serial sai pela USB
  (`ARDUINO_USB_CDC_ON_BOOT=1`). No boot, o chip manda o próprio log pelo 21 e o SCL pisca
  por alguns milissegundos. Nenhum sensor reage, porque o SDA fica alto nesse tempo.
- **Diagramas da placa:** alguns trocam TX e RX. No ESP32-C3, o 20 é RX e o 21 é TX.

## Monitor serial desde o boot

O que a placa imprime no boot sai antes de o monitor abrir e se perde. Para ver desde a
primeira linha, reinicie a placa pelo próprio monitor, sem tirar a USB:

1. Abra o monitor: `pio device monitor`, sem `--dtr 0 --rts 0`.
2. Aperte **Ctrl+T** e depois **Ctrl+D**.
3. Aperte **Ctrl+T** e **Ctrl+D** de novo. A placa reinicia, e o monitor mostra tudo desde a
   primeira linha do chip (`ESP-ROM:esp32c3...`).

- **Por que funciona:** Ctrl+T Ctrl+D liga e desliga o sinal DTR da serial. Na USB do
  ESP32-C3, essa troca de sinais reinicia o chip sem desconectar a USB.
- **Sem `--dtr 0 --rts 0`:** com os dois sinais forçados em 0 ao abrir, o atalho não
  reinicia a placa.
- **Teleplot:** não tem esse atalho. Veja o boot no monitor, feche-o e abra o Teleplot.
- **Não use o botão RST para isso:** ele desconecta a USB, e o monitor perde as primeiras
  linhas enquanto reconecta.
- **`while (!Serial)` não resolve nesta placa:** no Windows, o Serial do C3 se diz conectado
  mesmo sem monitor aberto, e a espera passa direto.

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
