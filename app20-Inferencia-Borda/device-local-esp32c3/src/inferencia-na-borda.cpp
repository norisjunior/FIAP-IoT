/* inferencia-na-borda (ESP32-C3 Super Mini, com motor de vibração) — o ESP32 classifica a janela com a Random Forest embarcada */

#include <Arduino.h>
#include "FastIMU.h"
#include <Wire.h>
#include <math.h>

/* ---- Modelo (gerado pelo Colab) ---- */
#include "ModeloMotorScaler.hpp"   // Scaler::standardize()
#include "ModeloMotorRF.hpp"       // Eloquent::ML::Port::RandomForest

Eloquent::ML::Port::RandomForest modeloRF;

/* ---- Pinos (ESP32-C3 Super Mini) ---- */
#define SDA_PIN      20
#define SCL_PIN      21
#define LED_AZUL      1   // operando
#define LED_VERDE     3   // inclinado_frente
#define LED_LARANJA   4   // inclinado_tras
#define LED_VERMELHO 10   // anomalia
#define BUZZER        7   // anomalia (buzzer passivo)
#define LED_RGB       8   // LED RGB da placa: a mesma cor do LED aceso
#define MOTOR_PIN     0   // motor de vibração (pino IN do módulo)

/* ---- Cores do LED RGB (R, G, B) ---- */
#define RGB_AZUL      0,  0, 40   // operando
#define RGB_VERDE     0, 40,  0   // inclinado_frente
#define RGB_LARANJA  40, 10,  0   // inclinado_tras
#define RGB_VERMELHO 40,  0,  0   // anomalia
#define RGB_APAGADO   0,  0,  0

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
void apagarLeds();
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
  delay(1000);   // USB nativa: tempo para o Monitor Serial conectar

  // Saídas
  pinMode(LED_AZUL,     OUTPUT);
  pinMode(LED_VERDE,    OUTPUT);
  pinMode(LED_LARANJA,  OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(BUZZER,       OUTPUT);
  apagarTodasAsSaidas();

  // Motor desligado: a calibração precisa da placa parada
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);

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

  // Teste das saídas: cada LED com a mesma cor no RGB
  Serial.println("Testando as saidas...");
  digitalWrite(LED_AZUL,     HIGH); neopixelWrite(LED_RGB, RGB_AZUL);     delay(400);
  digitalWrite(LED_AZUL,     LOW);
  digitalWrite(LED_VERDE,    HIGH); neopixelWrite(LED_RGB, RGB_VERDE);    delay(400);
  digitalWrite(LED_VERDE,    LOW);
  digitalWrite(LED_LARANJA,  HIGH); neopixelWrite(LED_RGB, RGB_LARANJA);  delay(400);
  digitalWrite(LED_LARANJA,  LOW);
  tone(BUZZER, 500, 250);   // bipe junto com o vermelho
  digitalWrite(LED_VERMELHO, HIGH); neopixelWrite(LED_RGB, RGB_VERMELHO); delay(400);
  apagarTodasAsSaidas();

  // Motor vibrando o tempo todo, como na coleta
  digitalWrite(MOTOR_PIN, HIGH);

  Serial.println("Sistema pronto. Uma janela por segundo, sem rede nenhuma.");
  Serial.println("  Saidas (uma por classe, o RGB na mesma cor):");
  Serial.printf("    GPIO %2d  LED azul     = operando\r\n",         LED_AZUL);
  Serial.printf("    GPIO %2d  LED verde    = inclinado_frente\r\n", LED_VERDE);
  Serial.printf("    GPIO %2d  LED laranja  = inclinado_tras\r\n",   LED_LARANJA);
  Serial.printf("    GPIO %2d  LED vermelho = anomalia\r\n",         LED_VERMELHO);
  Serial.printf("    GPIO %2d  buzzer       = anomalia (bipe)\r\n\r\n", BUZZER);
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

/* ---- Acende a saída da classe, e o RGB na mesma cor ---- */
void acionarSaida(int classe) {
  // Apaga só os LEDs: o RGB recebe uma cor só (duas seguidas, ele fica na primeira)
  apagarLeds();

  switch (classe) {
    case 0:                                                                       // anomalia
      digitalWrite(LED_VERMELHO, HIGH); neopixelWrite(LED_RGB, RGB_VERMELHO);
      tone(BUZZER, 500, 250);
      break;
    case 1: digitalWrite(LED_VERDE,   HIGH); neopixelWrite(LED_RGB, RGB_VERDE);   break;   // inclinado_frente
    case 2: digitalWrite(LED_LARANJA, HIGH); neopixelWrite(LED_RGB, RGB_LARANJA); break;   // inclinado_tras
    case 3: digitalWrite(LED_AZUL,    HIGH); neopixelWrite(LED_RGB, RGB_AZUL);    break;   // operando
    default: neopixelWrite(LED_RGB, RGB_APAGADO);                                 break;   // tudo apagado
  }
}

/* ---- Apaga os LEDs externos (o RGB fica como está) ---- */
void apagarLeds() {
  digitalWrite(LED_AZUL,     LOW);
  digitalWrite(LED_VERDE,    LOW);
  digitalWrite(LED_LARANJA,  LOW);
  digitalWrite(LED_VERMELHO, LOW);
  digitalWrite(BUZZER,       LOW);
}

/* ---- Apaga os LEDs e o RGB ---- */
void apagarTodasAsSaidas() {
  apagarLeds();
  neopixelWrite(LED_RGB, RGB_APAGADO);
}
