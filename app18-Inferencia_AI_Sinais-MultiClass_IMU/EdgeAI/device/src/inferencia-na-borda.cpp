/* inferencia-na-borda — o ESP32 classifica a janela com a Random Forest embarcada */

#include <Arduino.h>
#include "FastIMU.h"
#include <Wire.h>
#include <math.h>

/* ---- Modelo (gerado pelo Colab) ---- */
#include "ModeloMotorScaler.hpp"   // Scaler::standardize()
#include "ModeloMotorRF.hpp"       // Eloquent::ML::Port::RandomForest

Eloquent::ML::Port::RandomForest modeloRF;

/* ---- Pinos ---- */
#define SDA_PIN      22
#define SCL_PIN      23
#define LED_AZUL      4   // operando
#define LED_AMARELO  21   // inclinado_frente
#define LED_VERMELHO 18   // inclinado_tras
#define BUZZER       19   // anomalia

/* ---- Sensor (MPU6050 ou MPU6500) ---- */
#define MPU_TYPE MPU6500 // no Wokwi: MPU6050
MPU_TYPE mpu;

calData calib = { 0 };

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

/* ---- Nomes das classes, na ordem de classes_ ---- */
const char* NOMES_CLASSES[4] = { "anomalia", "inclinado_frente",
                                 "inclinado_tras", "operando" };

/* ---- Protótipos ---- */
int  classificarJanela(const float features[8]);
void acionarSaida(int classe);
void apagarTodasAsSaidas();

/* =========================== Features =========================== */
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

  Serial.println("Deixe o motor na posicao inicial/de uso e nao o movimente durante a calibracao...");
  delay(2000);
  // Calibração (no Wokwi, comente esta linha)
  mpu.calibrateAccelGyro(&calib);
  mpu.init(calib, 0x68);
  // Fundo de escala ±8 g
  mpu.setAccelRange(8);

  Serial.println("MPU iniciado");

  pinMode(LED_AZUL,     OUTPUT);
  pinMode(LED_AMARELO,  OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(BUZZER,       OUTPUT);

  apagarTodasAsSaidas();

  // Teste das saídas
  Serial.println("Testando as saidas...");
  digitalWrite(LED_AZUL,     HIGH); delay(400); digitalWrite(LED_AZUL,     LOW);
  digitalWrite(LED_AMARELO,  HIGH); delay(400); digitalWrite(LED_AMARELO,  LOW);
  digitalWrite(LED_VERMELHO, HIGH); delay(400); digitalWrite(LED_VERMELHO, LOW);
  tone(BUZZER, 500, 250); noTone(BUZZER);   // buzzer passivo

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
    tempoAnterior += AMOSTRA_MS;
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

      // Features na ordem das colunas do treino
      float features[8] = { mx, my, mz, sx, sy, sz, stdMag, p2p };

      // Classifica e aciona a saída
      int classe = classificarJanela(features);
      acionarSaida(classe);

      indice = 0;
    }
  }
}

/* ---- Inferência ---- */
int classificarJanela(const float features[8]) {
  float padronizado[8];

  uint32_t t0 = micros();
  Scaler::standardize(features, padronizado);
  int classe = modeloRF.predict(padronizado);
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

/* ---- Acende a saída da classe ---- */
void acionarSaida(int classe) {
  apagarTodasAsSaidas();

  switch (classe) {
    case 0: tone(BUZZER, 500, 250);           break;   // anomalia
    case 1: digitalWrite(LED_AMARELO,  HIGH); break;   // inclinado_frente
    case 2: digitalWrite(LED_VERMELHO, HIGH); break;   // inclinado_tras
    case 3: digitalWrite(LED_AZUL,     HIGH); break;   // operando
    default: break;                                    // tudo apagado
  }
}

/* ---- Apaga as quatro saídas ---- */
void apagarTodasAsSaidas() {
  digitalWrite(LED_AZUL,     LOW);
  digitalWrite(LED_AMARELO,  LOW);
  digitalWrite(LED_VERMELHO, LOW);
  digitalWrite(BUZZER,       LOW);
}
