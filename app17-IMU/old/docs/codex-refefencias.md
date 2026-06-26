# Referências de código para o Codex

Este documento lista os códigos anteriores que devem ser usados como referência de estilo, simplicidade e organização.

O objetivo é fazer com que a Aula 14 pareça uma evolução natural dos códigos já usados na disciplina.

## Regra principal

Use os códigos antigos apenas como referência.

Não alterar, mover, apagar ou refatorar códigos antigos.

## Referências obrigatórias

### 1. app17-IMU/app17-5-JanelaFeatures/

Usar como referência principal para:

- leitura do acelerômetro;
- uso da biblioteca FlixPeriph;
- uso de MPU6050/MPU6500;
- uso de arrays para janela;
- coleta de 100 amostras;
- cálculo de mean;
- cálculo de std;
- cálculo de RMS;
- cálculo de RMS da magnitude;
- organização simples do código;
- comentários didáticos.

A nova Aula 14 deve evoluir esse exemplo para:

- saída em CSV ou JSON;
- timestamp;
- label;
- coleta_id;
- envio MQTT;
- armazenamento em InfluxDB;
- janela fixa;
- janela com sobreposição;
- features adicionais.

### 2. app17-IMU/app17-2-Coleta/

Usar como referência para:

- coleta raw;
- leitura contínua de aceleração;
- saída Serial;
- simplicidade do código;
- estrutura para visualização posterior.

A nova Forma 2 deve ser uma evolução desse padrão:

ESP32 envia raw → Python adiciona timestamp e label → CSV.

### 3. app17-IMU/app17-0-Plot/

Usar como referência para:

- visualização temporal;
- ideia de gráfico de aceleração;
- formato didático de observação do sinal.

Se houver scripts Python nessa pasta, usar como referência de estilo.

## Referências de MQTT

Pesquisar no repositório exemplos anteriores envolvendo:

- Wi-Fi;
- MQTT;
- PubSubClient;
- publicação em tópico;
- reconexão simples;
- payload JSON ou CSV.

Usar o estilo mais simples encontrado.

Se não houver exemplo anterior suficiente, criar código novo, mas manter:

- Wi-Fi simples;
- MQTT simples;
- reconexão básica;
- payload bem documentado;
- variáveis de configuração no início do arquivo.

## Referências de PlatformIO

Manter padrão didático:

- platformio.ini explícito;
- dependências em lib_deps;
- monitor_speed = 115200;
- src/main.cpp simples;
- README.md com passos de execução.

## Referências de Python

Pesquisar no repositório exemplos anteriores de:

- pandas;
- matplotlib;
- serial;
- CSV;
- Colab;
- scikit-learn;
- TensorFlow/Keras.

Manter estilo simples:

- código em células;
- comentários curtos;
- nada de pipelines complexos;
- nada de classes desnecessárias.

## O que reaproveitar

Reaproveitar:

- estilo didático;
- organização de código;
- clareza dos nomes;
- estrutura de setup/loop;
- padrão de leitura do sensor;
- padrão de cálculo de features;
- padrão de README.

## O que não reaproveitar cegamente

Não copiar sem adaptação:

- pinos se a nova placa exigir outros;
- biblioteca incompatível com Wokwi;
- prints feitos apenas para depuração;
- formatos de saída difíceis de processar;
- valores de configuração sem explicar.

## Padrão desejado para novos códigos

Os novos códigos devem ser compreensíveis para alunos.

Preferir:

- constantes no início do arquivo;
- funções pequenas para cada feature;
- arrays simples;
- saída padronizada;
- README explicando o objetivo;
- exemplos de saída;
- instruções de execução.

## Resultado esperado

A Aula 14 deve gerar uma resposta-modelo com duas formas:

### Forma 1 — Edge Analytics no dispositivo

ESP32 calcula features e envia por MQTT.

### Forma 2 — Analytics no Colab

ESP32 envia raw via Serial; Python/Colab calcula features depois.

As duas formas devem ser apresentadas como complementares, não como concorrentes.