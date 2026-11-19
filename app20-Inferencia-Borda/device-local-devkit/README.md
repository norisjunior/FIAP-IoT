# Só decidir: a mesma decisão, dentro do ESP32, sem rede

O app18 coloca o modelo na nuvem: a janela sai por MQTT,
atravessa o n8n e a FastAPI, e a classe volta por outro tópico. Aqui o modelo
desce para o dispositivo. A janela é a mesma, as 8 features são as mesmas, as 4
saídas são as mesmas — **muda só quem decide, e a resposta para de sair da
placa.**

```text
nuvem:  ESP32 → MQTT → n8n → API (.pkl) → n8n → MQTT → ESP32
borda:  ESP32 → Random Forest na flash → ESP32
```

Sem Wi-Fi, sem broker, sem n8n, sem servidor: esta pasta não usa nada da
IoT-platform. O firmware da aula, que decide **e** publica, é o de
[`../device-MQTT-devkit/`](../README.md#2-o-firmware).

## O que muda

| | app18, `device-devkitv1/` | `device-local-devkit/` |
|---|---|---|
| Modelo | rede neural (MLP), num `.pkl` | Random Forest, em `if`/`else` |
| Onde roda | FastAPI, no computador | na flash do ESP32 |
| Como chega | JSON por MQTT, classe em texto | vetor de 8 floats, classe em número |
| Bibliotecas | FastIMU + PubSubClient + ArduinoJson | FastIMU |
| Tempo de resposta, depois da janela | 48 ms (rede + n8n + API), sem contar o Wi-Fi | microssegundos |
| Se a rede cair | tudo apagado | não muda nada |
| Flash / RAM | 60,0% / 14,4% | 23,0% / 7,2% |

Os dois últimos números são a aula inteira em uma linha: o firmware que **carrega
o modelo** é menos da metade do tamanho do firmware que só sabia **perguntar** a
ele. A pilha Wi-Fi + TCP + MQTT custava cerca de 474 KB; a floresta de 15
árvores custa **1,9 KB**. (Medido com `pio run` nos dois projetos, `esp32dev`,
`espressif32@6.12.0`, com a floresta do dataset de referência que acompanha o
projeto. Os 48 ms da nuvem: broker, n8n e API na mesma máquina, 60 janelas,
mediana; de 34 a 79 ms.)

## 1) Treinar a floresta

O mesmo notebook da aula, [`../colab/treinamento_rf_edge.ipynb`](../colab/treinamento_rf_edge.ipynb),
explicado no [README da pasta](../README.md#1-treinar-a-floresta). Os mesmos dois `.hpp`
servem a este firmware e ao de `../device-MQTT-devkit/`.

## 2) Colar os dois headers

Descompacte o ZIP baixado e cole os dois arquivos em `src/`, por cima dos
que vieram no projeto (os do dataset de referência):

```text
device-local-devkit/src/
├── inferencia-na-borda.cpp
├── ModeloMotorRF.hpp        ← gerado no Colab
└── ModeloMotorScaler.hpp    ← gerado no Colab
```

> Os dois **sempre do mesmo treino**. O scaler guarda a escala em que os limiares
> da floresta foram escritos; misturar o scaler de uma execução com a floresta de
> outra não dá erro de compilação — dá predição errada, em silêncio.

A seção 9 do notebook imprime a linha do vetor `NOMES_CLASSES[4]`. Cole-a também:
é ela que transforma o número que sai do `predict()` de volta em nome de classe.

## 3) Compilar

```bash
cd device-local-devkit
pio run
```

Não há nada para configurar: nem SSID, nem senha, nem IP de broker.

| GPIO | Componente | Classe | Índice |
|---|---|---|---|
| 19 | buzzer | `anomalia` | 0 |
| 21 | LED amarelo | `inclinado_frente` | 1 |
| 18 | LED vermelho | `inclinado_tras` | 2 |
| 4 | LED azul | `operando` | 3 |

Uma saída por classe, e ela **fica** ligada até a próxima janela fechar, um
segundo depois. Depois do primeiro segundo há sempre uma saída acesa — na versão
com API, "tudo apagado" queria dizer "a nuvem não respondeu"; aqui isso não
existe mais.

No Serial, uma janela por segundo:

```text
--- Janela fechada ---
  mean_ax=0.042  mean_ay=0.714  mean_az=0.671
  std_ax=0.008   std_ay=0.011   std_az=0.009
  std_mag=0.008  p2p_mag=0.076
  PREDIÇÃO:  1 -> inclinado_frente
  Inferencia: N us
----------------------
```

Uma janela real do dataset de referência, com o motor inclinado uns 45° para a
frente. O `N` da última linha é o tempo do scaler mais a floresta, em
microssegundos: anote o da sua placa.

Para montar o firmware do zero, em cinco etapas que compilam:
[CONSTRUIR-O-FIRMWARE.md](CONSTRUIR-O-FIRMWARE.md).

Na ESP32-C3 do professor, com motor de vibração:
[`../device-local-esp32c3/`](../device-local-esp32c3/README.md).

No Wokwi a **inclinação** sai: o MPU6050 tem controle de aceleração em X, Y e Z.
A `anomalia` não, porque ela é vibração e o controle parado deixa as 100
amostras da janela idênticas. Troque o `#define MPU_TYPE` para `MPU6050` e
comente a linha `mpu.calibrateAccelGyro(&calib);` — sem isso o ESP32 aborta no
simulador, com "Guru Meditation: IntegerDivideByZero".

## Estrutura

```text
src/   inferencia-na-borda.cpp · ModeloMotorRF.hpp · ModeloMotorScaler.hpp
```

## Se der errado

**Sempre a mesma classe.** Os dois `.hpp` são do mesmo treino? Um scaler antigo
com uma floresta nova joga todas as janelas para o mesmo lado da primeira
comparação.

**As inclinações saem trocadas.** Confira o `NOMES_CLASSES[4]` contra a linha que
o Colab imprimiu e, depois, a orientação física do sensor: girar o sensor 90° no
gabarito troca `mean_ax` por `mean_ay`.

**`ModeloMotorRF.hpp: No such file or directory`.** O arquivo não está direto em
`src/`. Ao descompactar, o Windows cria uma pasta com o nome do `.zip`: copie os
dois `.hpp`, não a pasta.

**`'Eloquent' does not name a type`.** O `ModeloMotorRF.hpp` está lá, mas não é o
que o micromlgen gerou, ou foi colado pela metade: ele precisa conter
`namespace Eloquent`.

**O header ficou gigante.** O notebook usa `max_features=None` justamente para
manter as árvores rasas. Se o seu ficou com milhares de linhas, alguma coisa mudou
na configuração da floresta.

**Predição diferente da que o Colab mostrou.** Confira primeiro se os dois
`.hpp` são do mesmo treino. Se forem, o problema é paridade de features — vá
para o checklist do guia do firmware.
