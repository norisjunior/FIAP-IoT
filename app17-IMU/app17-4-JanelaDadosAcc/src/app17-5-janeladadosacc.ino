#include <FlixPeriph.h>
// Descomente se estiver usando MPU6500 vendido como MPU6050 generico.
#include <MPU6500.h>
#include <Wire.h>

#define SDAPIN 22
#define SCLPIN 23

// Wiring:
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
}

// Taxa de amostragem: 1000 / AMOSTRA_MS = Hz
const int AMOSTRA_MS = 10; // 10ms = 100Hz

int indice = 0;
uint32_t tempoAnterior = 0;

void loop() {
  if (millis() - tempoAnterior >= AMOSTRA_MS) {
    tempoAnterior = millis();

    mpu.read();
    mpu.getAccel(ax[indice], ay[indice], az[indice]);
    indice++;

    if (indice >= TAMANHO_JANELA) {
      Serial.println("Janela coletada:");
      for (int i = 0; i < TAMANHO_JANELA; i++) {
        Serial.print(ax[i]); Serial.print(", ");
        Serial.print(ay[i]); Serial.print(", ");
        Serial.println(az[i]);
      }
      indice = 0;
    }
  }
}