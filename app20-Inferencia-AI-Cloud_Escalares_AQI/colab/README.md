# colab — o treinamento, com dataset público

`AQI_NN.ipynb` treina a rede neural que a API serve e gera os dois arquivos que
vão para `api/`:

| Arquivo | O que guarda |
|---|---|
| `modelo_aqi_nn.keras` | a rede neural |
| `preprocess_aqi.pkl` | o `StandardScaler`, o `LabelEncoder` e os nomes das classes |

## O que muda aqui em relação aos outros apps

Nos apps do motor e da bag, **a turma gera o dataset**: alguém segura o gabarito,
aperta um botão para dizer qual é a classe, e cada janela sai rotulada pelo
dispositivo.

Aqui não. O dataset vem pronto do Kaggle — *Air Quality Data in India*, o
arquivo `city_hour.csv`, baixado pelo `kagglehub` dentro do próprio notebook.
São medições reais de estações de monitoramento indianas, com o rótulo já
atribuído.

Isso muda o que dá para ensinar com cada um:

| | dataset da turma | dataset público |
|---|---|---|
| Rotulagem | vocês decidem, e vive-se o custo disso | vem pronta, com o critério de outra pessoa |
| Tamanho | centenas de janelas | centenas de milhares de linhas |
| O que se aprende | que o rótulo é projetado | que o modelo escala com dado de verdade |

## As três classes, e por que três

O dataset traz o `AQI_Bucket` em **seis** faixas, que é a escala oficial
indiana. Para a operação de uma fábrica, seis é detalhe demais: o que muda o
comportamento de quem está lá dentro são três situações.

| Faixas do dataset | Classe | O que se faz |
|---|---|---|
| Good, Satisfactory, Moderate | `Aceitável` | nada |
| Poor, Very Poor | `Ruim` | atenção, reduzir exposição |
| Severe | `Perigoso` | evacuar |

**Estes três nomes são um contrato.** O fluxo do n8n compara a resposta do
modelo com a palavra `Perigoso` para decidir se manda o alerta crítico. Mudar
um nome aqui não dá erro em lugar nenhum — o ramo do alerta simplesmente para
de disparar. Por isso a célula do mapa termina com um `assert` que falha na hora
se alguma das três sumir.

## As versões

A primeira célula pina as versões, e elas são **as mesmas** do
`api/requirements.txt`. Não é zelo: o `.pkl` é um pickle de objetos do
scikit-learn, e o `.keras` carrega metadado da versão que o escreveu. Versões
diferentes nos dois lados dão erro no `load` — ou, pior, carregam e predizem
errado sem avisar.

`tensorflow 2.20`, `numpy 2.1` e `pandas 2.2.3` são as primeiras com wheel para
o Python 3.13 do Colab. As anteriores não instalam mais lá.

## Rodar

1. Abra o notebook no Colab e rode a primeira célula (as versões). Se ele pedir
   *Restart session*, reinicie e rode de novo.
2. As células seguintes baixam o CSV pelo `kagglehub` — na primeira vez o Colab
   pede para você conectar a conta do Kaggle.
3. Ao fim, baixe `modelo_aqi_nn.keras` e `preprocess_aqi.pkl` e cole os dois em
   `api/`, substituindo os que vieram no projeto.

Os dois **sempre juntos, do mesmo treino**: o scaler guarda a escala em que a
rede foi treinada, e misturar um com a rede de outra execução não dá erro — dá
predição errada, em silêncio.
