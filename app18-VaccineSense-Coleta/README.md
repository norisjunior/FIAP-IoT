# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 18 - Vaccine Sense: coleta e rotulagem do dataset

Uma caixa térmica de transporte de vacinas mede suas condições uma vez por
segundo e publica no broker. Os dados atravessam a plataforma até o Colab, onde
o aluno **rotula as medições** e gera o dataset.

Este app não decide nada: ele coleta. O modelo de ML entra no `app19`.

## O pipeline

```
ESP32  →  MQTT Broker (local)  →  Node-RED (local)  →  InfluxDB (cloud)  →  Colab
                                                                              ↓
                                                                    dataset rotulado
```

## Montagem

| Componente | Pino | Mede / faz |
|---|---:|---|
| DS18B20 | GPIO 23 | temperatura interna da carga |
| DHT22 | GPIO 22 | temperatura e umidade externas |
| HC-SR04 TRIG / ECHO | GPIO 19 / 18 | distância da tampa até a carga |
| Potenciômetro | GPIO 34 | criticidade da carga (0–100) |
| LDR | GPIO 35 | luz dentro da caixa |
| LED | GPIO 25 | sinalização |
| Botão | GPIO 27 | inicia e para a coleta |

O buzzer (GPIO 26) não é usado neste app.

## Configurar

No topo do `.ino`:

```cpp
const char* WIFI_SSID  = "Wokwi-GUEST";
const char* WIFI_SENHA = "";
const char* BROKER_IP  = "host.wokwi.internal";   // na sala: IP da Ethernet do notebook
const char* TOPICO     = "fiap/iot/vaccinesense";
const char* CLIENT_ID  = "ESP32Noris001Vaccine";  // único no broker e no dataset
```

O `CLIENT_ID` identifica a equipe nas duas pontas: é o nome no broker e vai no
payload como `device`.

## Botão e LED

O **botão** inicia e para a coleta, e **numera a rodada**: cada vez que você
inicia, o firmware abre uma rodada nova, incrementa o número e zera o `id` e o
contador de exposição. Enquanto está parado, nada é publicado.

```
>>> Coleta INICIADA - rodada 3
>>> Coleta PARADA
```

Anote o que foi cada rodada — é isso que você vai preencher no Colab. Se
reiniciar o ESP32, a numeração recomeça do 1.

| LED | Significa |
|---|---|
| aceso | ambiente hostil (temperatura externa acima de 30 °C) |
| piscando | carga precisa de avaliação técnica |

O LED é **conferência ao vivo durante a coleta, não rótulo** — ele não vai no
payload. Se numa rodada de ambiente hostil o LED nunca acender, o aquecedor não
foi suficiente e a rodada saiu fraca: aumente e repita.

Para tampa aberta **não existe LED**, porque o firmware não tem regra para isso
de propósito — é justamente o que o modelo vai aprender no `app19`. A
conferência é no Serial Monitor: quando você abre a tampa, `luz` sobe e
`distancia` muda no JSON que aparece a cada segundo.

O rótulo de cada rodada é definido **por você, no Colab**, a partir da sua
anotação. Nenhum `if` do firmware entra nisso.

## Payload

Um JSON por segundo em `fiap/iot/vaccinesense`. O horário quem carimba é o
Node-RED.

```json
{"device":"ESP32Noris001Vaccine","rodada":3,"id":42,
 "tempInterna":5.2,"tempExterna":24.1,"umidade":61,
 "luz":850,"criticidade":18,"distancia":12.4,"tempoForaDaFaixa":0}
```

`device` vem do `CLIENT_ID` e `rodada` vem do botão — os dois nascem no
firmware. No Node-RED, a única tag é a `rodada`, e **não há nada para editar
entre rodadas**.

## Passo a passo

**1. Subir a plataforma.** Broker MQTT e Node-RED, conforme
`IoT-platform/README.md`.

**2. Compilar e simular.** `pio run` e inicie o Wokwi, ou grave no ESP32 físico.
Serial Monitor em `115200`.

**3. Conferir se chega no broker.**

```bash
mosquitto_sub -h localhost -t "fiap/iot/vaccinesense" -v
```

**4. Importar o fluxo Node-RED.** `nodered/vaccinesense-ingestao.json`.
Configure o nó InfluxDB com a sua URL, organização, bucket e token. Isso é feito
**uma vez** — depois não se mexe mais no fluxo.

**5. Coletar.** Monte a condição, **depois** aperte o botão. Colete cinco
minutos. Aperte de novo para parar. Anote o número da rodada e o que ela foi.

**6. Abrir o Colab.** `colab/app18_coleta_e_rotulagem.ipynb` — traz os dados do
InfluxDB, você rotula cada rodada e salva o dataset.

## Protocolo de coleta

Três rodadas de cada tipo, cinco minutos cada.

| O que fazer | Rótulo no Colab |
|---|---|
| caixa fechada, gelo suficiente, sala normal | `TRANSPORTE_OK` |
| **caixa fechada**, perto de aquecedor ou ao sol | `AMBIENTE_HOSTIL` |
| tampa aberta | `CARGA_EM_PERIGO` |

## Receita das nove rodadas no Wokwi

Clique em cada peça do simulador para abrir o controle: DS18B20 e DHT22 têm
sliders de temperatura, o fotorresistor tem `lux`, o HC-SR04 tem distância em cm.

**Varie os valores devagar durante a rodada.** Se você deixar os controles
parados, as 300 medições saem quase idênticas e o modelo aprende nove *pontos*
em vez de nove *regiões*. Os números entre parênteses são a faixa para percorrer.

| # | Rótulo | Temp. interna | Temp. externa | Luz | Distância |
|---:|---|---|---|---|---|
| 1 | `TRANSPORTE_OK` | 4 (3–5) | 22 (21–24) | baixa | 12 (10–14) |
| 2 | `TRANSPORTE_OK` | 5 (4–6) | 24 (23–26) | baixa | 15 (13–17) |
| 3 | `TRANSPORTE_OK` | 6 (5–7) | 26 (25–28) | baixa | 10 (8–12) |
| 4 | `AMBIENTE_HOSTIL` | 5 (4–6) | **35** (33–37) | baixa | 12 (10–14) |
| 5 | `AMBIENTE_HOSTIL` | 5,5 (5–6,5) | **37** (35–39) | baixa | 14 (12–16) |
| 6 | `AMBIENTE_HOSTIL` | 6 (5–7) | **40** (38–42) | baixa | 12 (10–14) |
| 7 | `CARGA_EM_PERIGO` | **9** (8,5–10) | **38** (36–40) | baixa | 12 (10–14) |
| 8 | `CARGA_EM_PERIGO` | **8,5** (8,2–9,5) | 25 (24–26) | **alta ≈ 3000** | **60** (50–70) |
| 9 | `CARGA_EM_PERIGO` | **9,5** (9–10,5) | 24 (23–25) | **baixa** | **55** (45–65) |

A umidade é contexto: deixe entre 50 e 70, variando um pouco.

### Por que estas nove e não outras

Cada trio existe para desfazer uma confusão possível:

- **1–3 contra 4–6** — mesma temperatura interna boa, temperatura externa muito
  diferente. Ensina que calor lá fora, com a caixa fechada, **não** é perigo.
- **4–6 contra 7** — mesma temperatura externa alta; o que muda é a interna.
  Sem isso o modelo aprenderia "externa alta = perigo" e alarmaria em todo dia
  quente.
- **8 contra 9** — as duas são tampa aberta, mas na 9 a sala está escura e a luz
  quase não sobe. Obriga o modelo a olhar também a distância, em vez de decidir
  só pelo LDR.

A rodada 9 é a mais importante do conjunto: é o caminhão à noite.

### Antes da rodada 8: calibre o `lux`

O Wokwi controla `lux`, mas o payload traz `luz = 4095 − analogRead`. A
correspondência depende do circuito, então descubra na tela:

1. Clique no fotorresistor e ponha `lux` no mínimo. Olhe o valor de `luz` no
   Serial Monitor.
2. Ponha `lux` no máximo e olhe de novo.
3. Anote os dois extremos e escolha o `lux` que deixa `luz` perto de 3000.

O mesmo valor de "luz baixa" das outras rodadas sai do primeiro extremo.

### Confira pelo LED enquanto coleta

| Rodada | LED esperado |
|---:|---|
| 1–3 | apagado |
| 4–7 | **aceso** (temp. externa passou de 30) |
| 8–9 | apagado |
| 7, 8, 9 | **piscando** depois de ~2 min (temperatura interna acumulou 120 s fora da faixa) |

Se o LED não fizer isso, a rodada saiu diferente do planejado — vale repetir
antes de descobrir no Colab.

### Folha de anotação

| Rodada | Condição |
|---:|---|
| 1, 2, 3 | caixa fechada, sala normal |
| 4, 5, 6 | caixa fechada, ambiente quente |
| 7 | ambiente quente **e** carga esquentando |
| 8 | tampa aberta, sala clara |
| 9 | tampa aberta, sala escura |

Regras:

- monte a condição **antes** de apertar o botão, e mantenha estável até parar;
- as rodadas `hostil` são obrigatórias: sem elas o modelo confunde dia quente
  com tampa aberta, porque as duas coisas esquentam a carga;
- rodadas separadas e numeradas — é isso que permite dividir treino e teste sem
  vazamento.

## Sobre os limiares

O firmware tem um único limiar: 30 °C para ambiente hostil.

Os valores de **luz** e **distância** que indicam tampa aberta não estão no
firmware, porque mudam com o tamanho da caixa, a posição dos sensores e a
iluminação da sala. Por isso a rotulagem no Colab é feita **pela rodada**, não
por limiar.

O limite de exposição térmica (120 segundos fora da faixa) é didático. Na
prática ele vem da estabilidade documentada de cada imunobiológico.

## Estrutura

```
src/
├── app18-vaccinesense-publisher.ino   Wi-Fi, MQTT, acumulador, LED
├── VC_TemperaturaCarga.hpp            DS18B20
├── VC_AmbienteExterno.hpp             DHT22
├── VC_Luz.hpp                         LDR
├── VC_Criticidade.hpp                 potenciômetro
└── VC_Carga.hpp                       HC-SR04
nodered/
└── vaccinesense-ingestao.json         MQTT → InfluxDB
colab/
└── app18_coleta_e_rotulagem.ipynb     InfluxDB → dataset rotulado
```

## Próximo passo

Aplicação 19 — treinar o modelo com este dataset e servir a classificação por
API a partir do `.pkl`.
