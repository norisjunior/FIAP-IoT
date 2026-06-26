# Sprint 3 — Coleta experimental de vibração e formação da base bruta

## Objetivo

Nesta Sprint, a equipe deve coletar dados raw de aceleração usando ESP32 + MPU6050, organizar os dados por condição experimental e gerar uma base bruta rotulada.

O foco desta Sprint é entender o sinal temporal.

Não é o momento principal de treinar modelo de Machine Learning nem de extrair features no dispositivo.

## Contexto

A vibração não é bem representada por uma única leitura.

Um valor isolado de aceleração pode significar:

- sensor parado;
- sensor inclinado;
- início de movimento;
- fim de movimento;
- ruído;
- vibração momentânea.

Por isso, a equipe deve observar a aceleração ao longo do tempo.

## Hardware esperado

- ESP32 físico ou ESP32 no Wokwi;
- MPU6050 ou MPU6500 compatível;
- cabo USB;
- computador com VSCode + PlatformIO;
- Python para coleta serial e geração de CSV.

## Condições experimentais

Usar duas classes:

### normal

Sensor parado sobre a mesa, sem impactos.

### vibracao

Sensor sobre a mesa enquanto são aplicados impactos na mesa para simular vibração/anomalia.

## Protocolo de coleta

Para cada classe:

1. preparar o ambiente;
2. iniciar a coleta;
3. aguardar alguns segundos para estabilização;
4. coletar por pelo menos 1 minuto;
5. salvar o arquivo CSV;
6. verificar visualmente o gráfico;
7. repetir em outra rodada, se possível.

## Taxa de amostragem

Usar preferencialmente:

- 100 Hz;
- 1 leitura a cada 10 ms.

Isso gera aproximadamente:

- 100 amostras por segundo;
- 6000 amostras por minuto;
- por classe: pelo menos 6000 leituras.

## Dados enviados pelo ESP32

O ESP32 deve enviar via Serial:

ax,ay,az

Exemplo:

0.10,0.02,9.78
0.12,0.01,9.79
0.30,0.05,9.65

## Timestamp

Na Sprint 3, o timestamp pode ser gerado pelo script Python no momento em que a leitura é recebida pela Serial.

Observação importante:

O timestamp do Python representa o momento de recebimento no computador, não necessariamente o instante exato da medição no ESP32.

Para fins didáticos, isso é aceitável.

## Arquivos esperados

A equipe deve gerar:

- normal.csv
- vibracao.csv
- dataset_raw.csv

## Formato dos arquivos individuais

normal.csv:

timestamp,ax,ay,az
2026-06-12T20:10:05.010,0.10,0.02,9.78
2026-06-12T20:10:05.020,0.11,0.01,9.79

vibracao.csv:

timestamp,ax,ay,az
2026-06-12T20:12:05.010,0.40,0.20,9.50
2026-06-12T20:12:05.020,1.10,0.80,8.90

## Formato do CSV final

dataset_raw.csv:

timestamp,ax,ay,az,label,coleta_id

Exemplo:

2026-06-12T20:10:05.010,0.10,0.02,9.78,normal,normal_01
2026-06-12T20:12:05.010,0.40,0.20,9.50,vibracao,vibracao_01

## Visualização obrigatória

A equipe deve gerar gráficos temporais de:

- ax;
- ay;
- az;
- magnitude.

A magnitude pode ser calculada por:

mag = sqrt(ax² + ay² + az²)

## Entregáveis

A equipe deve entregar:

1. código ESP32 para coleta raw;
2. script Python de coleta;
3. CSV normal;
4. CSV vibracao;
5. CSV final unificado;
6. gráficos da série temporal;
7. breve explicação do protocolo de coleta;
8. comentário sobre diferenças visuais entre normal e vibracao.

## Critérios de avaliação

A entrega será avaliada por:

- funcionamento da coleta;
- organização dos CSVs;
- consistência das labels;
- quantidade mínima de dados;
- clareza dos gráficos;
- explicação do protocolo experimental;
- entendimento de que vibração é sinal temporal.

## Observação didática

Nesta Sprint, o mais importante é o aluno perceber que:

Um ponto não forma movimento. Uma sequência forma movimento.

E também:

Em IoT, coletar dados é fácil. Coletar dados bem anotados é engenharia.