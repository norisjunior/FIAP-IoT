**Sprint 3 - ****Coleta experimental de vibração e formação da base bruta**

Enunciado

Nesta Sprint, a equipe deverá evoluir a solução IoT para realizar a coleta experimental de vibração do ativo monitorado, utilizando um acelerômetro MPU6050 conectado ao ESP32. O objetivo é capturar dados brutos de aceleração em frequência adequada, organizar as medições por condição de funcionamento e formar uma base temporal rotulada para análise futura. 

1. Coleta de vibração com acelerômetro (usando Wokwi vale 25 pontos, usando ESP32 físico vale 35 pontos)

- Configurar o ESP32 com sensor MPU6050, para coletar aceleração nos eixos X, Y e Z (apenas aceleração, não giroscópio).

- Realizar a coleta com frequência de 100 Hz.

- Exibir as medições no Monitor Serial em formato estruturado.

- Cada linha coletada pelo ESP32 deve conter: X, Y, Z. Exemplo:

																ax,ay,az

																0.12,-0.04,9.81

																0.18,-0.02,9.79 

2. Captura dos dados com registro temporal (20 pontos)

- Criar um script em Python para ler as medições enviadas pelo ESP32 via porta Serial.

- Registrar um timestamp para cada amostra recebida.

- Salvar os dados em arquivo CSV ou formato equivalente.

- O arquivo gerado deve conter, no mínimo: timestamp, aceleração X, aceleração Y, aceleração Z e classe da condição coletada. Exemplo: 

timestamp,ax,ay,az,label

2026-06-11 20:30:01.123,0.12,-0.04,9.81,normal

2026-06-11 20:30:01.133,0.18,-0.02,9.79,normal

3. Formação da base experimental por condição do ativo (25 pontos)

- Coletar pelo menos 1 minuto de dados para cada condição:

- condição normal: motor parado ligado (exemplo: MPU6050 em repouso sobre a mesa);

- condição anômala: motor vibrando (exemplo: MPU6050 sendo movimentado/chacoalhado).

- Separar corretamente as amostras de cada classe.

- Apresentar a quantidade total de amostras coletadas por classe.

- Organizar os dados para que possam ser usados posteriormente na extração de features e no treinamento de modelo.

- *Com coleta aproximada de 100 Hz durante 1 minuto, espera-se cerca de 6000 amostras por classe, totalizando aproximadamente 12000 amostras brutas para duas classes.*

4. Visualização exploratória dos dados brutos (10 pontos)

- Construir pelo menos uma visualização simples dos dados coletados.

- A visualização deve permitir comparar o comportamento das classes normal e anômala.

- Podem ser usados gráficos temporais dos eixos X, Y e Z ou da magnitude bruta da aceleração.

- A intenção é que a visualização permita perceber diferença visual entre a condição normal e a condição anômala.

5. Conteúdo da entrega:

- Vídeo curto de até 5 minutos demonstrando a coleta, o Monitor Serial, o script Python, os arquivos CSV e a visualização inicial dos dados 

- Código-fonte do projeto do ESP32

- Script Python de coleta Serial

- Arquivo CSV com os dados brutos rotulados, podendo ser um único arquivo com coluna de classe ou arquivos separados por condição.

- Gráfico comparando visualmente as condições normal e anômala.

**Sprint 4 - ****Base analítica, modelo e interpretação dos resultados**

Enunciado

Nesta Sprint, a equipe deverá utilizar as medições brutas rotuladas na Sprint 3 para gerar uma base analítica com features extraídas a partir dos sinais de vibração. A entrega deve demonstrar a transformação dos dados brutos em atributos numéricos, o treinamento de um modelo de Machine Learning e a interpretação dos resultados obtidos.

1. Preparação da base analítica (25 pontos)

- Utilizar obrigatoriamente as medições rotuladas geradas na Sprint 3.

- Gerar as features a partir das medições brutas de aceleração dos eixos X, Y e Z.

- As features devem ser extraídas por janelas de dados, e não a partir de uma única medição isolada.

- Garantir que o dataset final esteja balanceado.

- O dataset deverá conter, no mínimo:

- 30 registros rotulados da classe normal;

- 30 registros rotulados da classe anômala.

- Informar a quantidade de registros por classe e quais features foram utilizadas.

2. Treinamento e avaliação do modelo (20 pontos)

- Treinar pelo menos um modelo de Machine Learning para classificar as condições do ativo.

- A saída do modelo deverá indicar a classe da condição analisada:

- normal;

- anômala.

- Avaliar o modelo usando separação entre treino e teste.

- Apresentar métricas e matriz de confusão

3. Feature importance e interpretação (25 pontos)

- Apresentar quais features foram mais relevantes para a classificação.

- Relacionar as features mais importantes ao comportamento físico observado no ativo.

- Explicar se o modelo conseguiu diferenciar adequadamente a condição normal da condição anômala.

- Comentar possíveis limitações da coleta ou do experimento.

4. Conclusão da solução

- Apresentar uma conclusão sobre a viabilidade da solução proposta.

- Explicar como a abordagem poderia apoiar uma aplicação de monitoramento ou manutenção preditiva.

- Indicar possíveis melhorias futuras para o projeto.

5. Conteúdo da entrega:

- Vídeo curto de até 5 minutos com o treinamento realizado no dataset gerado pelo grupo

- Comparativo entre o Dataset bruto rotulado gerado na Sprint 3 e o Dataset final com as features extraídas

- Métricas do modelo

- Gráfico de feature importance

- Código utilizado