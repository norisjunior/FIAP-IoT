# AGENTS.md — Aula 14 Edge Analytics

## Escopo desta pasta

Esta pasta contém a resposta-modelo da Aula 14 — Da série temporal ao Edge Analytics.

Todo código novo desta aula deve ser criado dentro desta pasta.

Não alterar códigos antigos fora desta pasta sem autorização explícita.

## Referências externas à pasta

Antes de implementar, consultar:

- ../AGENTS.md
- ../docs/codex-referencias.md
- ../docs/aula14-edge-analytics.md
- ../docs/sprint3-coleta-raw.md
- ../docs/sprint4-features-edge.md
- ../app17-IMU/app17-0-Plot/
- ../app17-IMU/app17-2-Coleta/
- ../app17-IMU/app17-5-JanelaFeatures/

## Estrutura esperada

Criar ou manter a seguinte estrutura:

aula14-edge-analytics/
├── README.md
├── infra/
│   ├── docker-compose.yml
│   ├── telegraf.conf
│   └── README.md
├── firmware/
│   ├── 01_edge_janela_fixa_fisico/
│   ├── 02_edge_janela_fixa_wokwi/
│   ├── 03_edge_janela_sobreposta_fisico/
│   ├── 04_edge_janela_sobreposta_wokwi/
│   └── 05_raw_serial/
├── python/
│   ├── coleta_raw_normal.py
│   ├── coleta_raw_vibracao.py
│   ├── unir_csvs_rotular.py
│   ├── requirements.txt
│   └── README.md
└── colab/
    ├── 01_influx_features_janela_fixa.py
    ├── 02_influx_features_sobreposicao_ml.py
    └── 03_raw_csv_para_features_ml.py

## Forma 1 — Edge Analytics

Criar códigos em que o ESP32 calcula features no dispositivo.

### Janela fixa

Pasta:

firmware/01_edge_janela_fixa_fisico/

Parâmetros:

- 100 Hz;
- 10 ms entre amostras;
- janela de 1 segundo;
- 100 amostras;
- sem sobreposição.

### Janela com sobreposição

Pasta:

firmware/03_edge_janela_sobreposta_fisico/

Parâmetros:

- 100 Hz;
- 10 ms entre amostras;
- janela de 2 segundos;
- 200 amostras;
- passo de 1 segundo;
- 100 amostras de deslocamento;
- 50% de sobreposição.

## Forma 2 — Analytics no Colab

Criar código em que o ESP32 envia apenas raw via Serial.

Pasta:

firmware/05_raw_serial/

Saída Serial:

ax,ay,az

O Python deve adicionar:

- timestamp;
- label;
- coleta_id.

## Labels

Usar somente:

- normal
- vibracao

## Wokwi

Criar versões Wokwi quando solicitado.

Se MQTT não for viável no Wokwi, usar saída Serial alternativa e documentar a limitação.

## MQTT e InfluxDB

Usar:

ESP32 → Mosquitto → Telegraf → InfluxDB

Tópico MQTT padrão:

iot/vibracao/features

## Regras de implementação

- Código simples.
- Sem classes desnecessárias.
- Sem abstrações complexas.
- Comentários didáticos.
- README em cada pasta.
- Configurações no início dos arquivos.
- Não esconder limitações.
- Não prometer o que o código não faz.

## Antes de implementar

Sempre apresentar um plano curto antes de criar múltiplos arquivos.

O plano deve dizer:

1. arquivos que serão criados;
2. exemplos antigos consultados;
3. decisões técnicas;
4. limitações;
5. como testar.