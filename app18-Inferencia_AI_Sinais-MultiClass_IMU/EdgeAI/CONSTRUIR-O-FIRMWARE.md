# Construir o firmware da borda, do zero

Cinco etapas. A cada uma, o arquivo compila e faz mais uma coisa.

O que este firmware faz: mede o acelerômetro, fecha uma janela de 1 s, calcula
8 features, padroniza, entrega a uma Random Forest que mora na própria flash e
acende a saída da classe prevista. Não há um único `if` sobre vibração ou
inclinação — e também não há rede nenhuma.

Comece com `device/src/inferencia-na-borda.cpp` vazio.

---

## Etapa 0 — O contrato com o treino

Antes de escrever qualquer linha: o modelo aprendeu a partir de números
produzidos por um firmware específico, com uma configuração específica de
sensor. **Se a inferência produzir os números de outro jeito, o modelo recebe
features fora da distribuição em que foi treinado — e erra sem avisar.**

Nove coisas precisam ser idênticas às do app de coleta, que gerou o dataset. Elas
aparecem marcadas com **⚖ paridade** ao longo do guia:

| # | O quê | Valor |
|---|---|---|
| 1 | Chip do IMU | o **mesmo** usado na coleta (`MPU6500` ou `MPU6050`) |
| 2 | Fundo de escala | `setAccelRange(8)` |
| 3 | Filtro anti-aliasing | `setAccelLPF(41)` + `setGyroLPF(42)` |
| 4 | Calibração | `calibrateAccelGyro()`, na posição de uso, motor parado |
| 5 | Amostragem | 100 Hz, janela de 100 amostras |
| 6 | As 4 funções de feature | copiadas literalmente |
| 7 | Nomes e unidade | os 8 nomes exatos, valores em `g` |
| 8 | **A ordem das 8 features** | a ordem das colunas no Colab vira a ordem de `x[]` |
| 9 | **A ordem das 4 classes** | `classes_` (alfabética) vira o vetor `NOMES_CLASSES[]` |

Os itens 8 e 9 são novos: eles não existiam na versão com API, onde o JSON
levava cada feature pelo **nome** e a classe voltava como **texto**. Aqui não há
JSON e não há texto: entra um vetor de 8 posições e sai um número de 0 a 3.
Posição errada e número errado não causam erro de compilação — causam predição
errada, calada.

O `platformio.ini` já vem pronto. Tem uma biblioteca só, porque não há mais rede:

```ini
[env:esp32]
platform = espressif32@6.12.0
framework = arduino
board = esp32dev
lib_deps =
    https://github.com/LiquidCGS/FastIMU.git#1.3.0
```

Compare com o `platformio.ini` da versão com API: saíram o `PubSubClient` e o
`ArduinoJson`. Ninguém precisa falar com um broker nem montar um JSON quando a
resposta nasce dentro da placa.

---

## Etapa 1 — Esqueleto: ler o acelerômetro

Objetivo: imprimir `accelX/Y/Z` no Monitor Serial.

**⚖ paridade 1, 2, 3 e 4** estão todas nesta etapa. São quatro linhas fáceis de
copiar errado e impossíveis de descobrir depois: o erro não aparece como falha,
aparece como um modelo que "não funciona direito".

```cpp
#include <Arduino.h>
#include "FastIMU.h"
#include <Wire.h>
#include <math.h>

#define SDA_PIN 22
#define SCL_PIN 23

// ⚖ paridade 1: o MESMO chip usado na coleta.
// No Wokwi, troque para MPU6050.
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

  mpu.setAccelRange(8);   // ⚖ paridade 2

  // ⚖ paridade 3: anti-aliasing para amostrar a 100 Hz (Nyquist 50 Hz).
  // São duas chamadas porque nenhuma sozinha cobre os dois chips:
  mpu.setAccelLPF(41);   // MPU6500: escreve ACCEL_CONFIG2 -> 41 Hz.
                         // MPU6050: o FastIMU não implementa, devolve -1 sem fazer nada.
  mpu.setGyroLPF(42);    // MPU6050: o DLPF é COMPARTILHADO, então é ESTA linha que
                         // filtra o acelerômetro (44 Hz). MPU6500: só o giroscópio.

  Serial.println("Deixe o motor na posicao inicial/de uso e nao o movimente durante a calibracao...");
  delay(2000);
  mpu.calibrateAccelGyro(&calib);   // ⚖ paridade 4 — comente esta linha no Wokwi
  mpu.init(calib, 0x68);

  Serial.println("MPU iniciado");
}

void loop() {
  AccelData accel;
  mpu.update();
  mpu.getAccel(&accel);
  Serial.printf("ax=%.3f  ay=%.3f  az=%.3f\r\n", accel.accelX, accel.accelY, accel.accelZ);
  delay(200);
}
```

**Confira antes de seguir:** com o motor parado e nivelado, `az` fica perto de
`1.0` e `ax`/`ay` perto de `0`. Se `az` der `9.8`, o valor está em m/s² e não em
`g` — o modelo foi treinado em `g` e vai errar tudo. Se as inclinações depois
saírem trocadas, volte aqui: é montagem do sensor, não é o modelo.

---

## Etapa 2 — Amostrar a 100 Hz e fechar a janela

Objetivo: uma mensagem por segundo dizendo que a janela fechou. **⚖ paridade 5.**

O `delay(200)` do loop anterior sai: quem marca o tempo agora é o `millis()`.

```cpp
/* ---- Amostragem: 100 Hz, janela de 1 s ---- */
const int FS_HZ          = 100;
const int AMOSTRA_MS     = 1000 / FS_HZ;      // 10 ms
const int TAMANHO_JANELA = FS_HZ;             // 100 amostras = 1 s

float ax_buf[TAMANHO_JANELA];
float ay_buf[TAMANHO_JANELA];
float az_buf[TAMANHO_JANELA];
float mag_buf[TAMANHO_JANELA];

int indice = 0;
uint32_t tempoAnterior = 0;

void loop() {
  if (millis() - tempoAnterior >= AMOSTRA_MS) {
    // Avança em passos fixos de AMOSTRA_MS (e não "= millis()"): assim o atraso
    // de um ciclo não empurra o próximo e a taxa não escorrega abaixo de 100 Hz.
    tempoAnterior += AMOSTRA_MS;
    // Se ainda estamos mais de uma amostra atrasados, não adianta amostrar em
    // rajada para recuperar: as amostras sairiam sem espaçamento real.
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

**Confira antes de seguir:** uma linha `janela fechada` por segundo, sem atrasos
acumulando. Se sair a cada 1,3 s, a taxa real não é 100 Hz e as features de
vibração saem menores do que as do treino.

---

## Etapa 3 — As 8 features

Objetivo: imprimir os oito números por janela. **⚖ paridade 6 e 7.**

Copie as quatro funções **literalmente**. Não é excesso de zelo: `calcStd`
dividindo por `n - 1` em vez de `n` muda todos os `std_*` e desloca os limiares
da floresta.

```cpp
float calcMean(float arr[], int n) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += arr[i];
  return soma / n;
}

float calcStd(float arr[], int n, float media) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += (arr[i] - media) * (arr[i] - media);
  return sqrt(soma / n);         // ⚖ divide por n
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

E, dentro do `if (indice >= TAMANHO_JANELA)`, no lugar do `Serial.println`:

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

      // ⚖ paridade 8: a ORDEM é a ordem das colunas no Colab.
      // x[0] é mean_ax porque mean_ax era a primeira coluna lá.
      float features[8] = { mx, my, mz, sx, sy, sz, stdMag, p2p };

      Serial.printf("mean: %.3f %.3f %.3f | std: %.3f %.3f %.3f | mag: %.3f %.3f\r\n",
                    features[0], features[1], features[2], features[3],
                    features[4], features[5], features[6], features[7]);
      indice = 0;
```

**Confira antes de seguir:** nivelado e parado, `mean_az ≈ 1.0` e os `std_*`
perto de zero. Inclinando 25° para a frente, parte da gravidade migra para
`mean_ax` (`sen 25° ≈ 0,42`) e `mean_az` cai para perto de `0,91`. Se esses dois
números não se mexerem como o esperado, pare aqui: nenhum modelo conserta uma
feature errada.

---

## Etapa 4 — O modelo na flash

Objetivo: imprimir a classe prevista. É a etapa onde este app deixa de ser o
outro.

Dois arquivos entram em `device/src/`, gerados pelo notebook do Colab:

- `ModeloMotorScaler.hpp` — a média e o desvio de cada uma das 8 features
- `ModeloMotorRF.hpp` — as 15 árvores, escritas em `if`/`else`

> O projeto já vem com uma versão **sintética** dos dois, só para compilar de
> saída. Substitua os dois **juntos**, sempre do mesmo treino: o scaler guarda a
> escala em que os limiares da floresta foram escritos. Misturar o scaler de uma
> execução com a floresta de outra não dá erro nenhum — dá predição errada.

Abra o `ModeloMotorRF.hpp` antes de continuar. É um arquivo legível:

```cpp
int predict(float *x) {
    uint8_t votes[4] = { 0 };
    // tree #1
    if (x[5] <= 0.0557...) {
        if (x[0] <= -0.7006...) {
            votes[2] += 1;
        }
        ...
```

Cada árvore percorre no máximo três comparações e deposita **um voto**. No fim,
a classe com mais votos ganha. Não há multiplicação de matriz, não há função de
ativação, não há biblioteca de ML: um modelo de árvore vira `if`/`else` porque é
literalmente isso que ele é. Esse é o motivo de a Random Forest ser a porta de
entrada natural para inferência embarcada.

Os limiares estão na escala do scaler, não em `g` — por isso aparece
`-0.70` e não `-0.42`. É a única leitura que o scaler atrapalha, e o preço de
manter a mesma sequência do outro app embarcado da disciplina.

```cpp
/* ---- Os dois arquivos do modelo, sempre do mesmo treino ---- */
#include "ModeloMotorScaler.hpp"   // Scaler::standardize()
#include "ModeloMotorRF.hpp"       // Eloquent::ML::Port::RandomForest

Eloquent::ML::Port::RandomForest modeloRF;

/* ⚖ paridade 9: os nomes na ordem de classes_, que é ALFABÉTICA.
   O predict() devolve 0, 1, 2 ou 3; quem dá nome a esse número é este vetor.
   A ordem não é a que a gente usaria: o scikit-learn ordena as classes pelo
   nome, então anomalia fica no índice 0 e operando no 3. A seção 9 do Colab
   imprime esta linha pronta para colar — copie de lá em vez de escrever.

   Trocar duas linhas aqui não gera erro — só faz o motor inclinado para trás
   acender o LED de inclinado para a frente, para sempre. */
const char* NOMES_CLASSES[4] = { "anomalia", "inclinado_frente",
                                 "inclinado_tras", "operando" };
```

Antes da função, **o protótipo**. Este arquivo é um `.cpp`, e não um `.ino`: o
Arduino gera os protótipos sozinho nos `.ino`, o C++ não. Como a
`classificarJanela()` é chamada dentro do `loop()` e escrita depois dele, sem
esta linha o compilador reclama que ela não foi declarada. Coloque junto das
outras, logo abaixo do `NOMES_CLASSES`:

```cpp
/* ---- Protótipos ---- */
int  classificarJanela(const float features[8]);
void acionarSaida(int classe);
void apagarTodasAsSaidas();
```

As duas últimas são da etapa 5; declare-as agora e o arquivo já fica pronto
para ela.

E a função que faz o trabalho — duas linhas de inferência, o resto é impressão.
Repare que ela **devolve** a classe e não acende nada: quem decide o que fazer
com o número é o `loop()`. Separadas assim, dá para trocar a saída sem tocar na
inferência:

```cpp
int classificarJanela(const float features[8]) {
  float padronizado[8];

  uint32_t t0 = micros();
  Scaler::standardize(features, padronizado);  // (valor - media) / desvio
  int classe = modeloRF.predict(padronizado);  // a floresta decide
  uint32_t duracao = micros() - t0;

  Serial.println("--- Janela fechada ---");
  Serial.printf("  mean_ax=%.3f  mean_ay=%.3f  mean_az=%.3f\r\n",
                features[0], features[1], features[2]);
  Serial.printf("  std_ax=%.3f   std_ay=%.3f   std_az=%.3f\r\n",
                features[3], features[4], features[5]);
  Serial.printf("  std_mag=%.3f  p2p_mag=%.3f\r\n", features[6], features[7]);

  if (classe >= 0 && classe < 4) {
    Serial.printf("  PREDIÇÃO:  %d -> %s\r\n", classe, NOMES_CLASSES[classe]);
  } else {
    Serial.printf("  PREDIÇÃO:  %d -> indice fora da faixa\r\n", classe);
  }
  Serial.printf("  Inferencia: %lu us\r\n", duracao);
  Serial.println("----------------------");

  return classe;
}
```

No `loop()`, troque o `Serial.printf` da etapa 3 pela chamada:

```cpp
      classificarJanela(features);
```

O retorno é ignorado **por enquanto** — nesta etapa quem mostra a classe é o
`Serial.printf` de dentro da função. Na etapa 5 ele passa a ser guardado e
entregue à saída.

**A ordem importa e é sempre esta:** ler as 8 features → `Scaler::standardize()` →
`predict()`. Chamar `predict()` com os valores crus compila, roda, e responde
qualquer coisa.

**Confira antes de seguir:** uma classe por segundo no Serial, e o tempo de
inferência na casa de poucos microssegundos — são 15 árvores de três
comparações cada, mais oito divisões do scaler. Guarde esse número: a versão
com API levava cerca de **um segundo** para responder a mesma pergunta, e
precisava de Wi-Fi, broker, n8n e um servidor no ar.

---

## Etapa 5 — As saídas: 3 LEDs e um buzzer

Objetivo: cada classe acende a sua saída. Os mesmos pinos da versão com API.

| GPIO | Componente | Classe | Índice |
|---|---|---|---|
| 19 | buzzer | `anomalia` | 0 |
| 21 | LED amarelo | `inclinado_frente` | 1 |
| 18 | LED vermelho | `inclinado_tras` | 2 |
| 4 | LED azul | `operando` | 3 |

```cpp
#define LED_AZUL      4   // operando
#define LED_AMARELO  21   // inclinado_frente
#define LED_VERMELHO 18   // inclinado_tras
#define BUZZER       19   // anomalia
```

No `setup()`, os `pinMode` e um teste de ligação — ele existe para você
conferir a fiação **antes** de depender do modelo. Se o LED amarelo não acender
aqui, o problema é o fio, não a floresta:

```cpp
  pinMode(LED_AZUL,     OUTPUT);
  pinMode(LED_AMARELO,  OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(BUZZER,       OUTPUT);

  apagarTodasAsSaidas();

  Serial.println("Testando as saidas...");
  digitalWrite(LED_AZUL,     HIGH); delay(400); digitalWrite(LED_AZUL,     LOW);
  digitalWrite(LED_AMARELO,  HIGH); delay(400); digitalWrite(LED_AMARELO,  LOW);
  digitalWrite(LED_VERMELHO, HIGH); delay(400); digitalWrite(LED_VERMELHO, LOW);
  tone(BUZZER, 500, 250); noTone(BUZZER);   // buzzer passivo: precisa de frequencia

  Serial.println("Sistema pronto. Uma janela por segundo, sem rede nenhuma.");
  Serial.println("  Saidas (uma por classe):");
  Serial.printf("    GPIO %2d  LED azul     = operando\r\n",         LED_AZUL);
  Serial.printf("    GPIO %2d  LED amarelo  = inclinado_frente\r\n", LED_AMARELO);
  Serial.printf("    GPIO %2d  LED vermelho = inclinado_tras\r\n",   LED_VERMELHO);
  Serial.printf("    GPIO %2d  BUZZER       = anomalia\r\n\r\n",     BUZZER);
```

E as duas funções que acendem. Elas são o **outro lado** da separação feita na
etapa 4: a `classificarJanela()` responde qual é a classe, a `acionarSaida()`
decide o que fazer com ela.

```cpp
void acionarSaida(int classe) {
  apagarTodasAsSaidas();

  switch (classe) {
    case 0: tone(BUZZER, 500, 250);           break;   // anomalia: bipe de 250 ms
    case 1: digitalWrite(LED_AMARELO,  HIGH); break;   // inclinado_frente
    case 2: digitalWrite(LED_VERMELHO, HIGH); break;   // inclinado_tras
    case 3: digitalWrite(LED_AZUL,     HIGH); break;   // operando
    default: break;                                    // tudo apagado
  }
}

void apagarTodasAsSaidas() {
  digitalWrite(LED_AZUL,     LOW);
  digitalWrite(LED_AMARELO,  LOW);
  digitalWrite(LED_VERMELHO, LOW);
  digitalWrite(BUZZER,       LOW);
}
```

E, no `loop()`, a chamada da etapa 4 passa a guardar o retorno e entregá-lo à
saída — as duas linhas que fecham o firmware:

```cpp
      int classe = classificarJanela(features);
      acionarSaida(classe);
```

### Duas coisas a reparar aqui

**A saída fica ligada até a próxima janela.** Não há temporização nenhuma: a
saída é a memória do dispositivo, exatamente como era na versão com API. A
diferença é que lá o índice chegava do outro lado do mundo, e aqui ele nasce
dentro da placa.

**O estado "tudo apagado" praticamente sumiu.** Na versão com API, tudo apagado
queria dizer "a nuvem não respondeu" — e isso acontecia sempre que o Wi-Fi, o
broker, o n8n ou a API caíssem. Aqui só existe no primeiro segundo, antes de
fechar a primeira janela. Depois disso há sempre uma saída acesa, e é ela que
mostra que o dispositivo está vivo, já que não há mais tráfego de rede para
observar. O LED onboard, que indicava conexão com o broker, também não faz mais
sentido e saiu.

**Confira:** ao ligar, os três LEDs acendem em sequência e o buzzer dá um bipe.
Depois, uma saída acesa por segundo, trocando conforme você move o motor.

---

## Checklist de paridade

Antes de acreditar numa predição, confira contra o app de coleta, que gerou o
dataset:

- [ ] `#define MPU_TYPE` — o mesmo chip da coleta
- [ ] `setAccelRange(8)`
- [ ] `setAccelLPF(41)` **e** `setGyroLPF(42)`, nesta ordem
- [ ] `calibrateAccelGyro()` presente, feita na posição de uso e motor parado
- [ ] `FS_HZ = 100`, `TAMANHO_JANELA = 100`
- [ ] `tempoAnterior += AMOSTRA_MS` (passo fixo)
- [ ] `calcStd` dividindo por `n`
- [ ] valores em `g` (nivelado: `mean_az ≈ 1.0`)
- [ ] a ordem de `features[8]` igual à ordem de `FEATURES` no Colab
- [ ] `NOMES_CLASSES[4]` igual à linha que o Colab imprimiu
- [ ] os dois `.hpp` são do **mesmo** treino
- [ ] montagem física do sensor na mesma orientação da coleta

O último item não está no código e é o que mais quebra na prática: girar o
sensor 90° no gabarito troca `mean_ax` por `mean_ay`, e o modelo passa a
confundir `inclinado_frente` com `inclinado_tras`. Se as inclinações saírem
trocadas, suspeite da montagem antes de suspeitar do modelo.

## Teste por partes

Sem rede, o caminho é curto — e cada degrau isola uma peça:

1. **Os dois `.hpp` são do mesmo treino?** É a única coisa que não dá erro de
   compilação quando está errada. Na dúvida, rode o Colab de novo e traga os
   dois juntos.
2. **A fiação:** os três LEDs em sequência e o bipe, no `setup()`.
3. **As features:** motor nivelado e parado → `mean_az ≈ 1.0`, `std_*` baixos.
   Inclinado 25° → `mean_ax ≈ ±0,42`.
4. **A decisão:** o Serial mostra o índice e o nome. Se o número está certo e o
   LED errado acende, o problema é a etapa 5; se o número está errado, é o
   modelo ou a paridade.

No Wokwi dá para reproduzir a **inclinação**, com o controle de aceleração do
MPU6050 em X, Y e Z: ajuste até o Serial mostrar `mean_az ≈ 0,91` e
`mean_ax ≈ ±0,42`. O que **não** sai é a `anomalia`, que é vibração — com o
controle parado as 100 amostras da janela ficam idênticas, e `std_*` e
`p2p_mag` dão zero. Em compensação, aqui não é preciso configurar Wi-Fi
nenhum — o firmware roda inteiro sem rede.
