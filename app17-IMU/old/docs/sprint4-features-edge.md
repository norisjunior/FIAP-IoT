# Sprint 4 — Janelas, features e Edge Analytics

## Objetivo

Nesta Sprint, a equipe deve transformar dados de aceleração em features por janela e comparar duas formas de análise:

1. Edge Analytics no ESP32;
2. Analytics no Colab a partir dos dados raw.

O foco é entender como uma série temporal vira um dataset tabular para classificação.

## Continuidade com a Sprint 3

Na Sprint 3, a equipe coletou dados raw:

timestamp,ax,ay,az,label,coleta_id

Na Sprint 4, esses dados serão transformados em janelas e features.

Além disso, será construída uma alternativa em que o próprio ESP32 calcula as features antes de enviar os dados.

## Forma 1 — Edge Analytics no dispositivo

O ESP32 deve:

1. coletar aceleração X, Y e Z;
2. organizar as leituras em janelas;
3. calcular features no próprio dispositivo;
4. adicionar timestamp;
5. adicionar coleta_id;
6. adicionar label;
7. enviar as features via MQTT;
8. permitir armazenamento no InfluxDB.

## Forma 1.1 — Janela fixa

Parâmetros:

- taxa: 100 Hz;
- intervalo: 10 ms;
- janela: 1 segundo;
- tamanho da janela: 100 amostras;
- sobreposição: nenhuma.

Cada janela gera uma mensagem MQTT.

## Forma 1.2 — Janela com sobreposição

Parâmetros:

- taxa: 100 Hz;
- intervalo: 10 ms;
- janela: 2 segundos;
- tamanho da janela: 200 amostras;
- passo: 1 segundo;
- deslocamento: 100 amostras;
- sobreposição: 50%.

Cada janela gera uma mensagem MQTT.

## Forma 2 — Analytics no Colab

Na Forma 2, o ESP32 envia apenas raw via Serial:

ax,ay,az

O Python no computador gera:

timestamp,ax,ay,az,label,coleta_id

Depois o Colab:

1. carrega dataset_raw.csv;
2. visualiza a série temporal;
3. cria janelas fixas;
4. cria janelas sobrepostas;
5. calcula as mesmas features da Forma 1;
6. gera dataset de features;
7. treina uma rede neural básica.

## Features obrigatórias

Calcular as seguintes features.

### Por eixo

Para ax, ay e az:

- mean;
- std;
- rms;
- min;
- max;
- p2p.

Exemplo:

- mean_ax;
- std_ax;
- rms_ax;
- min_ax;
- max_ax;
- p2p_ax.

### Magnitude

Calcular a magnitude por amostra:

mag = sqrt(ax² + ay² + az²)

Depois calcular:

- rms_mag;
- energy_mag;
- jerk_mag;
- crest_mag.

## Definições das features

### mean

Média da janela.

Ajuda a observar tendência central, orientação e influência da gravidade.

### std

Desvio padrão.

Ajuda a observar instabilidade e variação do sinal.

### rms

Raiz da média dos quadrados.

Ajuda a capturar intensidade efetiva da vibração.

### min e max

Valores mínimo e máximo da janela.

Ajudam a observar extremos.

### p2p

Pico a pico.

p2p = max - min

Ajuda a detectar amplitude de movimento e impactos.

### rms_mag

RMS da magnitude.

Resume a intensidade geral do movimento considerando os três eixos.

### energy_mag

Energia da magnitude.

Soma dos quadrados da magnitude ou da magnitude centralizada, conforme implementação documentada.

### jerk_mag

Variação média ou RMS da mudança entre magnitudes consecutivas.

Ajuda a detectar trancos e mudanças bruscas.

### crest_mag

Relação entre pico e RMS da magnitude.

Ajuda a detectar impacto isolado.

## Labels

Usar apenas:

- normal;
- vibracao.

## Timestamp

Para Edge Analytics com Wi-Fi, preferir timestamp via NTP.

Para Wokwi ou ambiente sem Internet, permitir timestamp relativo com millis().

Documentar a limitação.

## MQTT e InfluxDB

Usar arquitetura:

ESP32 → MQTT Broker → Telegraf → InfluxDB

Tópico sugerido:

iot/vibracao/features

Payload pode ser JSON ou line protocol, desde que o README explique como o Telegraf interpreta.

Preferência didática:

- payload JSON simples;
- Telegraf convertendo para InfluxDB.

## Colab — InfluxDB

Criar notebook para:

1. conectar no InfluxDB;
2. carregar as features;
3. converter para DataFrame;
4. visualizar séries temporais de features;
5. comparar normal vs vibracao.

Atenção:

Se o ESP32 enviou apenas features, o Colab não deve prometer visualizar raw ax, ay, az.

Ele deve visualizar features ao longo do tempo, como:

- rms_mag;
- std_ax;
- p2p_ax;
- energy_mag;
- crest_mag.

## Colab — ML

Criar notebook para treinar uma rede neural básica.

Usar features como entrada e label como saída.

Modelo sugerido:

- Dense(16, relu);
- Dense(8, relu);
- Dense(1, sigmoid).

Avaliar com:

- acurácia;
- matriz de confusão;
- classification_report.

## Separação treino/teste

Se houver janela com sobreposição, não usar split aleatório simples.

Separar por coleta_id.

Exemplo:

Treino:

- normal_01;
- vibracao_01.

Teste:

- normal_02;
- vibracao_02.

Motivo:

Janelas sobrepostas são muito parecidas. Se forem misturadas aleatoriamente entre treino e teste, o modelo pode parecer melhor do que realmente é.

## Entregáveis

A equipe deve entregar:

1. código ESP32 para janela fixa;
2. código ESP32 para janela com sobreposição;
3. versão física;
4. versão Wokwi, quando aplicável;
5. envio MQTT;
6. configuração de armazenamento no InfluxDB;
7. notebook Colab para visualização;
8. notebook Colab para ML;
9. comparação entre normal e vibracao;
10. breve análise dos resultados.

## Critérios de avaliação

A entrega será avaliada por:

- funcionamento da janela fixa;
- funcionamento da janela com sobreposição;
- cálculo correto das features;
- consistência do dataset;
- uso correto das labels;
- cuidado com separação treino/teste;
- clareza dos gráficos;
- explicação das limitações;
- conexão entre teoria e prática.

## Mensagem central

Dados raw mostram o fenômeno.

Features resumem o fenômeno.

Janelas definem o pedaço observado.

Edge Analytics decide onde o processamento acontece.