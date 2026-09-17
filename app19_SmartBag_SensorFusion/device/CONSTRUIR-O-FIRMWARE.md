# Construir o firmware do app19, do zero

**O app15 publicava sempre e obedecia à nuvem.** Aqui o dispositivo faz o contrário:
publica **só quando um humano manda**, e não recebe nada de volta. É um gerador de
dataset — quem rotula cada amostra é a pessoa que aperta o botão, não um limiar.

**Repare no que sai.** O `callbackMQTT`, o tópico `cmd` e o `subscribe` do app15 **são
removidos**. Um gerador de dataset que aceita comando da nuvem contamina o próprio
dataset: a nuvem mexeria no dispositivo no meio da coleta, e você não saberia quais
amostras foram afetadas.

Quatro iterações. Cada uma compila e roda.

1. Duas medidas novas no Serial: luz e inclinação.
2. Os dois botões: rodada e situação.
3. O baseline da distância.
4. A amostra de um segundo indo por MQTT.

---

## Iteração 0 — A base

Copie `app15_NexoLog_PUB_SUB` inteiro e renomeie a pasta para `device`. Renomeie o
`.ino` para `app19-SmartBag.ino`. Ele precisa estar rodando as três iterações do guia de
firmware do app15 antes de continuar: publicando JSON a cada 1 s e obedecendo ao comando
que chega em `cmd`.

**Paridade com o app15** — o que não muda:

| O quê | Valor |
|---|---|
| Sensores e pinos | DHT 4, TRIG 19, ECHO 18, SDA 22, SCL 23 — os mesmos |
| Headers `ESP32Sensors*.hpp` | `Ambiente`, `Distancia`, `Accel` — os mesmos arquivos |
| Saída no Serial | CSV com `\r\n`, linha de título no `setup()` |
| Reconexão Wi-Fi/MQTT | por `millis()`, `INTERVALO_RECONEXAO` 5000 |
| `mqttClient.setKeepAlive(120)` | mantido |
| Intervalo de publicação | 1000 ms |
| Client ID | um por equipe |

**O que sai:**

| O quê | Por quê |
|---|---|
| `callbackMQTT`, `setCallback`, `subscribe`, `MQTT_SUB_TOPIC` | Aqui ninguém manda no dispositivo |
| `accel_x`, `accel_y`, `accel_z` no JSON | Viram duas features derivadas, não três eixos crus |
| `dist` no JSON | Vira `delta_distancia`, relativo ao baseline |

**O que entra:** dois botões, um LDR, e três arquivos —
`ESP32SensorsLDR.hpp`, `ESP32SensorsLED.hpp` e uma função no `ESP32SensorsAccel.hpp`.

Troque também o tópico e o Client ID, senão você grava por cima do NexoLog:

```cpp
#define MQTT_CLIENT_ID "SmartBagEquipe01"
#define MQTT_PUB_TOPIC "FIAPIoT/smartbag/equipe01/dados"
```

---

## Iteração 1 — Luz e inclinação

Duas medidas novas. Nada de botão ainda: o programa continua publicando a cada segundo,
como o app15.

**a) O LDR.** Crie `src/ESP32SensorsLDR.hpp`:

```cpp
#pragma once
#include <Arduino.h>

namespace ESP32Sensors {
  namespace LDR {
    static uint8_t ldrPin = 0;

    void inicializar(uint8_t pin) {
      ldrPin = pin;
      analogReadResolution(12);
      pinMode(pin, INPUT);
    }

    int ler() {
      return analogRead(ldrPin);
    }
  }
}
```

**Três linhas de função, e é de propósito.** O app30 tem um `ESP32SensorsLDR.hpp` que
converte para lux, com resistência, gama e logaritmo. Aqui a feature é o **RAW de 0 a
4095**, sem conversão nenhuma.

Converter para lux não acrescenta informação — é uma função monótona do RAW, e o modelo
aprende a mesma coisa nas duas escalas. O que a conversão acrescenta é uma fórmula com
três constantes que dependem do módulo exato: troque o LDR e o lux mente, enquanto o RAW
continua sendo o que o conversor leu. Para dataset, o cru é mais honesto que o
convertido.

Só não esqueça: **preserve a montagem do LDR entre a coleta e a inferência**. Se o RAW é
a feature, a posição do sensor faz parte do modelo.

**b) A inclinação.** Acrescente ao final do namespace `Accel`, em
`ESP32SensorsAccel.hpp`, junto de `medirMovimentacao`:

```cpp
    float medirInclinacao(AccelData accel) {
      float x = accel.accelX, y = accel.accelY, z = accel.accelZ;
      float norma = sqrtf(x*x + y*y + z*z);
      if (!isfinite(norma) || norma < 0.1f) return NAN;
      // MPU no corpo da bag: +Z para cima quando ela esta na vertical.
      return acosf(constrain(z / norma, -1.0f, 1.0f)) * 180.0f / PI;
    }
```

O ângulo entre o vetor de aceleração e o eixo vertical de montagem. `constrain` antes do
`acos` porque erro de arredondamento faz `z/norma` passar de 1,0 e o `acos` devolver NaN
sem avisar. `norma < 0.1f` descarta queda livre, onde não existe vertical para medir.

**c) No `.ino`**, inclua os dois headers, acrescente o pino e inicialize:

```cpp
const uint8_t LDR_PIN = 35;
```

Publique as duas no Serial, junto das que já saem. Compile, abra o Wokwi e confira:

- [ ] Luz entre 0 e 4095, e **mais luz reduz o número** neste módulo
- [ ] MPU parado com +Z para cima: inclinação perto de **0°**
- [ ] Deitado, eixo horizontal para cima: perto de **90°**
- [ ] De cabeça para baixo: perto de **180°**

> O ângulo pelo acelerômetro é uma **aproximação**: ele mede para onde aponta a
> aceleração total, não para onde aponta a gravidade. Um buraco levanta o movimento
> **e** a inclinação estimada ao mesmo tempo. Isso é uma limitação real deste exemplo, e
> ela vai aparecer nos dados. Não tente corrigir com filtro aqui.

---

## Iteração 2 — Os dois botões

Este é o pedaço que vem do **app17-7**, igual: dois botões em `INPUT_PULLUP`, debounce de
300 ms tratado no próprio `loop()`, e uma sequência cíclica que incrementa a rodada ao
fechar a volta.

**a) Pinos e estado:**

```cpp
const uint8_t LED_PIN = 21;
const uint8_t BTN_COLETA = 27;
const uint8_t BTN_SITUACAO = 26;

bool coletando = false;
String situacao = "";
uint32_t rodada = 1;
uint8_t indiceSituacao = 0;
const char* SITUACOES[] = {
  "parada_fechada", "transporte_normal", "buraco", "aberta_parada",
  "aberta_movimento", "tombamento", "problema_termico"
};
const uint8_t TOTAL_SITUACOES = sizeof(SITUACOES) / sizeof(SITUACOES[0]);

int ultimoBotaoColeta = HIGH, ultimoBotaoSituacao = HIGH;
unsigned long ultimoDebounceColeta = 0, ultimoDebounceSituacao = 0;
const unsigned long debounceMs = 300;
```

**b) No `setup()`**, os dois `pinMode(..., INPUT_PULLUP)` e `situacao = SITUACOES[0];`.

**c) No começo do `loop()`**, os dois blocos de botão (estão no arquivo pronto). O
segundo só age com a coleta **parada** — trocar de situação no meio de uma coleta
misturaria duas classes na mesma rodada, sem deixar rastro no CSV.

**d) O LED** acende com a coleta e apaga sem ela. Só isso.

No app15 o LED era o **alerta decidido pela nuvem**. Aqui ele é o **indicador de estado
de gravação**, como a luz vermelha de uma câmera. Ele não significa "problema" — quem vai
dizer se é problema é o modelo, nos apps 20 a 22, e aí o LED volta a ter esse sentido.

**Rodada é o grupo do split.** Uma rodada é uma volta completa pelas sete situações.
Depois da sétima, o botão SITUAÇÃO volta à primeira **e incrementa `rodada`**. No Colab,
a última rodada inteira vira o conjunto de teste — por isso ela não pode estar pela
metade, e por isso você não pode reiniciar o ESP32 no meio da coleta: `rodada` volta a 1
e duas rodadas diferentes passam a ter o mesmo número.

Compile e confira **sem olhar o MQTT ainda**:

- [ ] COLETA acende e apaga o LED, e o Serial mostra `[COLETA] INICIADA/PARADA`
- [ ] SITUAÇÃO avança na lista, só com a coleta parada
- [ ] Depois de `problema_termico`, volta a `parada_fechada` e a rodada vira 2
- [ ] Parar e iniciar de novo **não** mexe na rodada nem na situação

---

## Iteração 3 — Baseline e a janela de um segundo

**a) O baseline.** A bag fechada não tem distância zero — tem a distância entre o sensor
na tampa e a carga. Essa distância muda a cada carga. Então o que entra no modelo não é
a distância, é o **quanto ela mudou desde que a bag foi fechada**:

```cpp
void calibrarDistancia() {
  float soma = 0;
  int validas = 0;
  unsigned long inicio = millis();
  while (validas < 5 && millis() - inicio < 5000) {
    float dist = ESP32Sensors::Distancia::medirDistancia();
    if (isfinite(dist)) { soma += dist; validas++; }
    delay(60);
  }
  distBase = validas == 5 ? soma / validas : NAN;
  Serial.printf("[BASELINE] %.2f cm\r\n", distBase);
  baselinePendente = !isfinite(distBase);
  if (baselinePendente) Serial.println("[BASELINE] Sem medida. Feche a bag e tente na proxima coleta.");
}
```

Cinco leituras válidas, espaçadas em 60 ms, com teto de 5 s. É a **única** função
bloqueante do programa, e pode ser: ela roda uma vez por rodada, com a bag parada, antes
de qualquer amostra sair. Se falhar, `distBase` fica NaN e o delta sai `null` — nunca
400 cm, que é o que o HC-SR04 "responde" quando não volta eco.

O `medirDistancia()` desta versão devolve `float` direto, em cm, ou `NAN` sem eco. O
app15 devolvia uma `struct DISTANCIA` com um `bool valido`; com `isfinite()` o bool era
informação repetida.

**b) Os máximos.** O MPU é lido a cada 50 ms, mas só sai **uma** amostra por segundo.
O que vai no CSV é o **pico** de cada janela:

```cpp
movMax = fmaxf(movMax, ESP32Sensors::Accel::medirMovimentacao(accel));
inclMax = fmaxf(inclMax, ESP32Sensors::Accel::medirInclinacao(accel));
```

Os dois começam em `NAN`, e `fmaxf(NAN, x)` devolve `x` — então o primeiro valor válido
entra sozinho, sem precisar de flag de "primeira leitura". Se não houve leitura nenhuma
no segundo, continuam `NAN` e saem `null`.

**Zere os dois logo depois de publicar**, e também ao iniciar a coleta. Carregar o pico
de uma janela para a seguinte é o erro clássico aqui: um impacto isolado apareceria em
todos os segundos seguintes, e o modelo aprenderia a rotular pelo passado.

> **Máximo não guarda ordem.** Uma abertura no fim do segundo e um impacto no começo do
> mesmo segundo produzem exatamente a mesma linha no CSV que os dois simultâneos. É a
> segunda limitação intencional deste exemplo. Se quiser preservar ordem, é o app17 —
> sinal, janela e features de forma de onda.

**c) O amortecimento do DHT.** Ele continua sendo lido a cada 2500 ms, **mesmo com a
coleta parada**, e a última leitura válida fica em cache. Assim, quando você aperta
COLETA, a primeira amostra já sai com temperatura — em vez de `null` nos primeiros
segundos de toda coleta.

---

## Iteração 4 — A amostra indo por MQTT

Nove campos: as seis features e três identificadores.

```cpp
  JsonDocument doc;
  doc["device"] = MQTT_CLIENT_ID;
  doc["rodada"] = rodada;
  doc["situacao"] = situacao;
  doc["temperatura"] = ambiente.temp;
  doc["umidade"] = ambiente.umid;
  doc["delta_distancia"] = dist - distBase;
  doc["luz"] = luz;
  doc["mov_max"] = movMax;
  doc["incl_max"] = inclMax;
```

E a publicação só acontece dentro do `if (coletando && ...)`.

**O que NÃO vai no JSON**, e por quê:

| Campo tentador | Por que fica de fora |
|---|---|
| `coletando` | O ESP32 só publica coletando. O campo seria sempre `true` |
| `segundo_coleta` | Tempo dentro da coleta não é feature; seria um atalho para o modelo |
| `distancia`, `dist_base` | O modelo usa o delta. Mandar as três convida a usar a absoluta |
| `valido`, `falhas_distancia` | `null` já diz que faltou. Contador é telemetria de produção |

`NAN` vira `null` no JSON sozinho — o ArduinoJson faz isso por padrão. Você não precisa
testar `isnan()` em lugar nenhum.

**Confira no Serial e no `mosquitto_sub`:**

- [ ] Uma linha por segundo, **só** com a coleta ligada
- [ ] Nove campos, nem um a mais
- [ ] Tape o HC-SR04: `delta_distancia` vira `null`, o LED **não** muda
- [ ] Bag fechada e parada: delta perto de 0, `mov_max` perto de 0
- [ ] Baseline 10 cm, afaste para 15 cm: delta **+5**. Aproxime para 8 cm: delta **−2**

O sinal do delta é preservado de propósito. Abrir a tampa afasta a carga do sensor
(delta positivo); a carga subir/tombar contra o sensor aproxima (delta negativo). São
eventos diferentes e o sinal é o que os separa.

---

## Ordem do arquivo

Configurações → `setup()`/`loop()` → baseline → conexão → publicação. Sensores continuam
em funções nos headers `ESP32Sensors*.hpp`, como desde o app14.

## Depois do firmware

Transporte para o InfluxDB: [CONSTRUIR-O-FLUXO.md](../NodeRED/CONSTRUIR-O-FLUXO.md).
Protocolo das duas rodadas: [Guia-SmartBag.md](../Guia-SmartBag.md).
