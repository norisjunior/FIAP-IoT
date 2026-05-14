#include <FlixPeriph.h>
// Descomente se estiver usando MPU6500 vendido como MPU6050 generico.
#include <MPU6500.h>
#include <Wire.h>
#include <math.h>

#define SDAPIN    22
#define SCLPIN    23
#define MOTOR_INA 25
#define MOTOR_INB 26
#define BTN_A     32  // gira para frente
#define BTN_B     33  // gira para tras
#define LED_PIN   27  // acende enquanto qualquer botao estiver pressionado

// Wiring MPU:
// VCC -> 3V3
// GND -> GND
// SDA -> GPIO22
// SCL -> GPIO23

// Troque para MPU6050 se estiver usando o sensor original.
MPU6500 mpu(Wire);
//MPU6050 mpu(Wire);

// Arrays para armazenar a janela de 100 leituras
const int TAMANHO_JANELA = 100;
float ax[TAMANHO_JANELA];
float ay[TAMANHO_JANELA];
float az[TAMANHO_JANELA];

void setup() {
  Serial.begin(115200);

  Wire.begin(SDAPIN, SCLPIN);

  if (!mpu.begin()) {
    Serial.println("Erro: MPU nao encontrado");
    while (1);
  }

  // Sensibilidade do acelerometro:
  // ACCEL_RANGE_2G  = mais sensivel
  // ACCEL_RANGE_4G
  // ACCEL_RANGE_8G
  // ACCEL_RANGE_16G = mede acelerações maiores
  mpu.setAccelRange(IMUInterface::ACCEL_RANGE_8G);

  Serial.println("MPU iniciado");

  pinMode(MOTOR_INA, OUTPUT);
  pinMode(MOTOR_INB, OUTPUT);
  pinMode(LED_PIN,   OUTPUT);
  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);

  digitalWrite(MOTOR_INA, LOW);
  digitalWrite(MOTOR_INB, LOW);
  digitalWrite(LED_PIN,   LOW);
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
  // Motor: lê botões e aciona a cada iteração
  bool btnA = digitalRead(BTN_A) == LOW;
  bool btnB = digitalRead(BTN_B) == LOW;

  if (btnA) {
    digitalWrite(MOTOR_INA, HIGH);
    digitalWrite(MOTOR_INB, LOW);
  } else if (btnB) {
    digitalWrite(MOTOR_INA, LOW);
    digitalWrite(MOTOR_INB, HIGH);
  } else {
    digitalWrite(MOTOR_INA, LOW);
    digitalWrite(MOTOR_INB, LOW);
  }
  digitalWrite(LED_PIN, btnA || btnB);

  // IMU: coleta a cada AMOSTRA_MS
  if (millis() - tempoAnterior >= AMOSTRA_MS) {
    tempoAnterior = millis();

    mpu.read();
    mpu.getAccel(ax[indice], ay[indice], az[indice]);
    indice++;

    if (indice >= TAMANHO_JANELA) {
      const char* estado = btnA ? "frente" : (btnB ? "tras" : "parado");
      Serial.printf("--- Features [motor: %s] ---\n", estado);
      Serial.printf("mean_ax=%.3f  mean_ay=%.3f  mean_az=%.3f\r\n", calcMean(ax, TAMANHO_JANELA), calcMean(ay, TAMANHO_JANELA), calcMean(az, TAMANHO_JANELA));
      Serial.printf("std_ax=%.3f   std_ay=%.3f   std_az=%.3f\r\n",  calcStd(ax,  TAMANHO_JANELA), calcStd(ay,  TAMANHO_JANELA), calcStd(az,  TAMANHO_JANELA));
      Serial.printf("rms_ax=%.3f   rms_ay=%.3f   rms_az=%.3f\r\n",  calcRMS(ax,  TAMANHO_JANELA), calcRMS(ay,  TAMANHO_JANELA), calcRMS(az,  TAMANHO_JANELA));
      Serial.printf("rms_mag=%.3f\r\n", calcRMSMagnitude(ax, ay, az, TAMANHO_JANELA));
      indice = 0;
    }
  }
}