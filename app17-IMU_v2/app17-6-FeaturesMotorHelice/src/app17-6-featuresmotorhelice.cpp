#include <Arduino.h>
#include "FastIMU.h"
#include <Wire.h>
#include <math.h>

#define SDAPIN    22
#define SCLPIN    23
#define MOTOR_INA 18
#define MOTOR_INB 19
#define BTN_A     4  // liga
#define BTN_B     21  // desliga
#define LED_PIN   2  // acende enquanto o motor estiver ligado


#define MPU_TYPE MPU6500 // troque para MPU6050 se estiver usando o sensor original
MPU_TYPE mpu;

calData calib = { 0 }; // preenchida pela calibracao no setup()

bool motorLigado = false;

// Os botoes sao lidos no loop, sem interrupcoes. Isso evita que pulsos curtos
// causados pelo ruido do motor sejam interpretados como acionamentos.
const uint32_t DEBOUNCE_MS = 60;

struct BotaoComDebounce {
  uint8_t pino;
  bool leituraAnterior;
  bool estadoEstavel;
  uint32_t ultimaMudanca;
};

BotaoComDebounce botaoLiga = {BTN_A, HIGH, HIGH, 0};
BotaoComDebounce botaoDesliga = {BTN_B, HIGH, HIGH, 0};

bool foiPressionado(BotaoComDebounce& botao, uint32_t agora) {
  const bool leituraAtual = digitalRead(botao.pino);

  if (leituraAtual != botao.leituraAnterior) {
    botao.leituraAnterior = leituraAtual;
    botao.ultimaMudanca = agora;
  }

  if ((agora - botao.ultimaMudanca >= DEBOUNCE_MS) &&
      (leituraAtual != botao.estadoEstavel)) {
    botao.estadoEstavel = leituraAtual;
    return botao.estadoEstavel == LOW;
  }

  return false;
}

void lerBotoes() {
  const uint32_t agora = millis();

  if (foiPressionado(botaoLiga, agora)) {
    motorLigado = true;
    Serial.println("Motor ligado pelo botao A");
  }

  // Se os dois botoes forem pressionados juntos, desligar tem prioridade.
  if (foiPressionado(botaoDesliga, agora)) {
    motorLigado = false;
    Serial.println("Motor desligado pelo botao B");
  }
}

// Arrays para armazenar a janela de 100 leituras
const int TAMANHO_JANELA = 100;
float ax[TAMANHO_JANELA];
float ay[TAMANHO_JANELA];
float az[TAMANHO_JANELA];

void setup() {
  Serial.begin(115200);

  // Garante que o motor fique desligado durante a inicializacao da IMU.
  pinMode(MOTOR_INA, OUTPUT);
  pinMode(MOTOR_INB, OUTPUT);
  pinMode(LED_PIN,   OUTPUT);
  digitalWrite(MOTOR_INA, LOW);
  digitalWrite(MOTOR_INB, LOW);
  digitalWrite(LED_PIN,   LOW);

  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);

  Wire.begin(SDAPIN, SCLPIN);

  if (mpu.init(calib, 0x68) != 0) {
    Serial.println("Erro: MPU nao encontrado");
    while (true) {
      delay(100);
    }
  }

  // 50 kHz deixa o I2C mais tolerante ao ruido produzido pelo motor.
  Wire.setClock(50000);

  // Sensibilidade do acelerometro, em g:
  // 2  = mais sensivel
  // 4
  // 8
  // 16 = mede acelerações maiores
  mpu.setAccelRange(8);

  Serial.println("Mantenha o sensor parado e nivelado para calibrar...");
  delay(2000);
  mpu.calibrateAccelGyro(&calib);
  mpu.init(calib, 0x68);

  Serial.println("MPU iniciado");
}

float calcMean(float arr[], int n) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += arr[i];
  return soma / n;
}

float calcStd(float arr[], int n) {
  float media = calcMean(arr, n);
  float soma = 0;
  for (int i = 0; i < n; i++) soma += (arr[i] - media) * (arr[i] - media);
  return sqrt(soma / n);
}

float calcRMS(float arr[], int n) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += arr[i] * arr[i];
  return sqrt(soma / n);
}

float calcRMSMagnitude(float ax[], float ay[], float az[], int n) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += ax[i]*ax[i] + ay[i]*ay[i] + az[i]*az[i];
  return sqrt(soma / n);
}

// Taxa de amostragem: 1000 / AMOSTRA_MS = Hz
const int AMOSTRA_MS = 10; // 10ms = 100Hz

int indice = 0;
uint32_t tempoAnterior = 0;

void loop() {
  lerBotoes();

  // Motor: estado mantido ate que o outro botao seja pressionado.
  if (motorLigado) {
    digitalWrite(MOTOR_INA, HIGH);
    digitalWrite(MOTOR_INB, LOW);
  } else {
    digitalWrite(MOTOR_INA, LOW);
    digitalWrite(MOTOR_INB, LOW);
  }
  digitalWrite(LED_PIN, motorLigado);

  // IMU: coleta a cada AMOSTRA_MS
  if (millis() - tempoAnterior >= AMOSTRA_MS) {
    tempoAnterior = millis();

    AccelData accel;
    mpu.update();
    mpu.getAccel(&accel);
    ax[indice] = accel.accelX;
    ay[indice] = accel.accelY;
    az[indice] = accel.accelZ;
    indice++;

    if (indice >= TAMANHO_JANELA) {
      const char* estado = motorLigado ? "ligado" : "parado";
      Serial.printf("--- Features [motor: %s] ---\n", estado);
      Serial.printf("mean_ax=%.3f  mean_ay=%.3f  mean_az=%.3f\r\n", calcMean(ax, TAMANHO_JANELA), calcMean(ay, TAMANHO_JANELA), calcMean(az, TAMANHO_JANELA));
      Serial.printf("std_ax=%.3f   std_ay=%.3f   std_az=%.3f\r\n",  calcStd(ax,  TAMANHO_JANELA), calcStd(ay,  TAMANHO_JANELA), calcStd(az,  TAMANHO_JANELA));
      Serial.printf("rms_ax=%.3f   rms_ay=%.3f   rms_az=%.3f\r\n",  calcRMS(ax,  TAMANHO_JANELA), calcRMS(ay,  TAMANHO_JANELA), calcRMS(az,  TAMANHO_JANELA));
      Serial.printf("rms_mag=%.3f\r\n", calcRMSMagnitude(ax, ay, az, TAMANHO_JANELA));
      indice = 0;
    }
  }
}
