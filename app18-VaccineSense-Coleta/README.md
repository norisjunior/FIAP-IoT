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

**5. Coletar.** Monte a condição, **depois** aperte o botão. Colete cerca de
**40 medições** por rodada, variando os sensores dentro da faixa indicada.
Aperte de novo para parar. Anote o número da rodada e o que ela foi.

Conte pelas **medições do Serial Monitor**, não pelo relógio: o Wokwi simula
mais devagar que o tempo real, então 40 segundos de simulação levam mais que
40 segundos de aula.

**6. Abrir o Colab.** `colab/app18_coleta_e_rotulagem.ipynb` — traz os dados do
InfluxDB, você rotula cada rodada e salva o dataset.

## Protocolo de coleta

São **24 rodadas**, cada uma com uma condição física estável. Doze rodadas
produzem a classe `0` do alvo binário (`TRANSPORTE_OK` e `AMBIENTE_HOSTIL`) e
doze produzem a classe `1` (`CARGA_EM_PERIGO`).

| Condição física | Rótulo no Colab |
|---|---|
| caixa fechada, ambiente normal e carga dentro da faixa | `TRANSPORTE_OK` |
| caixa fechada, ambiente acima de 30 °C e carga ainda preservada | `AMBIENTE_HOSTIL` |
| tampa aberta, ou carga fora da faixa aceita para sua criticidade | `CARGA_EM_PERIGO` |

### Por que 24 e não 12

**Cada condição aparece em duas rodadas.** Não é redundância: é o que permite
dividir treino e teste por rodada.

Se uma condição existir em uma rodada só e essa rodada cair no teste, o treino
nunca viu aquela região do espaço — e o modelo erra 100% ali, sem ter como
aprender. Com o par, uma fica no treino e ensina a outra.

Isso é uma regra geral de coleta em IoT: **o número de rodadas é ditado pelo
split, não pela quantidade de dados**.

## A faixa aceita depende da criticidade

O potenciômetro define o tipo da carga, e o tipo da carga define o que é seguro:

| Criticidade | Tipo da carga | Faixa aceita |
|---|---|---|
| 0 – 50 | padrão | 2 a 8 °C |
| 51 – 100 | crítica | 4 a 6 °C |

A mesma temperatura de 7 °C é **normal** numa carga padrão e é **perigo** numa
carga crítica. É essa interação que o modelo precisa aprender, e é por isso que
a criticidade entra como feature.

**A criticidade não muda durante uma rodada** — o potenciômetro fica parado.
O valor lido oscila alguns pontos porque o `analogRead` tem ruído, e isso é
esperado.

## Receita das 24 rodadas no Wokwi

`Luz baixa`, `média` e `alta` são valores do campo `luz` no Serial Monitor, não
o número de `lux` do controle do Wokwi.

O HC-SR04 fica na tampa e mede até o topo da carga:

- **mais vacinas**, próximas da tampa → distância **menor**;
- **menos vacinas**, no fundo da caixa → distância **maior**;
- **tampa aberta** → distância maior **e** luz alta.

### Não-perigo — tampa sempre fechada

| # | Rótulo | Situação | Temp. int. (°C) | Temp. ext. (°C) | Criticidade | Luz | Dist. (cm) |
|---:|---|---|---:|---:|---:|---|---:|
| 1 | `TRANSPORTE_OK` | padrão, varrendo a faixa segura | 2,4–7,6 | 21–27 | 12–46 | baixa | 10–18 |
| 2 | `AMBIENTE_HOSTIL` | padrão preservada no calor | 2,6–7,4 | **34–39** | 14–44 | baixa | 12–20 |
| 3 | `TRANSPORTE_OK` | padrão perto do limite superior | **6,0–7,7** | 23–29 | 10–45 | baixa | 10–18 |
| 4 | `TRANSPORTE_OK` | padrão perto do limite superior | **6,2–7,6** | 22–28 | 16–48 | baixa | 11–19 |
| 5 | `TRANSPORTE_OK` | padrão perto do limite inferior | **2,4–3,8** | 20–28 | 12–44 | baixa | 10–18 |
| 6 | `TRANSPORTE_OK` | padrão perto do limite inferior | **2,3–4,0** | 21–27 | 18–46 | baixa | 12–20 |
| 7 | `TRANSPORTE_OK` | crítica na faixa estreita | 4,3–5,7 | 20–28 | **56–92** | baixa | 10–18 |
| 8 | `TRANSPORTE_OK` | crítica na faixa estreita | 4,4–5,8 | 21–29 | **62–98** | baixa | 12–20 |
| 9 | `AMBIENTE_HOSTIL` | padrão preservada no calor | 2,6–7,4 | **33–38** | 12–46 | baixa | 10–18 |
| 10 | `AMBIENTE_HOSTIL` | crítica preservada no calor | 4,3–5,7 | **35–41** | **58–94** | baixa | 10–18 |
| 11 | `TRANSPORTE_OK` | pouca carga, sala com luz variando | 3,0–7,4 | 21–29 | 14–45 | **30–850** | **40–70** |
| 12 | `TRANSPORTE_OK` | pouca carga, sala com luz variando | 3,2–7,2 | 23–30 | 16–47 | **30–850** | **30–65** |

### Perigo

| # | Rótulo | Situação | Temp. int. (°C) | Temp. ext. (°C) | Criticidade | Luz | Dist. (cm) |
|---:|---|---|---:|---:|---:|---|---:|
| 13 | `CARGA_EM_PERIGO` | padrão superaquecida | **8,5–11,0** | 24–40 | 12–46 | baixa | 10–18 |
| 14 | `CARGA_EM_PERIGO` | padrão superaquecida | **8,6–10,5** | 22–38 | 16–44 | baixa | 12–20 |
| 15 | `CARGA_EM_PERIGO` | padrão congelando | **0,2–1,6** | 20–28 | 14–45 | baixa | 10–18 |
| 16 | `CARGA_EM_PERIGO` | padrão congelando | **0,4–1,7** | 21–27 | 18–47 | baixa | 12–20 |
| 17 | `CARGA_EM_PERIGO` | crítica acima de 6 °C | **6,5–8,4** | 20–29 | **56–93** | baixa | 10–18 |
| 18 | `CARGA_EM_PERIGO` | crítica acima de 6 °C | **6,6–8,2** | 21–28 | **60–97** | baixa | 12–20 |
| 19 | `CARGA_EM_PERIGO` | crítica abaixo de 4 °C | **1,8–3,6** | 20–28 | **54–90** | baixa | 10–18 |
| 20 | `CARGA_EM_PERIGO` | crítica abaixo de 4 °C | **2,0–3,5** | 21–27 | **64–99** | baixa | 12–20 |
| 21 | `CARGA_EM_PERIGO` | tampa aberta, carga ainda fria | 3,4–6,6 | 22–29 | 12–46 | **alta 2700–3450** | **22–70** |
| 22 | `CARGA_EM_PERIGO` | tampa aberta em ambiente hostil | 3,8–6,4 | **34–40** | 15–44 | **alta 2500–3400** | **20–65** |
| 23 | `CARGA_EM_PERIGO` | tampa apenas entreaberta | 3,6–6,4 | 20–27 | 13–45 | **média-alta 1150–2100** | **18–32** |
| 24 | `CARGA_EM_PERIGO` | tampa apenas entreaberta | 3,8–6,6 | 21–28 | 17–47 | **média-alta 1200–2200** | **20–35** |

### A umidade acompanha a temperatura externa

Não escolha a umidade ao acaso: ar mais quente carrega umidade relativa mais
baixa. Ajuste o slider do DHT22 seguindo a temperatura externa da rodada.

| Temp. externa | Umidade externa |
|---:|---:|
| 20–24 °C | 72–82 % |
| 25–29 °C | 64–74 % |
| 30–34 °C | 56–66 % |
| 35–42 °C | 48–60 % |

Como as rodadas quentes aparecem dos dois lados do alvo (rodadas 2, 9 e 10 são
não-perigo; 13, 14 e 22 são perigo), a umidade dá contexto sem entregar o rótulo.

### Por que estas 24

Cada atalho possível tem um contracaso no dataset:

| Atalho errado | Contracasos que o desfazem |
|---|---|
| temperatura interna alta é sempre perigo | 3 e 4 são seguras a 7,6 °C; 21–24 são perigo com a carga ainda fria |
| temperatura externa alta é sempre perigo | 2, 9 e 10 preservam a carga no calor; 13, 14 e 22 são perigo no mesmo calor |
| luz alta é sempre tampa aberta | 11 e 12 têm luz até 850 com a tampa fechada |
| distância alta é sempre tampa aberta | 11 e 12 são fechadas com pouca carga, na mesma faixa de distância de 21 e 22 |
| criticidade alta é sempre perigo | 7, 8 e 10 preservam carga crítica; 17–20 mostram quando ela participa do risco |
| a umidade identifica a classe | as faixas de umidade aparecem dos dois lados do alvo |

As rodadas **21 a 24** são essenciais: a tampa está aberta e a temperatura ainda
não subiu. Sem elas, uma regra de temperatura resolveria o dataset e não haveria
motivo para treinar um modelo.

As rodadas **3, 4, 17 e 18** são as mais difíceis de propósito: 7,6 °C é seguro
numa carga padrão e 6,6 °C é perigo numa carga crítica. É aí que o modelo erra —
e é aí que se aprende que o limite físico é uma região, não uma linha.

### Calibrar a luz antes de coletar

O Wokwi controla `lux`, mas o payload traz `luz = 4095 − analogRead`. A
correspondência depende do circuito:

1. Ponha `lux` no mínimo e anote o campo `luz` do Serial Monitor.
2. Ponha `lux` no máximo e anote novamente.
3. Escolha um ajuste para `luz` baixa, um para `luz` média entre 200 e 800 e um
   para `luz` alta perto de 3000.

Use os valores observados no payload; não suponha que o número de `lux` é igual
ao campo `luz`.

### Confira pelo LED enquanto coleta

| Rodadas | LED esperado |
|---|---|
| 1, 3–8, 11, 12, 21, 23, 24 | apagado |
| 2, 9, 10, 22 | **aceso** (temperatura externa acima de 30 °C) |
| 13–20 | apagado no início e **piscando** depois de ~2 min |

O pisca começa após mais de 120 segundos com a temperatura interna fora da faixa
aceita para a criticidade. O LED é conferência ao vivo: não é rótulo nem feature.

Se numa rodada de ambiente hostil o LED nunca acender, o aquecedor não passou de
30 °C e a rodada saiu fraca. Repita.

### Folha de anotação

Leve esta folha para a bancada. O número vem do Serial Monitor quando você
aperta o botão.

| Rodada | Condição anotada |
|---:|---|
| 1 | fechada, padrão, varrendo a faixa segura |
| 2 | fechada, padrão preservada no calor |
| 3 | fechada, padrão perto de 8 °C |
| 4 | fechada, padrão perto de 8 °C |
| 5 | fechada, padrão perto de 2 °C |
| 6 | fechada, padrão perto de 2 °C |
| 7 | fechada, crítica entre 4 e 6 °C |
| 8 | fechada, crítica entre 4 e 6 °C |
| 9 | fechada, padrão preservada no calor |
| 10 | fechada, crítica preservada no calor |
| 11 | fechada, pouca carga, luz da sala variando |
| 12 | fechada, pouca carga, luz da sala variando |
| 13 | fechada, padrão superaquecida |
| 14 | fechada, padrão superaquecida |
| 15 | fechada, padrão congelando |
| 16 | fechada, padrão congelando |
| 17 | fechada, crítica acima de 6 °C |
| 18 | fechada, crítica acima de 6 °C |
| 19 | fechada, crítica abaixo de 4 °C |
| 20 | fechada, crítica abaixo de 4 °C |
| 21 | **aberta**, carga ainda fria |
| 22 | **aberta** em ambiente hostil |
| 23 | **entreaberta**, carga ainda fria |
| 24 | **entreaberta**, carga ainda fria |

Regras:

- configure a situação **antes** de apertar o botão;
- mantenha tampa, tipo da carga e rótulo estáveis durante toda a rodada, mas
  varie as medições dentro das faixas;
- não deixe sliders parados nem repita a mesma sequência em todas as rodadas;
- **não pule as rodadas pares**: elas são o par de cada condição, e sem elas o
  split por rodada deixa o modelo sem referência;
- rodadas separadas e numeradas permitem dividir treino e teste sem vazamento.

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
