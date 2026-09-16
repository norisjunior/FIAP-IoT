# app19 — SmartBag Sensor Fusion

Coletar → rotular → treinar RF e MLP → exportar os modelos.

| Pasta | Uso |
|---|---|
| `device/` | ESP32, PlatformIO e Wokwi |
| `fluxo/NodeRED/Fluxo_2_envio_InfluxDB.json` | MQTT → InfluxDB |
| `colab/app19_coleta_e_rotulagem.ipynb` | Consulta, anotações e CSV |
| `colab/app19_treinamento_smartbag_rf.ipynb` | RF, `.pkl` e header micromlgen |
| `colab/app19_treinamento_smartbag_mlp.ipynb` | MLP, TFLite e normalização |

## Executar

1. Abra `device/` no PlatformIO e compile com `pio run`.
2. Configure Wi-Fi/broker no `.ino`. No Wokwi local: `host.wokwi.internal`.
3. Importe **somente** `Fluxo_2_envio_InfluxDB.json`. Configure broker, URL, organização, bucket e token no Node-RED. O fluxo usa `smartbag_raw_adc_2026`.
4. Inicie o Wokwi. O contador começa na rodada 1; o baseline será calculado ao iniciar a primeira coleta da rodada, com a bag fechada/parada.
5. Use os dois botões:

- **COLETA (verde, GPIO 27):** inicia/para a coleta da situação selecionada. O LED fica aceso enquanto coleta.
- **SITUAÇÃO (azul, GPIO 26):** avança para a próxima situação, somente com a coleta parada.

Uma rodada é uma sequência completa das sete situações, como no app17-7. Após a última, o botão azul volta à primeira e incrementa `rodada`. O próximo início de coleta recalibra o baseline. Parar e iniciar mantém a mesma rodada e situação e reinicia os máximos. O Serial apenas mostra os dados e a situação selecionada.

6. Faça duas rodadas completas sem reiniciar o ESP32. No Colab de coleta, selecione a equipe e o intervalo de uma execução contínua, confira as anotações por rodada/situação e baixe `smartbag_dataset.csv`.
7. Envie **o mesmo CSV** aos dois Colabs de treinamento. Execute cada um do início ao fim.

## Dados

```text
temperatura, umidade, delta_distancia, luz, mov_max, incl_max
```

Uma amostra por segundo enquanto a coleta está ligada. DHT atualizado a cada 2,5 s; MPU amostrado aproximadamente a cada 50 ms. Distância em cm, luz em RAW de 0 a 4095, movimento em m/s² e inclinação em graus.

O JSON contém as seis features e apenas `device`, `rodada` e `situacao` para identificação. Não há contadores de qualidade, validade ou tempo no payload. O estado `coletando` permanece apenas no firmware, para os botões, o LED e a publicação.

LED aceso = coleta em andamento. Sem eco, `delta_distancia` é `null`; distância igual ao baseline produz zero. Os máximos também ficam `null` se não houver leitura do MPU no intervalo. O DHT mantém sua última leitura válida em cache.

O Node-RED grava as features disponíveis. O Colab mantém o `SELECT` SQL, associa situação ao target, remove linhas incompletas e salva o CSV. Não há descarte automático dos primeiros segundos: prepare a situação antes de iniciar e pare a coleta durante as transições.

## Saídas dos Colabs

- **RF:** `smartbag_rf.zip`, com `.pkl`, header C++, metadados e amostras para conferência. O notebook testa a equivalência Python/C++ antes do download do pacote.
- **MLP:** `smartbag_mlp.zip`, com `.tflite`, header, scaler, metadados e amostras de conferência. Rede 6 → 8 → 1; normalização ajustada apenas no treino.

Os notebooks não incluem dataset nem modelos fictícios. Preserve o CSV completo. Como no app17-7, a última rodada fica no teste e as anteriores no treino, tanto na RF quanto na MLP. Ao reiniciar o ESP32, rodada volta a 1; não misture execuções no CSV. Gere um novo CSV com este Colab: os anteriores usavam outro significado para rodada.

Sequência prevista: app20 usa a API RF; app21 usa o header RF; app22 usa os headers MLP/scaler. Os três Colabs permanecem neste app19.

## Referências

NexoLog/app14, coleta e notebooks Vaccine Sense em `OLD-ML-com-IoT`, app30/micromlgen e app31/MicroTFLite. Os arquivos de dashboard e n8n herdados da cópia são referências antigas; não participam desta coleta.

LDR lido diretamente com `analogRead()`, inclusive nos extremos 0 e 4095. O controle do [componente Wokwi](https://docs.wokwi.com/parts/wokwi-photoresistor-sensor) continua em lux, mas o firmware publica apenas RAW. A nova measurement `smartbag_raw_adc_2026` separa os dados dos ensaios antigos em lux; gere um novo CSV para treinar RF e MLP. Exportação RF com [micromlgen](https://github.com/eloquentarduino/micromlgen) e MLP pelo [conversor TensorFlow Lite](https://www.tensorflow.org/lite/models/convert/convert_models).
