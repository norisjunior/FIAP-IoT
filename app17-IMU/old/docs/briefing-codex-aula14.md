# Briefing para o Codex — Aula 14 Edge Analytics

## Pedido principal

Leia obrigatoriamente:

- AGENTS.md
- aula14-edge-analytics/AGENTS.md
- docs/codex-referencias.md
- docs/aula14-edge-analytics.md
- docs/sprint3-coleta-raw.md
- docs/sprint4-features-edge.md

Também consulte, sem alterar:

- app17-IMU/app17-0-Plot/
- app17-IMU/app17-2-Coleta/
- app17-IMU/app17-5-JanelaFeatures/

## Objetivo

Criar uma resposta-modelo completa para o professor demonstrar aos alunos duas formas de resolver coleta e análise de vibração com ESP32 + MPU6050.

## Forma 1 — Edge Analytics no dispositivo

O ESP32 calcula features no dispositivo e envia por MQTT para armazenamento no InfluxDB.

### Forma 1.1 — Janela fixa

- ESP32 físico;
- Wokwi;
- janela de 1 segundo;
- 100 Hz;
- 100 amostras;
- sem sobreposição;
- envio MQTT;
- armazenamento no InfluxDB.

### Forma 1.2 — Janela com sobreposição

- ESP32 físico;
- Wokwi;
- janela de 2 segundos;
- 100 Hz;
- 200 amostras;
- passo de 1 segundo;
- sobreposição de 50%;
- envio MQTT;
- armazenamento no InfluxDB.

## Forma 2 — Analytics no Colab

O ESP32 envia apenas:

ax,ay,az

Um script Python recebe as medições via Serial e salva:

timestamp,ax,ay,az,label,coleta_id

Depois o Colab:

1. visualiza a série temporal raw;
2. cria janelas fixas;
3. cria janelas com sobreposição;
4. calcula as mesmas features da Forma 1;
5. gera dataset de features;
6. treina rede neural básica.

## Features obrigatórias

Calcular:

- mean_ax, mean_ay, mean_az
- std_ax, std_ay, std_az
- rms_ax, rms_ay, rms_az
- min_ax, min_ay, min_az
- max_ax, max_ay, max_az
- p2p_ax, p2p_ay, p2p_az
- rms_mag
- energy_mag
- jerk_mag
- crest_mag

## Labels obrigatórias

Usar somente:

- normal
- vibracao

## InfluxDB

Usar arquitetura:

ESP32 → MQTT Broker → Telegraf → InfluxDB

Criar infraestrutura simples com:

- Mosquitto;
- Telegraf;
- InfluxDB 2.x.

## Rede neural

Usar rede neural básica:

- Dense(16, relu)
- Dense(8, relu)
- Dense(1, sigmoid)

Avaliar com:

- acurácia;
- matriz de confusão;
- classification_report.

## Separação treino/teste

Não usar split aleatório como solução principal quando houver janelas sobrepostas.

Separar por coleta_id.

Exemplo:

Treino:

- normal_01
- vibracao_01

Teste:

- normal_02
- vibracao_02

## Primeira tarefa

Antes de implementar, gere apenas um plano técnico.

O plano deve conter:

1. arquivos que serão criados;
2. estrutura de pastas;
3. exemplos antigos consultados;
4. padrões reaproveitados;
5. diferenças entre Forma 1 e Forma 2;
6. arquitetura MQTT → Telegraf → InfluxDB;
7. organização dos notebooks Colab;
8. estratégia de separação treino/teste;
9. limitações;
10. ordem de implementação.

Não implemente ainda.