# app18 / edge — a mesma decisão, dentro do ESP32

A pasta `api/` deste app coloca o modelo na nuvem: a janela sai por MQTT,
atravessa o n8n e a FastAPI, e a classe volta por outro tópico. Aqui o modelo
desce para o dispositivo. A janela é a mesma, as 8 features são as mesmas, as 4
saídas são as mesmas — **muda só quem decide, e a resposta para de sair da
placa.**

```text
nuvem:  ESP32 → MQTT → n8n → API (.pkl) → n8n → MQTT → ESP32
borda:  ESP32 → Random Forest na flash → ESP32
```

Sem Wi-Fi, sem broker, sem n8n, sem servidor. Este app não usa nada da
IoT-platform.

## O que muda

| | com API (`device/`) | na borda (`edge/device/`) |
|---|---|---|
| Modelo | rede neural (MLP), num `.pkl` | Random Forest, em `if`/`else` |
| Onde roda | FastAPI, no computador | na flash do ESP32 |
| Como chega | JSON por MQTT, classe em texto | vetor de 8 floats, classe em número |
| Bibliotecas | FastIMU + PubSubClient + ArduinoJson | FastIMU |
| Tempo de resposta | ~1 s (rede + n8n + API) | poucos microssegundos |
| Se a rede cair | tudo apagado | não muda nada |
| Flash / RAM | 60,0% / 14,4% | 23,0% / 7,2% |

Os dois últimos números são a aula inteira em uma linha: o firmware que **carrega
o modelo** é menos da metade do tamanho do firmware que só sabia **perguntar** a
ele. A pilha Wi-Fi + TCP + MQTT custava cerca de 474 KB; a floresta de 15
árvores custa **1,8 KB**. (Medido com `pio run` nos dois projetos, `esp32dev`,
`espressif32@6.12.0`, com o modelo sintético que acompanha o projeto.)

## 1) Treinar a floresta

[`colab/treinamento_rf_edge.ipynb`](colab/treinamento_rf_edge.ipynb) lê **o mesmo
dataset** do `app17-7` no InfluxDB, com o mesmo `SELECT` e o mesmo corte por
rodada, e gera três arquivos — o mesmo modelo, em três formatos:

| Arquivo | Serve para |
|---|---|
| `ModeloMotorRF.hpp` | as 15 árvores, em `if`/`else`, para o ESP32 |
| `ModeloMotorScaler.hpp` | a média e o desvio de cada feature, para o ESP32 |
| `modelo_motor_rf.pkl` | o Pipeline inteiro, para Python |

O notebook também compara a floresta com a rede neural no mesmo corte, antes de
exportar: da nuvem para a borda mudam o modelo *e* o lugar onde ele roda, e sem
essa comparação não dá para saber a qual dos dois atribuir uma diferença de
resultado.

O `.pkl` fecha esse raciocínio na prática. Ele é treinado com os rótulos em
**texto**, igual ao do `app17-7`, então roda na API do app18 sem adaptação
nenhuma — basta colocá-lo em `api/` e subir apontando para ele:

```bash
MODELO_ARQUIVO=modelo_motor_rf.pkl uvicorn service_app:app --host 0.0.0.0 --port 8000
```

Aí a **mesma** floresta está rodando na nuvem e na borda, e a comparação entre as
duas passa a ser só de *onde*, com o modelo controlado.

## 2) Colar os dois headers

Descompacte o ZIP baixado e cole os dois arquivos em `device/src/`, por cima dos
sintéticos que vieram no projeto:

```text
edge/device/src/
├── app18-edge-inferencia-rf.cpp
├── ModeloMotorRF.hpp        ← gerado no Colab
└── ModeloMotorScaler.hpp    ← gerado no Colab
```

> Os dois **sempre do mesmo treino**. O scaler guarda a escala em que os limiares
> da floresta foram escritos; misturar o scaler de uma execução com a floresta de
> outra não dá erro de compilação — dá predição errada, em silêncio.

O notebook imprime, no fim, a linha do vetor `NOMES_CLASSES[4]`. Cole-a também:
é ela que transforma o número que sai do `predict()` de volta em nome de classe.

## 3) Compilar

```bash
cd device
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
  mean_ax=0.418  mean_ay=-0.006  mean_az=0.909
  std_ax=0.031   std_ay=0.028   std_az=0.035
  std_mag=0.040  p2p_mag=0.187
  MODELO:  1 -> inclinado_frente
  Inferencia: 6 us
----------------------
```

Para montar o firmware do zero, em cinco etapas que compilam:
[CONSTRUIR-O-FIRMWARE.md](CONSTRUIR-O-FIRMWARE.md).

No Wokwi não há como inclinar o MPU6050: só `operando` e `anomalia` têm
equivalente no simulador. Troque o `#define MPU_TYPE` para `MPU6050` e comente a
linha `mpu.calibrateAccelGyro(&calib);`, que trava no simulador.

## Estrutura

```text
colab/     treinamento_rf_edge.ipynb
device/    firmware sem rede + os dois .hpp do modelo
```

## Se der errado

**Sempre a mesma classe.** Os dois `.hpp` são do mesmo treino? Um scaler antigo
com uma floresta nova joga todas as janelas para o mesmo lado da primeira
comparação.

**As inclinações saem trocadas.** Confira o `NOMES_CLASSES[4]` contra a linha que
o Colab imprimiu e, depois, a orientação física do sensor: girar o sensor 90° no
gabarito troca `mean_ax` por `mean_ay`.

**`'Eloquent' was not declared`.** O `ModeloMotorRF.hpp` não é o arquivo gerado
pelo micromlgen, ou foi colado pela metade — ele precisa conter
`namespace Eloquent`.

**O header ficou gigante.** O notebook usa `max_features=None` justamente para
manter as árvores rasas. Se o seu ficou com milhares de linhas, alguma coisa mudou
na configuração da floresta.

**Predição diferente da que o Colab mostrou.** Rode a seção 9 do notebook: ela
compila o header e compara com o scikit-learn. Se ela passa e o ESP32 diverge, o
problema é paridade de features — vá para o checklist do guia do firmware.
