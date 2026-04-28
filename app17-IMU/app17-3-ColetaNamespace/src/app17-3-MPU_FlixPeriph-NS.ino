#include "AICSSAccelMPU.hpp"

#define SCLPIN 27
#define SDAPIN 26
#define LED    25

int contador = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);

  AICSSAccel::inicializar(SDAPIN, SCLPIN);

  delay(1000);
  Serial.println("Dispositivo IoT ESP32 detector de vibração");
  Serial.println("id,x,y,z,status");
}

void loop() {
  contador++;

  AICSSAccel::ACCEL accel = AICSSAccel::lerAceleracao();

  delay(10);

  String movimento;
  if ((abs(accel.x) >= 1.5) || (abs(accel.y) >= 1.5) || (abs(accel.z) > 11)) {
    digitalWrite(LED, HIGH);
    movimento = "SEVERA";
  } else {
    digitalWrite(LED, LOW);
    movimento = "NORMAL";
  }

  Serial.printf("%d,%.2f,%.2f,%.2f,%s", contador, accel.x, accel.y, accel.z, movimento.c_str());
  Serial.println("");

  delay(100);
}
