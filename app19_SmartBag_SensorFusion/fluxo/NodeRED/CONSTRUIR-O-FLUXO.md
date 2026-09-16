# MQTT → InfluxDB

1. Instale `node-red-contrib-influxdb` pela paleta do Node-RED.
2. Importe `Fluxo_2_envio_InfluxDB.json`.
3. Configure o broker: `localhost` se ambos rodam diretamente na mesma máquina, ou o nome/IP do serviço se estiver em container.
4. No nó InfluxDB, configure URL e token; no nó de saída, organização e bucket. Mantemos escrita compatível com API v2 e consulta SQL do InfluxDB Cloud usado no Vaccine Sense.
5. Faça Deploy e inicie a coleta pelo botão.

Tópico: `FIAPIoT/smartbag/equipe01/dados`. Measurement: `smartbag_raw_adc_2026`.

A função grava `[campos, tags]`: campos são as seis features; tags são device, rodada e situação. O InfluxDB registra o horário de chegada. O firmware só publica durante a coleta, então o fluxo não precisa testar um campo coletando.

Campos `null` são omitidos na escrita, preservando as outras features. O Colab consulta com SQL, rotula pela situação e remove linhas incompletas. A measurement separa luz RAW dos ensaios anteriores em lux.

O dashboard herdado usa o payload antigo e não participa desta etapa.

Rodada é uma volta completa pelas sete situações. No Colab, selecione a equipe e o intervalo de uma execução sem reiniciar o ESP32; os contadores recomeçam após reset. Reimporte o fluxo atualizado e gere um novo CSV, sem juntar dados do contrato anterior.
