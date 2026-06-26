# Checklist de revisão — Aula 14 Edge Analytics

Use este checklist para revisar as entregas geradas pelo Codex.

## 1. Organização

- [ ] A pasta aula14-edge-analytics/ foi criada.
- [ ] Existe README.md principal.
- [ ] Existe pasta infra/.
- [ ] Existe pasta firmware/.
- [ ] Existe pasta python/.
- [ ] Existe pasta colab/.
- [ ] Cada subpasta tem README.md.

## 2. Códigos antigos

- [ ] O Codex consultou app17-IMU/app17-5-JanelaFeatures/.
- [ ] O Codex consultou app17-IMU/app17-2-Coleta/.
- [ ] O Codex consultou app17-IMU/app17-0-Plot/.
- [ ] Nenhum código antigo foi alterado sem autorização.

## 3. Forma 1 — Janela fixa

- [ ] Código para ESP32 físico criado.
- [ ] Código para Wokwi criado ou limitação documentada.
- [ ] Taxa de 100 Hz.
- [ ] Janela de 1 segundo.
- [ ] 100 amostras.
- [ ] Sem sobreposição.
- [ ] Features calculadas no ESP32.
- [ ] Mensagem MQTT por janela.
- [ ] Label incluída.
- [ ] coleta_id incluído.
- [ ] timestamp incluído ou limitação documentada.

## 4. Forma 1 — Janela com sobreposição

- [ ] Código para ESP32 físico criado.
- [ ] Código para Wokwi criado ou limitação documentada.
- [ ] Taxa de 100 Hz.
- [ ] Janela de 2 segundos.
- [ ] 200 amostras.
- [ ] Passo de 1 segundo.
- [ ] Sobreposição de 50%.
- [ ] Features calculadas no ESP32.
- [ ] Mensagem MQTT por janela.
- [ ] window_id incluído.
- [ ] coleta_id incluído.
- [ ] label incluída.
- [ ] timestamp_inicio e timestamp_fim incluídos.

## 5. Features

Verificar se foram calculadas:

- [ ] mean_ax
- [ ] mean_ay
- [ ] mean_az
- [ ] std_ax
- [ ] std_ay
- [ ] std_az
- [ ] rms_ax
- [ ] rms_ay
- [ ] rms_az
- [ ] min_ax
- [ ] min_ay
- [ ] min_az
- [ ] max_ax
- [ ] max_ay
- [ ] max_az
- [ ] p2p_ax
- [ ] p2p_ay
- [ ] p2p_az
- [ ] rms_mag
- [ ] energy_mag
- [ ] jerk_mag
- [ ] crest_mag

## 6. Labels

- [ ] Usa somente normal e vibracao.
- [ ] Não mistura anomalia, anomala, abnormal ou outros nomes.
- [ ] README explica como escolher a label.
- [ ] README explica protocolo de coleta.

## 7. Infraestrutura

- [ ] docker-compose.yml criado.
- [ ] Mosquitto incluído.
- [ ] InfluxDB incluído.
- [ ] Telegraf incluído.
- [ ] telegraf.conf criado.
- [ ] README explica como subir.
- [ ] README explica como testar MQTT.
- [ ] README explica como verificar dados no InfluxDB.

## 8. Forma 2 — Raw Serial

- [ ] ESP32 envia apenas ax,ay,az.
- [ ] Python adiciona timestamp.
- [ ] Python adiciona label.
- [ ] Python adiciona coleta_id.
- [ ] normal.csv gerado.
- [ ] vibracao.csv gerado.
- [ ] dataset_raw.csv gerado.
- [ ] README explica que timestamp é momento de recepção.

## 9. Colab

- [ ] Notebook/script para InfluxDB janela fixa.
- [ ] Notebook/script para InfluxDB janela sobreposta + ML.
- [ ] Notebook/script para CSV raw → features + ML.
- [ ] Visualiza features da Forma 1.
- [ ] Visualiza raw na Forma 2.
- [ ] Não promete raw quando só há features.
- [ ] Usa StandardScaler antes da rede neural.
- [ ] Usa rede neural simples.
- [ ] Mostra matriz de confusão.
- [ ] Mostra classification_report.
- [ ] Separa treino/teste por coleta_id.

## 10. Didática

- [ ] Código é simples.
- [ ] Comentários são úteis.
- [ ] README explica objetivo.
- [ ] README explica limitações.
- [ ] Não há abstrações complexas.
- [ ] A solução parece uma evolução dos códigos anteriores.
- [ ] A resposta-modelo do professor está clara.

## 11. Limitações explicitadas

- [ ] Edge Analytics reduz dados, mas perde raw.
- [ ] Raw permite testar novas janelas.
- [ ] Timestamp no Python é timestamp de recepção.
- [ ] Wokwi pode ter limitações.
- [ ] Janelas sobrepostas exigem separação treino/teste cuidadosa.
- [ ] Tapa na mesa é simulação didática, não ensaio industrial real.