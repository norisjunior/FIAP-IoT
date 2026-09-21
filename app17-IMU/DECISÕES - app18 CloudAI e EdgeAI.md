# Decisões — app18 (CloudAI e EdgeAI)

Registro curto do que foi decidido e por quê. Uma linha por decisão, duas no máximo.
Complementa `INSTRUÇÕES PARA POWERPOINT - Aula 17 v2 IMU.md`, que trata dos slides.

## Estrutura

- `CloudAI/` e `EdgeAI/` como pastas irmãs dentro do app: a divisão do diretório já é a
  mensagem da aula — o que muda é **onde o modelo roda**.
- `CloudAI/` é o app que já existia (`api/`, `device/`, `n8n/`), sem nenhuma alteração.
- `EdgeAI/` é novo: `colab/` com o treinamento e `device/` com o firmware sem rede.

## CloudAI — o que fica

- Modelo: a **MLP** treinada no app17-7, servida por FastAPI a partir de um `.pkl`.
- Caminho: ESP32 publica a janela → n8n → API → n8n → ESP32 acende a saída.
- Telegram só em `anomalia`, no segundo ramo do fluxo.

## EdgeAI — o que entra

- Modelo: uma **Random Forest** nova, treinada em `EdgeAI/colab/treinamento_rf_edge.ipynb`.
- Mesmo dataset do app17-7, mesmo `SELECT`, mesmo corte por rodada — só o modelo muda.
- Sem rede: saem MQTT, n8n e API. A resposta nasce e morre dentro do ESP32.
- O micromlgen traduz a floresta em `if`/`else`; o scaler vira um header à parte.
- Firmware é o do CloudAI sem `WiFi.h`, `PubSubClient` e `ArduinoJson`, mais duas linhas.

## Treinamento

- `y` em **texto**, como no app17-7: assim o `.pkl` roda na API do CloudAI sem adaptação.
- Com rótulo inteiro o `.pkl` derrubaria a API, no `join(modelo.classes_)` da inicialização.
- `classes_` vem em ordem **alfabética**: índice 0 = `anomalia` … 3 = `operando`.
- Os pinos por classe não mudaram; mudaram os números do `switch` no firmware.
- Empate de votos cai no menor índice — que agora é `anomalia`, o lado seguro para errar.
- **15 árvores**, número ímpar, para tornar o empate raro.
- `max_features=None`: header de 345 linhas em vez de 1389, profundidade 3 em vez de 11.
- `StandardScaler` mantido, embora a árvore não precise: simetria com o app29/app30.

## Nomes e contratos

- A função do scaler chama-se `standardize` — não `std`, não `normalize`.
- `StandardScaler` faz padronização; `normalize` no scikit-learn é outra conta, por amostra.
- A ordem das 8 features no Colab é a ordem de `x[]` no firmware; trocar não dá erro.
- O vetor `NOMES_CLASSES[4]` sai impresso no Colab: é ele que traduz o índice em nome.
- Os dois `.hpp` são sempre do mesmo treino — scaler velho com floresta nova compila igual.

## Números para o slide

- Nuvem: flash 60,0% e RAM 14,4%. Borda: flash 23,0% e RAM 7,2%.
- A pilha Wi-Fi + TCP + MQTT custa ~474 KB; a floresta inteira custa **1,8 KB**.
- Resposta: ~1 s na nuvem (rede + n8n + API), microssegundos na borda.
- Medido com `pio run`, `esp32dev`, `espressif32@6.12.0`, com o modelo sintético do projeto.

## Fica para a aula falada, fora do notebook

- Divergência C++ × Python: o micromlgen escreve o **voto** da folha, não a **dúvida** dela.
- Folha 51/49 vira voto inteiro, e duas árvores em dúvida derrubam uma que tinha certeza.
- Por isso a árvore cresce sem `max_depth`: limitar a profundidade é o que cria folha mista.
- Com MLP isso não existe — não há votação, só arredondamento de `float64` para `float32`,
  que só vira erro em quem já estava colado no limiar.
- Quantização `int8` é o terceiro degrau: aí a divergência deixa de ser desprezível.

## Material de apoio

- `EdgeAI/CONSTRUIR-O-FIRMWARE.md`: firmware do zero em **5 etapas** (o do CloudAI tem 7).
- Checklist de paridade com 12 itens, incluindo a orientação física do sensor no gabarito.
- Headers sintéticos comitados: o projeto compila de saída, como o `.pkl` sintético da API.

## Alinhamento feito fora do app18

- app29, app30 e app19: `Scaler::std` → `Scaler::standardize` nos `.hpp`, `.ino` e READMEs.
- O Colab do app29/app30 entrou no repositório, em `colab/`, já com o nome novo.
- Nesse Colab, `min_samples_leaf` perdeu o `None`, que invalidava 4 dos 12 candidatos do grid.
