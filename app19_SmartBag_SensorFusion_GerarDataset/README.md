# app19 — Smart Delivery Bag: sensor fusion

Coletar → rotular → treinar RF e MLP → exportar os modelos.

O app15 fechou a parte de **plataforma**: o NexoLog media, publicava e obedecia a um
limiar desenhado no Node-RED. Daqui em diante o limiar sai de cena. A mesma bag, com dois
sensores a mais, passa a gerar um **dataset rotulado por gente**, e quem decide vira um
modelo treinado.

| Pasta | Uso |
|---|---|
| `device/` | ESP32, PlatformIO e Wokwi |
| `NodeRED/` | MQTT → InfluxDB. Só transporte, sem decisão e sem dashboard |
| `colab/app19_coleta_e_rotulagem.ipynb` | Consulta SQL, rótulos e CSV |
| `colab/app19_treinamento_smartbag_rf.ipynb` | RF: `.pkl`, header micromlgen e scaler |
| `colab/app19_treinamento_smartbag_mlp.ipynb` | MLP: `.tflite`, header e scaler |

Sem n8n e sem Grafana neste app. Aqui não há notificação para mandar nem histórico para
mostrar — o destino do dado é um CSV de treinamento. O n8n volta no app20, como ponte da
inferência.

## A trilha

| App | O que roda | Onde |
|---|---|---|
| **app19** | coleta e treinamento | ESP32 + Colab |
| app20 | a **mesma RF**, servida por API | FastAPI, via n8n |
| app21 | a **mesma RF**, embarcada | ESP32, micromlgen |
| app22 | a MLP, embarcada | ESP32, TFLite |

São dois eixos, um de cada vez. **app20 → app21** troca *onde* o modelo roda, mantendo o
modelo. **app21 → app22** troca *o modelo*, mantendo a borda. Se os dois mudassem juntos,
nenhuma comparação diria nada.

## Executar

1. Abra `device/` no PlatformIO e compile com `pio run`. Para montar o firmware em
   quatro iterações: [CONSTRUIR-O-FIRMWARE.md](device/CONSTRUIR-O-FIRMWARE.md).
2. Entre no diretório `IoT-platform` que você recebeu e suba a plataforma com
   `docker compose up -d`. Configure Wi-Fi/broker no `.ino`: no Wokwi com gateway
   local, `host.wokwi.internal`.
3. Importe `NodeRED/Fluxo_2_envio_InfluxDB.json` e configure broker, URL, organização,
   bucket e token. Para montar em vez de importar:
   [CONSTRUIR-O-FLUXO.md](NodeRED/CONSTRUIR-O-FLUXO.md). Measurement:
   `smartbag_raw_adc_2026`. O destino é o **InfluxDB Cloud**, não o `influxdb:2.7` da
   IoT-platform: o Colab consulta com SQL, que só existe no v3. Da plataforma, este app
   usa o Mosquitto e o Node-RED.
4. Inicie o Wokwi. Começa na rodada 1; o baseline é calculado ao iniciar a primeira
   coleta da rodada, com a bag fechada e parada.
5. Os dois botões:
   - **COLETA (GPIO 27):** inicia/para a coleta da situação selecionada. LED aceso = coletando.
   - **SITUAÇÃO (GPIO 26):** avança para a próxima situação, só com a coleta parada.
6. Duas rodadas completas das sete situações, **sem reiniciar o ESP32**. O protocolo está
   em [Guia-SmartBag.md](Guia-SmartBag.md).
7. No Colab de coleta, selecione a equipe e o intervalo, confira as contagens por
   rodada/situação e baixe `smartbag_dataset.csv`.
8. Envie **o mesmo CSV** aos dois Colabs de treinamento.

Uma rodada é uma volta completa pelas sete situações, como no app17-7. Depois da última,
o botão azul volta à primeira e incrementa `rodada`. Parar e iniciar mantém rodada e
situação e reinicia os máximos. O Serial só exibe dados — este firmware não assina nada.

## Dados

```text
temperatura, umidade, delta_distancia, luz, mov_max, incl_max
```

Uma amostra por segundo, **só durante a coleta**. DHT atualizado a cada 2,5 s; MPU
amostrado a cada 50 ms. Distância em cm (delta em relação ao baseline, com sinal), luz em
RAW de 0 a 4095, movimento em m/s² e inclinação em graus.

O JSON leva as seis features e apenas `device`, `rodada` e `situacao`. Sem contadores de
qualidade, validade ou tempo. `coletando` fica só no firmware, para o LED e a publicação.

Sem eco, `delta_distancia` é `null`; distância igual ao baseline dá zero. Os máximos
também ficam `null` se não houve leitura do MPU no intervalo. O Node-RED grava o que
chegou e o Influx pula campo nulo; o Colab remove as linhas incompletas.

## Classes

```python
CLASSES = ["ENTREGA_OK", "REVISAR_ENTREGA"]   # 0 e 1
```

O índice **é** o código. É o inteiro que o `predict()` do micromlgen devolve no app21, e
o lado em que a sigmoide da MLP decide no app22. Os notebooks gravam esse mapa nos
metadados; os firmwares de inferência leem dali, não de memória.

## Saídas dos Colabs

- **RF** → `smartbag_rf.zip`: `modelo_smartbag.pkl` (um Pipeline, com o scaler dentro),
  `AIoTRandomForest_micromlgen.hpp`, `AIoTSmartBagScaler.hpp`, metadados e amostras.
  O notebook compila o header no próprio Colab e compara com o scikit-learn antes de
  liberar o download.
- **MLP** → `smartbag_mlp.zip`: `modelo_smartbag.tflite`, `modelo_smartbag.h`,
  `AIoTSmartBagScaler.hpp`, metadados e conferência. Rede 6 → 8 → 1.

**21 árvores, número ímpar.** O `vote.jinja` do micromlgen desempata com `>` estrito: num
empate vence `votes[0]`, que aqui é `ENTREGA_OK`. Com número par, um empate decidiria
silenciosamente pelo lado errado.

**O scaler sai dos dois notebooks com os mesmos números**, porque o split e o
`StandardScaler` são os mesmos. Os dois imprimem média e escala justamente para você
conferir. Na MLP o scaler é necessidade — `luz` chega a 4095 e `mov_max` fica abaixo de
20, e sem normalizar a luz domina o gradiente. Na RF ele está ali por simetria com o
app22 e com o app30: árvore compara uma feature por vez com um limiar, e escala não muda
a ordem. O que não pode é treinar numa escala e inferir em outra.

Os notebooks não trazem dataset nem modelo pré-treinados. Como no app17-7, a última
rodada fica no teste e as anteriores no treino, igual nos dois modelos. Ao reiniciar o
ESP32, `rodada` volta a 1: não misture execuções no mesmo CSV.

## Referências

Firmware e plataforma vêm do app15. O protocolo de botões, rodada e split por grupo vem
do app17-7; o padrão de API e a pinagem de versões, do app18. Como extensão, o app30 traz
RF embarcada com scaler e o app31 traz MicroTFLite.

LDR lido direto com `analogRead()`, inclusive nos extremos 0 e 4095. O controle do
[componente Wokwi](https://docs.wokwi.com/parts/wokwi-photoresistor-sensor) continua em
lux, mas o firmware publica RAW — e neste módulo mais luz **reduz** o RAW. A measurement
`smartbag_raw_adc_2026` separa esta coleta dos ensaios antigos em lux; não misture os
dois num CSV só. Exportação RF com
[micromlgen](https://github.com/eloquentarduino/micromlgen) e MLP pelo
[conversor TensorFlow Lite](https://www.tensorflow.org/lite/models/convert/convert_models).
