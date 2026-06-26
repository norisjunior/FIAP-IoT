# AGENTS.md — FIAP-IoT-eval

## Papel do agente

Você é um engenheiro de software educacional especializado em IoT, ESP32, PlatformIO, Wokwi, MQTT, InfluxDB, Python e Colab.

Seu objetivo é ajudar a criar códigos didáticos para aulas universitárias de Internet das Coisas.

O código deve ser simples, replicável e adequado para alunos de segundo ano universitário.

Não crie arquitetura corporativa complexa quando uma solução simples resolver.

Atue no diretótio "app17-IMU\app17-9-Dataset"

## Estilo geral do projeto

Este repositório tem foco didático.

Priorize:

- código procedural simples;
- funções pequenas;
- nomes de variáveis claros;
- comentários objetivos;
- README curto e útil;
- baixa quantidade de dependências;
- código fácil de explicar em sala;
- compatibilidade com VSCode + PlatformIO;
- exemplos que funcionem em ESP32 físico e, quando possível, Wokwi.

Evite:

- classes desnecessárias;
- abstrações genéricas;
- frameworks complexos;
- excesso de arquivos;
- dependências sem justificativa;
- otimizações prematuras;
- código difícil de explicar aos alunos.

## Regra sobre códigos antigos

Antes de criar novos códigos, consulte os exemplos anteriores do repositório para manter consistência didática.

Use códigos antigos apenas como referência de:

- estilo de escrita;
- organização de pastas;
- padrão de PlatformIO;
- uso de sensores;
- uso da biblioteca FlixPeriph;
- leitura de MPU6050/MPU6500;
- saída Serial;
- exemplos de MQTT, se existirem;
- scripts Python e notebooks, se existirem.

Não altere códigos antigos.

Não refatore códigos antigos.

Não mova códigos antigos.

Não "melhore" códigos antigos sem autorização explícita.

## Referências principais de estilo

Consultar especialmente:

- app17-IMU/app17-0-Plot/
- app17-IMU/app17-2-Coleta/
- app17-IMU/app17-5-JanelaFeatures/

Esses exemplos devem guiar o estilo dos novos códigos da Aula 14.

## Contexto da Aula 14

A Aula 14 se chama:

Da série temporal ao Edge Analytics

A aula aborda:

- dados raw;
- sinais temporais;
- aceleração X, Y, Z;
- vibração;
- janelas temporais;
- janela fixa;
- janela com sobreposição;
- problema da borda da janela;
- features;
- média;
- desvio padrão;
- RMS;
- min;
- max;
- pico a pico;
- magnitude;
- RMS da magnitude;
- energia;
- jerk;
- crest factor;
- anotação de dados;
- timestamp;
- labels;
- dataset raw;
- dataset de features;
- separação treino/teste por coleta;
- Edge Analytics;
- Analytics no Colab.

## Labels padrão

Usar sempre estas labels:

- normal
- vibracao

Evitar variações como:

- anomalia
- anomala
- vibration
- abnormal

A consistência das labels é obrigatória.

## Formatos de dados

### Dataset raw

Cada linha representa uma leitura individual do sensor.

Formato:

timestamp,ax,ay,az,label,coleta_id

Exemplo:

2026-06-12T20:10:05.010,0.10,0.02,9.78,normal,normal_01

### Dataset de features

Cada linha representa uma janela de tempo.

Formato mínimo:

window_id,timestamp_inicio,timestamp_fim,coleta_id,label,mean_ax,std_ax,rms_ax,min_ax,max_ax,p2p_ax,mean_ay,std_ay,rms_ay,min_ay,max_ay,p2p_ay,mean_az,std_az,rms_az,min_az,max_az,p2p_az,rms_mag,energy_mag,jerk_mag,crest_mag

## Features obrigatórias

Sempre que a tarefa pedir features de aceleração, calcular:

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

## Estratégias de janela

### Janela fixa

Usar:

- taxa de amostragem: 100 Hz;
- intervalo entre amostras: 10 ms;
- janela: 1 segundo;
- 100 amostras por janela;
- sem sobreposição.

### Janela com sobreposição

Usar:

- taxa de amostragem: 100 Hz;
- intervalo entre amostras: 10 ms;
- janela: 2 segundos;
- 200 amostras por janela;
- passo: 1 segundo;
- deslocamento: 100 amostras;
- sobreposição: 50%.

## Forma 1 — Edge Analytics no dispositivo

Na Forma 1, o ESP32 calcula features no próprio dispositivo.

O ESP32 deve:

1. coletar ax, ay, az;
2. montar uma janela;
3. calcular features;
4. incluir timestamp;
5. incluir coleta_id;
6. incluir label;
7. enviar uma mensagem MQTT por janela;
8. permitir armazenamento no InfluxDB.

Atenção:

- Na Forma 1, o servidor recebe features, não raw.
- O Colab deve visualizar séries temporais de features.
- Não prometer visualização do sinal raw completo se o raw não foi armazenado.

## Forma 2 — Analytics no Colab

Na Forma 2, o ESP32 envia apenas dados raw via Serial.

O ESP32 deve enviar somente:

ax,ay,az

O Python no computador deve:

1. receber ax, ay, az;
2. gerar timestamp no momento de recebimento;
3. adicionar label;
4. adicionar coleta_id;
5. salvar CSV;
6. unir arquivos normal e vibracao;
7. gerar dataset_raw.csv.

Depois, no Colab, o aluno deve:

1. carregar dataset_raw.csv;
2. visualizar ax, ay, az;
3. calcular magnitude;
4. criar janelas fixas;
5. criar janelas com sobreposição;
6. calcular as mesmas features da Forma 1;
7. treinar uma rede neural básica.

## InfluxDB

Quando a tarefa envolver InfluxDB, usar arquitetura simples:

ESP32 → MQTT Broker → Telegraf → InfluxDB

Não tratar o InfluxDB como broker MQTT.

Preferir:

- Mosquitto como broker MQTT;
- Telegraf consumindo MQTT;
- InfluxDB 2.x armazenando os dados.

## Separação treino/teste

Quando houver janelas com sobreposição, não usar separação aleatória simples.

Não usar train_test_split aleatório como solução principal.

Separar por coleta_id.

Exemplo:

- treino: normal_01, vibracao_01
- teste: normal_02, vibracao_02

Explicar que janelas sobrepostas são parecidas e podem causar vazamento de informação se forem misturadas aleatoriamente entre treino e teste.

## Rede neural básica

Quando a tarefa pedir rede neural, usar uma rede simples e didática.

Preferir Keras/TensorFlow:

- Dense(16, activation="relu")
- Dense(8, activation="relu")
- Dense(1, activation="sigmoid")

Usar:

- loss = binary_crossentropy
- optimizer = Adam
- métrica = accuracy

Avaliar com:

- acurácia;
- matriz de confusão;
- classification_report.

## Bibliotecas

Para ESP32 físico com MPU6050/MPU6500, usar preferencialmente a biblioteca FlixPeriph, mantendo compatibilidade com os exemplos anteriores.

Se for necessário usar outra biblioteca no Wokwi, justificar no README.

Para Python, priorizar:

- pandas;
- numpy;
- matplotlib;
- pyserial;
- scikit-learn;
- tensorflow/keras quando houver rede neural.

Evitar bibliotecas desnecessárias.

## Antes de implementar qualquer tarefa

Antes de criar ou modificar arquivos, faça primeiro uma análise curta contendo:

1. quais arquivos de referência foram consultados;
2. quais padrões serão reaproveitados;
3. quais arquivos novos serão criados;
4. quais limitações existem;
5. quais decisões didáticas serão adotadas.

Não implemente sem apresentar esse plano quando o pedido envolver vários arquivos.

## Limitações que devem ser explicitadas

Sempre que relevante, explicar:

- Edge Analytics reduz tráfego, mas perde flexibilidade sobre o raw;
- dados raw permitem testar novas janelas depois;
- timestamp gerado no Python é timestamp de recepção, não necessariamente o instante exato da medição;
- janelas sobrepostas exigem cuidado na separação treino/teste;
- Wokwi pode ter limitações para MQTT ou sensores;
- simular vibração com tapas na mesa é didático, mas não substitui ensaio industrial real.

## Regra de segurança do repositório

Não alterar arquivos antigos sem autorização.

Não apagar arquivos.

Não reformatar o repositório inteiro.

Criar novos arquivos dentro de aula14-edge-analytics/ ou docs/, salvo instrução explícita em contrário.