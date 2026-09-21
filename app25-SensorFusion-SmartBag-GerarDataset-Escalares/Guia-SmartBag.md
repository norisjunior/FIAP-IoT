# SmartBag — guia de coleta

## Preparar

- HC-SR04 e LDR na parte interna da tampa, voltados para a carga.
- MPU no corpo da bag, com +Z para cima quando ela está na vertical.
- Pinos: DHT 4; TRIG 19; ECHO 18; SDA 22; SCL 23; LED 21; LDR AO 35; botões COLETA 27 e SITUAÇÃO 26 → GND (`INPUT_PULLUP`).
- No Wokwi, comece com distância 10 cm, LDR 10 lux e MPU parado (+Z = 1 g).

Na placa real, confira alimentação e níveis elétricos: o ECHO de um HC-SR04 alimentado a 5 V precisa de adaptação para o ESP32. A feature luz é RAW (0–4095); preserve a montagem do LDR em coleta e inferência.

## Duas rodadas, sete situações

O contador começa em rodada 1. Cada rodada reúne as sete situações. Feche e imobilize a bag antes de apertar COLETA pela primeira vez: o firmware calcula o baseline com cinco leituras válidas. Se falhar, continua publicando com delta `null`; pare, corrija a montagem e inicie novamente para tentar o baseline.

Aperte COLETA para iniciar a coleta da situação selecionada (LED aceso) e novamente para parar (LED apagado). Com a coleta parada, aperte SITUAÇÃO para avançar na tabela. Prepare a situação antes de iniciar e colete por aproximadamente 60 s. Pare antes de alterar os sensores; não há descarte automático dos primeiros segundos. O Serial mostra rodada e situação, sem receber comandos. Reiniciar a coleta sem apertar SITUAÇÃO retoma o mesmo ensaio na mesma rodada, zerando os máximos.

Anote qualquer diferença entre a situação prevista e a execução, para ajustar ou descartar aquele par rodada/situação no Colab. Repetições do mesmo par compartilham a anotação. O firmware identifica a situação; o target continua sendo atribuído pela anotação.

| Ordem / situação | Execução | Target esperado |
|---|---|---|
| `parada_fechada` | Bag fechada e parada | ENTREGA_OK |
| `transporte_normal` | Movimento comum, inclinação moderada | ENTREGA_OK |
| `buraco` | Transporte com poucos impactos pontuais | ENTREGA_OK |
| `aberta_parada` | Abrir para entregar, pouco movimento | ENTREGA_OK |
| `aberta_movimento` | Abertura durante movimento | REVISAR_ENTREGA |
| `tombamento` | Bag tombada, com movimento variado | REVISAR_ENTREGA |
| `problema_termico` | Condição térmica inadequada do ensaio | REVISAR_ENTREGA |

Repita as sete em uma segunda rodada, variando carga e intensidades. Após a última situação, aperte SITUAÇÃO com a coleta parada para voltar à primeira e incrementar rodada. O baseline será recalculado no próximo início de coleta. Não há toque longo.

Não reinicie o ESP32 entre as duas rodadas. Ao reiniciar, rodada volta a 1. No Colab, informe início/fim (com fuso horário) de uma execução contínua e selecione o device da equipe. Não junte CSVs de reinicializações diferentes sem identificar os grupos separadamente. Descarte rodadas incompletas antes de treinar.

No Wokwi, altere os controles dos sensores para representar cada situação. O simulador não acopla automaticamente abertura, distância e luminosidade: ajuste HC-SR04 e LDR juntos quando simular abertura. Altere componentes da aceleração para inclinar, conservando aproximadamente 1 g quando representar inclinação estática.

O ajuste de luminosidade do Wokwi está em lux, mas o JSON contém RAW de 0 a 4095. No módulo usado, aumentar a iluminação reduz o RAW. Não misture CSVs antigos em lux com os novos.

## Evitar atalhos

- Faça impactos altos também em `buraco`, que é OK.
- Faça valores de luz e delta semelhantes tanto na entrega parada quanto na abertura em movimento.
- Varie iluminação e temperaturas também nas situações normais.
- Inclinações moderadas devem aparecer em transporte normal e nas transições para tombamento.
- A coleta de impacto não deve ficar inteira sob impacto contínuo.

Antes do ensaio térmico, defina e anote com a turma a condição da carga e o que constitui problema. Não invente um limite universal. As seis entradas não medem duração de exposição: não use como target uma regra que só seria conhecida por um histórico ausente do modelo.

Os rótulos representam a execução observada. Não mude medições nem rótulos para obter sobreposição artificial. Se houver um atalho por uma feature, colete contrapontos plausíveis. Se duas situações produzirem as mesmas seis entradas e respostas diferentes, existe ambiguidade que o modelo não poderá resolver perfeitamente.

## Conferir os sensores

Com baseline de 10 cm, distância de 15 cm deve produzir delta de +5 cm; 8 cm, delta de −2 cm. Movimento parado deve ficar perto de zero. Inclinação estática: +Z ≈ 0°, eixo horizontal ≈ 90°, −Z ≈ 180°.

O ângulo pelo acelerômetro sofre influência de impactos. Os máximos de um segundo também não preservam a ordem dos eventos. São duas limitações intencionais deste exemplo escalar.

O firmware mantém apenas os máximos de movimento/inclinação a cada segundo e a última leitura válida do DHT. Ausência de eco produz delta `null`; o Colab remove linhas com features ausentes. Não há classificação de qualidade das janelas nesta versão didática.

## Depois da coleta

Abra [coleta_e_rotulagem.ipynb](colab/coleta_e_rotulagem.ipynb), selecione a equipe e o intervalo de uma execução contínua e confira a tabela de anotações antes de exportar. Rode os dois notebooks de treinamento com o mesmo arquivo.

O transporte MQTT -> InfluxDB esta em [NodeRED/CONSTRUIR-O-FLUXO.md](NodeRED/CONSTRUIR-O-FLUXO.md); o firmware, em [device/CONSTRUIR-O-FIRMWARE.md](device/CONSTRUIR-O-FIRMWARE.md).

Duas rodadas são o mínimo didático. Para ajustar configurações e avaliar generalização, colete rodadas completas adicionais e preserve o teste. As acurácias do exemplo sintético anterior não são resultados desta coleta.

A última rodada completa fica no teste, como no app de coleta do motor. As demais ficam no treino. Esse mesmo split é usado nos dois notebooks.
