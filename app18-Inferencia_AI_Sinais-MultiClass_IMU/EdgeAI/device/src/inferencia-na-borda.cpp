/* inferencia-na-borda — a mesma decisão, agora dentro do ESP32.

   A janela é a mesma da coleta: 100 amostras a 100 Hz, as mesmas 8 features,
   as mesmas 4 classes, as mesmas 4 saídas. Muda só QUEM decide.

   Na versão com API, a janela virava JSON, saía pelo MQTT, atravessava n8n e
   FastAPI, e a classe voltava por outro tópico. Aqui a janela não sai da placa:
   ela é padronizada pelo Scaler e entregue à Random Forest que mora na flash.

       Scaler::standardize(bruto, padronizado);     // mesma conta do treino
       int classe = modeloRF.predict(padronizado);  // a floresta, em if/else

   Repare no que sumiu: WiFi.h, PubSubClient, ArduinoJson, os dois tópicos, o
   IP do broker, a reconexão, o callback. E repare no que NÃO sumiu: nada do
   caminho do dado. As funções de feature são as mesmas, letra por letra, e é
   por isso que o modelo treinado com o dataset da coleta funciona aqui — ele
   recebe exatamente os números que viu no treino.

       operando          LED azul      (4)
       inclinado_frente  LED amarelo  (21)
       inclinado_tras    LED vermelho (18)
       anomalia          buzzer       (19)

   Não há mais o estado "tudo apagado = a nuvem não respondeu": aqui sempre há
   resposta. Depois da primeira janela, uma saída está sempre acesa — e é essa
   saída que mostra que o dispositivo está vivo, já que não há mais tráfego de
   rede para observar.

   O LED onboard também sai: ele indicava conexão com o broker, e não há broker.
*/
/*
PARA USAR NO WOKWI:
- Ajustar #define MPU_TYPE:
  - #define MPU_TYPE MPU6050
- Remover/comentar a linha `mpu.calibrateAccelGyro(&calib);` (trava no Wokwi, FIFO ausente)
- As classes de inclinação não têm equivalente fiel no simulador (não há como
  inclinar o MPU6050 do Wokwi); serve para ver a inferência rodando e o tempo
  que ela leva.
- Não é preciso configurar Wi-Fi: este firmware não usa rede nenhuma.
*/

#include <Arduino.h>
#include "FastIMU.h"
#include <Wire.h>
#include <math.h>

/* ---- O modelo, em dois arquivos gerados pelo Colab ----
   Os dois SEMPRE do mesmo treino: o scaler guarda a média e o desvio de cada
   feature, e os limiares da floresta estão nessa escala. Misturar o scaler de
   uma execução com a floresta de outra não dá erro de compilação — dá predição
   errada, em silêncio. */
#include "ModeloMotorScaler.hpp"   // Scaler::standardize()
#include "ModeloMotorRF.hpp"       // Eloquent::ML::Port::RandomForest

Eloquent::ML::Port::RandomForest modeloRF;

/* ---- Pinos ---- */
#define SDA_PIN      22
#define SCL_PIN      23
/* Uma saída por classe, como na versão com API. */
#define LED_AZUL      4   // operando
#define LED_AMARELO  21   // inclinado_frente
#define LED_VERMELHO 18   // inclinado_tras
#define BUZZER       19   // anomalia

/* ---- Sensor (MPU6050 ou MPU6500) ---- */
#define MPU_TYPE MPU6500
MPU_TYPE mpu;

calData calib = { 0 };

/* ---- Amostragem: 100 Hz, janela de 1 s (mesmo padrão da coleta) ---- */
const int FS_HZ          = 100;
const int AMOSTRA_MS     = 1000 / FS_HZ;      // 10 ms
const int TAMANHO_JANELA = FS_HZ;             // 100 amostras = 1 s

float ax_buf[TAMANHO_JANELA];
float ay_buf[TAMANHO_JANELA];
float az_buf[TAMANHO_JANELA];
float mag_buf[TAMANHO_JANELA];

int indice = 0;
uint32_t tempoAnterior = 0;

/* ---- Os nomes das classes, NA ORDEM DO scikit-learn ----
   O predict() devolve um número: 0, 1, 2 ou 3. Quem dá nome a esse número é
   este vetor, e a ordem é a de classes_, que o scikit-learn devolve sempre em
   ordem ALFABÉTICA — não na ordem em que a gente pensa nas classes. Por isso
   anomalia é o índice 0. O Colab imprime esta linha pronta na seção 10.

   Trocar duas linhas aqui não gera erro nenhum — só faz o motor inclinado
   acender o LED errado para sempre. */
const char* NOMES_CLASSES[4] = { "anomalia", "inclinado_frente",
                                 "inclinado_tras", "operando" };

/* ---- Protótipos ---- */
int  classificarJanela(const float features[8]);
void acionarSaida(int classe);
void apagarTodasAsSaidas();

/* =========================== Features ===========================
   As 8 que o modelo recebe: mean_* (orientação), std_* e std_mag (vibração)
   e p2p_mag (pior caso da janela). Idênticas às da coleta — é esse "idênticas"
   que faz o modelo valer aqui. */
float calcMean(float arr[], int n) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += arr[i];
  return soma / n;
}

float calcStd(float arr[], int n, float media) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += (arr[i] - media) * (arr[i] - media);
  return sqrt(soma / n);
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

/* ============================== SETUP ============================== */
void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);
  if (mpu.init(calib, 0x68) != 0) {
    Serial.println("Erro: MPU não encontrado");
    while (1);
  }
  mpu.setAccelRange(8);
  // Anti-aliasing p/ amostragem a 100 Hz (Nyquist 50 Hz): o filtro interno do
  // acelerômetro precisa cortar abaixo de 50 Hz. São duas chamadas porque
  // nenhuma delas sozinha cobre os dois chips:
  mpu.setAccelLPF(41);   // MPU6500: escreve ACCEL_CONFIG2 -> 41 Hz.
                         // MPU6050: o FastIMU não implementa, devolve -1 sem fazer nada.
  mpu.setGyroLPF(42);    // MPU6050: o DLPF é COMPARTILHADO, então é ESTA linha que
                         // filtra o acelerômetro (44 Hz). MPU6500: só o giroscópio.

  Serial.println("Deixe o motor na posicao inicial/de uso e nao o movimente durante a calibracao...");
  delay(2000);
  mpu.calibrateAccelGyro(&calib);   // habilite no ESP32 físico; trava no Wokwi (FIFO ausente)
  mpu.init(calib, 0x68);

  Serial.println("MPU iniciado");

  pinMode(LED_AZUL,     OUTPUT);
  pinMode(LED_AMARELO,  OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(BUZZER,       OUTPUT);

  apagarTodasAsSaidas();

  // Teste de ligação: acende uma saída por vez, para você conferir se cada
  // componente está no pino certo ANTES de depender do modelo. Se o LED
  // amarelo não acender aqui, o problema é o fio — não a floresta.
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
}

/* ============================== LOOP =============================== */
void loop() {
  // --- Coleta IMU a 100 Hz ---
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

      // A ORDEM deste vetor é a ordem das colunas no treino. É o contrato do
      // modelo: x[0] é mean_ax porque mean_ax era a primeira coluna no Colab.
      float features[8] = { mx, my, mz, sx, sy, sz, stdMag, p2p };

      // Uma função responde QUAL é a classe; a outra decide o que fazer com
      // ela. Separadas, dá para trocar a saída sem tocar na inferência.
      int classe = classificarJanela(features);
      acionarSaida(classe);

      indice = 0;
    }
  }
}

/* ---- A inferência: duas linhas, e o resto é impressão ----
   Onde antes havia um publish, uma viagem pela rede e um callback, agora há
   uma padronização e uma varredura de 15 árvores. O micros() está aqui para
   você mostrar o número em aula: a nuvem respondia em ~1 s; isto responde em
   microssegundos, e sem Wi-Fi.

   Esta função só RESPONDE: devolve o índice da classe e não mexe em pino
   nenhum. Quem acende é a acionarSaida(), chamada pelo loop. */
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

/* ---- Acende SÓ a saída da classe prevista ----
   A saída fica ligada até a próxima janela fechar, um segundo depois: ela é a
   memória do dispositivo, exatamente como era na versão com API. A diferença é
   que lá o índice chegava do outro lado do mundo; aqui ele nasce aqui dentro. */
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

/* ---- Apaga as quatro saídas ----
   Chamada antes de acender a nova, para nunca ficarem duas ligadas. */
void apagarTodasAsSaidas() {
  digitalWrite(LED_AZUL,     LOW);
  digitalWrite(LED_AMARELO,  LOW);
  digitalWrite(LED_VERMELHO, LOW);
  digitalWrite(BUZZER,       LOW);
}
