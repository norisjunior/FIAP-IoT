# Construir o firmware do app25, do zero

Sete etapas. A cada uma, o arquivo compila e faz mais uma coisa.

O que este firmware faz: mede o acelerômetro, fecha uma janela de 1 s, calcula
8 features, publica em `FIAPIoT/motor/multiclasse` e espera a classe voltar em
`FIAPIoT/motor/multiclasse/cmd`. Não há um único `if` sobre vibração ou
inclinação — quem decide é o `.pkl`.

Comece com `src/app25-multiclasse-inferencia.cpp` vazio.

---

## Etapa 0 — O contrato com o treino

Antes de escrever qualquer linha: o modelo aprendeu a partir de números
produzidos por um firmware específico, com uma configuração específica de
sensor. **Se a inferência produzir os números de outro jeito, o modelo recebe
features fora da distribuição em que foi treinado — e erra sem avisar.**

Sete coisas precisam ser idênticas às do `app17-7`. Elas aparecem marcadas
com **⚖ paridade** ao longo do guia:

| # | O quê | Valor |
|---|---|---|
| 1 | Chip do IMU | o **mesmo** usado na coleta (`MPU6500` ou `MPU6050`) |
| 2 | Fundo de escala | `setAccelRange(8)` |
| 3 | Filtro anti-aliasing | `setAccelLPF(41)` + `setGyroLPF(42)` |
| 4 | Calibração | `calibrateAccelGyro()`, na posição de uso, motor parado |
| 5 | Amostragem | 100 Hz, janela de 100 amostras |
| 6 | As 4 funções de feature | copiadas literalmente |
| 7 | Nomes e unidade | os 8 nomes exatos, valores em `g` |

O `platformio.ini` já vem pronto e é igual ao do `app17-7` — mesma versão de
plataforma e as mesmas três bibliotecas:

```ini
platform = espressif32@6.12.0
lib_deps =
    https://github.com/LiquidCGS/FastIMU.git#1.3.0
    knolleary/PubSubClient @ ^2.8
    bblanchon/ArduinoJson @ ^7.4.1
```

---

## Etapa 1 — Esqueleto: ler o acelerômetro

Objetivo: imprimir `accelX/Y/Z` no Monitor Serial. Nada de rede ainda.

```cpp
#include <Arduino.h>
#include "FastIMU.h"
#include <Wire.h>
#include <math.h>

#define SDA_PIN 22
#define SCL_PIN 23

/* ⚖ paridade 1 — o MESMO chip da coleta. Trocar aqui muda os números:
   os dois têm offsets e ruído diferentes. */
#define MPU_TYPE MPU6500
MPU_TYPE mpu;

calData calib = { 0 };

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);
  if (mpu.init(calib, 0x68) != 0) {
    Serial.println("Erro: MPU não encontrado");
    while (1);
  }

  /* ⚖ paridade 2 — fundo de escala ±8 g.
     Em ±2 g a mesma vibração satura; em ±16 g ela vira ruído de
     quantização. Os dois casos mudam std_* e p2p_mag. */
  mpu.setAccelRange(8);

  /* ⚖ paridade 3 — anti-aliasing. Amostramos a 100 Hz, então Nyquist é
     50 Hz: o filtro interno precisa cortar abaixo disso, senão a vibração
     acima de 50 Hz "dobra" para dentro da banda e infla std_mag.
     São duas chamadas porque nenhuma sozinha cobre os dois chips:
       MPU6500 -> setAccelLPF(41) escreve ACCEL_CONFIG2 (41 Hz).
                  setGyroLPF só afeta o giroscópio.
       MPU6050 -> setAccelLPF não é implementado no FastIMU (devolve -1);
                  o DLPF é COMPARTILHADO, então é setGyroLPF(42) que
                  filtra o acelerômetro (44 Hz). */
  mpu.setAccelLPF(41);
  mpu.setGyroLPF(42);

  /* ⚖ paridade 4 — calibra na posição de uso, com o motor PARADO.
     Os offsets entram em todas as leituras: calibrar inclinado desloca
     mean_ax/ay/az de todas as classes de uma vez.
     No Wokwi, comente esta linha: trava (FIFO ausente). */
  Serial.println("Deixe o motor na posicao inicial/de uso e nao o movimente durante a calibracao...");
  delay(2000);
  mpu.calibrateAccelGyro(&calib);
  mpu.init(calib, 0x68);

  Serial.println("MPU iniciado");
}

void loop() {
  AccelData accel;
  mpu.update();
  mpu.getAccel(&accel);
  Serial.printf("%.3f,%.3f,%.3f\r\n", accel.accelX, accel.accelY, accel.accelZ);
  delay(100);
}
```

**Confira antes de seguir:** com o motor parado e nivelado, `accelZ` fica
perto de `1.0` e os outros dois perto de `0`. Se `accelZ` der `~9.8`, a
biblioteca está devolvendo m/s² e não `g` — o treino usou `g`.

---

## Etapa 2 — Amostrar a 100 Hz e fechar a janela

Objetivo: encher três buffers de 100 amostras e avisar quando fecharem.

```cpp
/* ⚖ paridade 5 — 100 Hz, janela de 1 s.
   Mudar o Fs muda o conteúdo de frequência que cabe na janela; mudar o
   tamanho muda a estatística. As duas coisas deslocam TODAS as features. */
const int FS_HZ          = 100;
const int AMOSTRA_MS     = 1000 / FS_HZ;   // 10 ms
const int TAMANHO_JANELA = FS_HZ;          // 100 amostras = 1 s

float ax_buf[TAMANHO_JANELA];
float ay_buf[TAMANHO_JANELA];
float az_buf[TAMANHO_JANELA];
float mag_buf[TAMANHO_JANELA];

int indice = 0;
uint32_t tempoAnterior = 0;
```

No `loop()`, no lugar do `delay(100)`:

```cpp
void loop() {
  if (millis() - tempoAnterior >= AMOSTRA_MS) {
    // Passo FIXO, não "= millis()": assim o atraso de um ciclo não empurra
    // o próximo e a taxa não escorrega abaixo de 100 Hz.
    tempoAnterior += AMOSTRA_MS;
    // Mais de uma amostra atrasado (reconexão, publish lento)? Não adianta
    // amostrar em rajada para recuperar — sairiam sem espaçamento real.
    if (millis() - tempoAnterior >= AMOSTRA_MS) tempoAnterior = millis();

    AccelData accel;
    mpu.update();
    mpu.getAccel(&accel);
    ax_buf[indice] = accel.accelX;
    ay_buf[indice] = accel.accelY;
    az_buf[indice] = accel.accelZ;
    indice++;

    if (indice >= TAMANHO_JANELA) {
      Serial.println("janela fechada");
      indice = 0;
    }
  }
}
```

**Confira:** uma linha `janela fechada` por segundo, sem atrasos acumulando.

Para espiar os valores, imprima a **última amostra** da janela. Repare no
índice: quando o `if` dispara, `indice` já vale `100`, e as posições válidas
vão de `0` a `99`.

```cpp
    if (indice >= TAMANHO_JANELA) {
      Serial.printf("janela fechada | ultima amostra [%d]: %.3f,%.3f,%.3f\r\n",
                    TAMANHO_JANELA - 1,
                    ax_buf[TAMANHO_JANELA - 1],
                    ay_buf[TAMANHO_JANELA - 1],
                    az_buf[TAMANHO_JANELA - 1]);
      indice = 0;
    }
```

> **A armadilha.** Escrever `ax_buf[indice]` aqui lê a posição `100`, que não
> existe: o C++ não avisa, apenas lê o que estiver naquele endereço de memória.
> O sintoma é traiçoeiro — um dos eixos fica **sempre 0.000** e os outros dois
> mostram valores plausíveis, mas de outra amostra. Não é o sensor: é leitura
> fora do array. Use `TAMANHO_JANELA - 1`, ou `indice - 1` antes de zerar.

> O passo fixo é paridade também. Com `tempoAnterior = millis()` a taxa real
> cai para ~95 Hz e a janela passa a cobrir 1,05 s de sinal.

---

## Etapa 3 — As 8 features

**⚖ paridade 6.** Copie estas quatro funções **literalmente** do `app17-7`.
Não reescreva, não "melhore".

```cpp
float calcMean(float arr[], int n) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += arr[i];
  return soma / n;
}

float calcStd(float arr[], int n, float media) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += (arr[i] - media) * (arr[i] - media);
  return sqrt(soma / n);          // divide por n, não por (n-1)
}

void calcMagnitude(float axArr[], float ayArr[], float azArr[], float magArr[], int n) {
  for (int i = 0; i < n; i++) {
    magArr[i] = sqrt(axArr[i]*axArr[i] + ayArr[i]*ayArr[i] + azArr[i]*azArr[i]);
  }
}

float calcPtP(float arr[], int n) {
  float mn = arr[0], mx = arr[0];
  for (int i = 1; i < n; i++) {
    if (arr[i] < mn) mn = arr[i];
    if (arr[i] > mx) mx = arr[i];
  }
  return mx - mn;
}
```

> O `sqrt(soma / n)` é desvio-padrão **populacional**.

Dentro do `if (indice >= TAMANHO_JANELA)`:

```cpp
      float mx = calcMean(ax_buf, TAMANHO_JANELA);
      float my = calcMean(ay_buf, TAMANHO_JANELA);
      float mz = calcMean(az_buf, TAMANHO_JANELA);
      float sx = calcStd(ax_buf, TAMANHO_JANELA, mx);
      float sy = calcStd(ay_buf, TAMANHO_JANELA, my);
      float sz = calcStd(az_buf, TAMANHO_JANELA, mz);

      calcMagnitude(ax_buf, ay_buf, az_buf, mag_buf, TAMANHO_JANELA);
      float mMag   = calcMean(mag_buf, TAMANHO_JANELA);
      float stdMag = calcStd(mag_buf, TAMANHO_JANELA, mMag);
      float p2p    = calcPtP(mag_buf, TAMANHO_JANELA);

      Serial.printf("%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\r\n",
                    mx, my, mz, sx, sy, sz, stdMag, p2p);
      indice = 0;
```

**Confira:** nivelado e parado, `mean_az ≈ 1.0` e os `std_*` perto de zero.
Inclinado 25°, parte da gravidade migra para `mean_ax` (`sen 25° ≈ 0,42`).

> `mMag` é usada só como média intermediária para o `stdMag` — ela **não**
> é uma das 8 features. O modelo recebe `std_mag`, não `mean_mag`.

---

## Etapa 4 — Wi-Fi e MQTT

Agora a rede. Acrescente aos includes:

```cpp
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
```

```cpp
/* ---- (A) Wokwi ----
const char* WIFI_SSID     = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
#define MQTT_SERVER "host.wokwi.internal"
*/
// ---- (B) ESP32 físico ----
const char* WIFI_SSID     = "SUA_REDE";
const char* WIFI_PASSWORD = "SUA_SENHA";
#define MQTT_SERVER "192.168.0.100"   // IP da máquina com a IoT-platform

WiFiClient wifiClient;

#define MQTT_PORT      1883
#define MQTT_PUB_TOPIC "FIAPIoT/motor/multiclasse"       // a janela vai por aqui
#define MQTT_SUB_TOPIC "FIAPIoT/motor/multiclasse/cmd"   // a classe volta por aqui
#define MQTT_CLIENT_ID "IoTDevInferenciaMultiClasse001"
PubSubClient mqttClient(wifiClient);
```

> **Sem NTP.** O `app17-7` sincronizava o relógio por NTP para carimbar cada
> janela com `ts_epoch_ms`. Ele precisava: as janelas iam para um BANCO, onde
> o tempo é o eixo e a ordem importa. Aqui a janela vale **agora** — é medida,
> classificada e respondida em menos de um segundo, e depois não serve para
> mais nada. Sem timestamp no payload, o NTP inteiro sai: `<time.h>`, os dois
> servidores, `sincronizarRelogio()` e `agoraEpochMs()`.

Declare os dois protótipos junto com os outros, antes do `setup()`:

```cpp
void conectarWiFi();
void conectarMQTT();
```

**O Wi-Fi.** Fica num laço até conectar: sem rede não há o que publicar, e o
resto do firmware não tem o que fazer.

```cpp
void conectarWiFi() {
  Serial.printf("Conectando ao WiFi %s", WIFI_SSID);
  // TxPower reduzido: evita brownout/reboot ao ligar o rádio nesta placa.
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_2dBm);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setSleep(false);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println("");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}
```

Três linhas nessa função não são óbvias:

- **`WiFi.mode(WIFI_STA)`** — o ESP32 liga em modo dual (estação **e** ponto de
  acesso) por padrão. Só precisamos de estação; o AP consome rádio à toa.
- **`WiFi.setTxPower(WIFI_POWER_2dBm)`** — potência reduzida. Ligar o rádio na
  potência cheia dá um pico de corrente que, em placas alimentadas pela USB do
  notebook, derruba a tensão e reinicia o ESP32 (*brownout*). Se a sua placa
  estiver longe do roteador e não conectar, é aqui que se aumenta.
- **`WiFi.setSleep(false)`** — desliga o modo de economia do rádio. Com ele
  ligado, o ESP32 dorme entre pacotes e a resposta da nuvem chega com centenas
  de milissegundos de atraso.

O `delay(500)` aqui é aceitável pelo mesmo motivo do teste de saídas: isto roda
no `setup()`, antes de a amostragem de 100 Hz começar.

**O MQTT.** Igual ao do `app17-7`, com **uma linha nova** — o `subscribe`, que
é o que faz este app receber a resposta:

```cpp
void conectarMQTT() {
  while (!mqttClient.connected()) {
    Serial.printf("Conectando ao MQTT Broker %s...", MQTT_SERVER);
    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      Serial.println(" Conectado!");
      mqttClient.subscribe(MQTT_SUB_TOPIC);          // <-- novo
      Serial.printf("Inscrito em: %s\r\n", MQTT_SUB_TOPIC);
    } else {
      Serial.printf(" Falha rc=%d. Tentando em 5s...\r\n", mqttClient.state());
      delay(5000);
    }
  }
}
```

No `setup()`, depois da calibração:

```cpp
  conectarWiFi();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setKeepAlive(60);
  mqttClient.setSocketTimeout(30);
  mqttClient.setBufferSize(512);
```

E no topo do `loop()`:

```cpp
  if (!mqttClient.connected()) {
    conectarMQTT();
  }
  mqttClient.loop();
```

O `mqttClient.loop()` precisa rodar a cada volta: é ele que processa as
mensagens que chegam e mantém a conexão viva. Sem essa linha o `subscribe`
não serve para nada — o broker até envia, mas o firmware nunca lê.

**Confira antes de seguir:** o Monitor Serial deve mostrar

```text
Conectando ao WiFi SUA_REDE....
IP: 192.168.0.123
Conectando ao MQTT Broker 192.168.0.100... Conectado!
Inscrito em: FIAPIoT/motor/multiclasse/cmd
```

Se travar nos pontinhos do Wi-Fi, o SSID ou a senha estão errados — o ESP32
só enxerga redes de **2,4 GHz**, então confira também se não é a rede de 5 GHz.
Se o Wi-Fi conectar mas o MQTT ficar em `Falha rc=-2`, o IP do broker está
errado ou a porta 1883 está bloqueada; o `rc` do `PubSubClient` é negativo para
falha de rede e positivo para recusa do protocolo.

---

## Etapa 5 — Publicar a janela

**⚖ paridade 7.** Os oito nomes são o contrato com o `.pkl`. Eles aparecem
em quatro lugares — aqui, no `FEATURES` do notebook, no `feature_names_in_`
do modelo e no `FEATURES` da API. Um erro de digitação em qualquer um deles
quebra o conjunto.

```cpp
void publicarJanela(float mx, float my, float mz,
                    float sx, float sy, float sz,
                    float stdMag, float p2p) {
  JsonDocument doc;
  doc["device"]  = MQTT_CLIENT_ID;
  doc["mean_ax"] = serialized(String(mx, 3));
  doc["mean_ay"] = serialized(String(my, 3));
  doc["mean_az"] = serialized(String(mz, 3));
  doc["std_ax"]  = serialized(String(sx, 3));
  doc["std_ay"]  = serialized(String(sy, 3));
  doc["std_az"]  = serialized(String(sz, 3));
  doc["std_mag"] = serialized(String(stdMag, 3));
  doc["p2p_mag"] = serialized(String(p2p, 3));

  String buffer;
  serializeJson(doc, buffer);

  if (!mqttClient.publish(MQTT_PUB_TOPIC, buffer.c_str())) {
    Serial.println("MQTT: falha no envio");
  }
}
```

Troque o `Serial.printf` da etapa 3 por:

```cpp
      publicarJanela(mx, my, mz, sx, sy, sz, stdMag, p2p);
      indice = 0;
```

> **O que NÃO vai no payload.** O `app17-7` publicava mais quatro campos, e
> os quatro saem por motivos diferentes:
>
> | campo | por que existia no app17-7 | por que sai aqui |
> |---|---|---|
> | `label` | o gabarito humano, do botão 18 | é justamente o que estamos perguntando |
> | `rodada` | agrupava as sessões para o split | não há treino aqui |
> | `janela` | contava até as 30 da coleta | a coleta não termina |
> | `ts_epoch_ms` | o tempo é o eixo do banco | a janela vale agora, e some em seguida |
>
> Sobra `device` mais as 8 features: 9 campos contra 13.
>
> O `serialized(String(x, 3))` mantém 3 casas decimais e evita a notação
> científica do ArduinoJson — o mesmo formato que foi gravado no InfluxDB.

**Confira sem nenhuma API no ar:**

```bash
mosquitto_sub -h localhost -t "FIAPIoT/motor/multiclasse" -v
```

Uma mensagem por segundo, com os 9 campos:

```json
{"device":"IoTDevInferenciaMultiClasse001","mean_ax":0.328,"mean_ay":-0.008,"mean_az":0.929,"std_ax":0.036,"std_ay":0.030,"std_az":0.042,"std_mag":0.046,"p2p_mag":0.215}
```

---

## Etapa 6 — As saídas: 3 LEDs e um buzzer

Uma saída por classe. A que estiver ligada é a resposta da nuvem.

| GPIO | Componente | Classe |
|---|---|---|
| 4 | LED azul | `operando` |
| 21 | LED amarelo | `inclinado_frente` |
| 18 | LED vermelho | `inclinado_tras` |
| 19 | buzzer | `anomalia` |
| 2 | LED onboard | aceso = conectado ao broker |

> Os pinos **21 e 18 eram os dois botões do `app17-7`**. Lá eles serviam para
> um humano informar a classe; aqui mostram a classe que o modelo escolheu.
> O mesmo par de pinos troca de lado quando o app troca de papel.

```cpp
#define LED_AZUL      4   // operando
#define LED_AMARELO  21   // inclinado_frente
#define LED_VERMELHO 18   // inclinado_tras
#define BUZZER       19   // anomalia
#define LED_ONBOARD   2   // LED onboard: aceso = conectado ao broker
```

Uma função só, que vamos usar sempre antes de acender a saída nova:

```cpp
void apagarTodasAsSaidas() {
  digitalWrite(LED_AZUL,     LOW);
  digitalWrite(LED_AMARELO,  LOW);
  digitalWrite(LED_VERMELHO, LOW);
  digitalWrite(BUZZER,       LOW);
}
```

No `setup()`, os `pinMode` e um **teste de ligação**:

```cpp
  // As quatro saídas de classe, mais o LED da placa.
  pinMode(LED_AZUL,     OUTPUT);
  pinMode(LED_AMARELO,  OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(BUZZER,       OUTPUT);
  pinMode(LED_ONBOARD,  OUTPUT);

  apagarTodasAsSaidas();
  digitalWrite(LED_ONBOARD, LOW);

  // Teste de ligação: acende uma saída por vez, para você conferir se cada
  // componente está no pino certo ANTES de depender do modelo. Se o LED
  // amarelo não acender aqui, o problema é o fio — não a rede neural.
  // Os delay() aqui não atrapalham: o setup roda uma vez, antes do loop.
  Serial.println("Testando as saidas...");
  digitalWrite(LED_AZUL,     HIGH); delay(400); digitalWrite(LED_AZUL,     LOW);
  digitalWrite(LED_AMARELO,  HIGH); delay(400); digitalWrite(LED_AMARELO,  LOW);
  digitalWrite(LED_VERMELHO, HIGH); delay(400); digitalWrite(LED_VERMELHO, LOW);
  tone(BUZZER, 500, 250); noTone(BUZZER);
```

E no `loop()`, logo abaixo do `mqttClient.loop()`, só o LED da placa:

```cpp
  digitalWrite(LED_ONBOARD, mqttClient.connected() ? HIGH : LOW);
```

**Confira:** ao ligar, os três LEDs acendem em sequência e o buzzer dá um bipe
curto. Se algum não responder, o problema é a ligação — resolva antes de seguir.

> **Por que `tone()` no buzzer, e não `digitalWrite()`?** Depende do
> componente. Um buzzer **ativo** tem oscilador próprio e apita com
> `digitalWrite(HIGH)`. Um buzzer **passivo** é só um alto-falante: com nível
> constante ele dá um clique e cala. Precisa de um sinal oscilando, e é isso
> que `tone(BUZZER, 500, 250)` faz — 500 Hz durante 250 ms. Se o seu buzzer
> ficou mudo com `digitalWrite`, ele é passivo: use `tone()`.

> **`delay()` no `setup()` pode; no `loop()`, não.** O `setup()` roda uma vez,
> antes de a amostragem começar. Um `delay(400)` dentro do `loop()` faria a
> taxa de 100 Hz desmoronar e as features saírem erradas.
>
> O `tone()` é a exceção que confirma a regra: **ele não bloqueia**. No ESP32
> a chamada só põe um comando numa fila, e quem espera os 250 ms é uma *task*
> separada do FreeRTOS. Por isso ele pode ser chamado de dentro do `loop()` sem
> derrubar a amostragem — e por isso o `noTone()` logo depois não corta o bipe:
> ele entra na mesma fila, atrás, e só executa quando o tom já acabou.

---

## Etapa 7 — Receber a classe e acender a saída

O n8n devolve a **string pura** da classe em `.../cmd` (`inclinado_tras`, e
não um JSON). Comparamos com os quatro nomes possíveis e acendemos a saída
daquele que casar.

> **O fluxo tem dois destinos, o firmware só conhece um.** Depois de
> classificar, o n8n abre em dois ramos paralelos: um publica em `.../cmd`
> (é este, o do dispositivo, a cada segundo) e o outro manda um alerta no
> Telegram a cada predição `anomalia`. Nada disso muda o firmware: ele
> assina um tópico e reage ao que chegar. Trocar o alerta de Telegram por
> e-mail, ou tirá-lo, não faz o ESP32 nem piscar diferente.

```cpp
void receberComando(char* topico, byte* conteudo, unsigned int tamanho) {
  // O payload MQTT não termina em '\0': por isso o String recebe o tamanho.
  String classe(conteudo, tamanho);
  classe.trim();

  // Mostra o que chegou ANTES de julgar: se o payload vier errado (um JSON
  // inteiro, por exemplo), é aqui que se vê o que o n8n publicou de fato.
  Serial.println("--- MQTT recebido ---");
  Serial.printf("  topico:  %s
", topico);
  Serial.printf("  tamanho: %u bytes
", tamanho);
  Serial.printf("  payload: \"%s\"
", classe.c_str());

  // Passo 1: apaga tudo. Assim nunca ficam duas saídas ligadas ao mesmo tempo.
  apagarTodasAsSaidas();

  // Passo 2: acende SÓ a saída da classe que chegou.
  if (classe == "operando") {
    digitalWrite(LED_AZUL, HIGH);
    Serial.println("  MODELO:  operando         -> LED azul aceso");

  } else if (classe == "inclinado_frente") {
    digitalWrite(LED_AMARELO, HIGH);
    Serial.println("  MODELO:  inclinado_frente -> LED amarelo aceso");

  } else if (classe == "inclinado_tras") {
    digitalWrite(LED_VERMELHO, HIGH);
    Serial.println("  MODELO:  inclinado_tras   -> LED vermelho aceso");

  } else if (classe == "anomalia") {
    tone(BUZZER, 500, 250);
    Serial.println("  MODELO:  anomalia         -> BUZZER ligado");

  } else {
    // Nenhum nome casou. As saídas ficam todas apagadas, e isso é proposital:
    // apagado avisa que algo está errado, melhor do que manter a última classe
    // e parecer que o sistema continua funcionando.
    Serial.println("  MODELO:  classe DESCONHECIDA - tudo apagado");
    Serial.println("           esperado, sem aspas e sem JSON:");
    Serial.println("           operando | inclinado_frente | inclinado_tras | anomalia");
  }

  Serial.println("---------------------");
}
```

Registre o callback no `setup()`, logo depois do `setServer`:

```cpp
  mqttClient.setCallback(receberComando);
```

> **Por que não um `switch`?** Em C++ o `switch` só aceita inteiro, `char` ou
> `enum` — com `String` o compilador responde `switch quantity not an integer`.
> Para usar um, seria preciso primeiro converter o nome em número, e o único
> jeito de fazer isso é comparando as strings uma a uma: o `if/else` aconteceria
> do mesmo jeito, com um `switch` em cima. O `if/else` encadeado é o idioma
> correto de C++ para despachar por string.

### Três coisas a reparar nesta função:

1. **É a única que mexe nas saídas.** O `loop()` não cuida de LED nenhum —
   quem acende é quem recebe a mensagem.
2. **Não há tempo nenhum aqui.** Acendeu, *fica* aceso até chegar a próxima
   mensagem — e ela chega a cada segundo. A saída é a memória do dispositivo.
3. **Comparamos NOMES, não números.** Em nenhum lugar do firmware existe um
   índice de classe. Se a nuvem mandasse `2` em vez de `inclinado_tras`, o
   firmware teria que manter uma tabela em sincronia com o Colab e com a API
   — e um dia alguém esqueceria de atualizar uma das três.

O parâmetro `topico` existe na assinatura mas costuma ficar sem uso. Aqui ele
serve: com mais de uma assinatura ativa, é o que diz de onde a mensagem veio.

**Confira sem o n8n:**

```bash
mosquitto_pub -h localhost -t "FIAPIoT/motor/multiclasse/cmd" -m "anomalia"
```

O buzzer liga e o Serial imprime:

```text
--- MQTT recebido ---
  topico:  FIAPIoT/motor/multiclasse/cmd
  tamanho: 8 bytes
  payload: "anomalia"
  MODELO:  anomalia         -> BUZZER ligado
---------------------
```

Publique `operando` em seguida: o buzzer cala e o LED azul acende.

> Se o `payload:` mostrar um JSON inteiro em vez do nome, o `Send Input Data`
> do nó MQTT do n8n ficou ligado. Veja o README. É exatamente para esse caso
> que o payload é impresso cru, antes de qualquer comparação.

> **Os três LEDs são estado; o buzzer é evento.** Um LED aceso **fica** aceso
> até a próxima mensagem. O `tone(BUZZER, 500, 250)` não: ele dá um bipe de
> 250 ms e cala sozinho. Como a nuvem responde uma vez por segundo, enquanto
> durar a `anomalia` isso vira um **bipe por segundo** — alarme intermitente,
> de graça, sem nenhum controle de tempo no `loop()`. Um `digitalWrite(HIGH)`
> daria um zumbido contínuo, correto para um alarme mas insuportável numa sala
> com a turma toda testando.
>
> Para mudar o alarme, mexa nos dois números: `tone(BUZZER, 500, 250)` é
> frequência em Hz e duração em ms. Mais agudo, `1500`; mais curto, `100`.

---

## Checklist de paridade

Antes de acreditar numa predição, confira contra o `app17-7` que gerou o
dataset:

- [ ] `#define MPU_TYPE` — o mesmo chip da coleta
- [ ] `setAccelRange(8)`
- [ ] `setAccelLPF(41)` **e** `setGyroLPF(42)`, nesta ordem
- [ ] `calibrateAccelGyro()` presente, feita na posição de uso e motor parado
- [ ] `FS_HZ = 100`, `TAMANHO_JANELA = 100`
- [ ] `tempoAnterior += AMOSTRA_MS` (passo fixo)
- [ ] `calcStd` dividindo por `n`
- [ ] os 8 nomes, escritos igual nos quatro lugares
- [ ] valores em `g` (nivelado: `mean_az ≈ 1.0`)
- [ ] montagem física do sensor na mesma orientação da coleta

O último item não está no código e é o que mais quebra na prática: girar o
sensor 90° no gabarito troca `mean_ax` por `mean_ay`, e o modelo passa a
confundir `inclinado_frente` com `inclinado_tras`. Se as inclinações
saírem trocadas, suspeite da montagem antes de suspeitar do modelo.

## Teste do caminho inteiro

Na ordem, parando no primeiro que falhar:

```bash
# 1. a API, sozinha
curl -X POST http://localhost:8000/predict \
  -H "Content-Type: application/json" \
  -d '{"mean_ax":-0.413,"mean_ay":0.811,"mean_az":0.965,"std_ax":0.017,"std_ay":0.011,"std_az":0.005,"std_mag":0.01,"p2p_mag":0.04}'

# 2. o firmware publicando
mosquitto_sub -h localhost -t "FIAPIoT/motor/multiclasse" -v

# 3. o n8n respondendo
mosquitto_sub -h localhost -t "FIAPIoT/motor/multiclasse/cmd" -v

# 4. o firmware obedecendo
mosquitto_pub -h localhost -t "FIAPIoT/motor/multiclasse/cmd" -m "anomalia"
```

O degrau 4 testa **o firmware inteiro sem n8n e sem API**: o buzzer tem que
tocar. Se tocar aqui e não tocar com o fluxo ligado, o problema está no n8n —
provavelmente o `Send Input Data`.

> **O alerta do Telegram fica no outro ramo do n8n.** O IF envia uma mensagem
> a cada resposta `anomalia` da API, mesmo se a classe se repetir.
> Publicar diretamente em `.../cmd` testa apenas a saída do ESP32, sem acionar o Telegram.

No Wokwi não há como inclinar o MPU: só `operando` e `anomalia` têm
equivalente no simulador. O ciclo MQTT → n8n → API → MQTT, esse funciona
inteiro.
