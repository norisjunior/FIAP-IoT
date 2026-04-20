#include <Arduino.h>

#define MIC_PIN 35

int centro = 0;

// === Janela de análise ===
const int JANELA_MS = 50;
unsigned long inicioJanela = 0;
long somaQuadrados = 0;
int contagem = 0;

void calibrarCentro() {
  long soma = 0;
  const int amostras = 500;
  for (int i = 0; i < amostras; i++) {
    soma += analogRead(MIC_PIN);
    delay(2);
  }
  centro = soma / amostras;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Calibrando...");
  delay(1000);
  calibrarCentro();
  Serial.printf("Centro calibrado: %d\n", centro);
  inicioJanela = millis();
}

void loop() {
  int raw = analogRead(MIC_PIN);
  int amplitude = abs(raw - centro);

  // Acumula o quadrado da amplitude
  somaQuadrados += (long)amplitude * amplitude;
  contagem++;

  if (millis() - inicioJanela >= JANELA_MS) {
    int rms = sqrt(somaQuadrados / contagem);

    Serial.printf(">amplitude:%d\n", amplitude);
    Serial.printf(">rms:%d\n", rms);

    somaQuadrados = 0;
    contagem = 0;
    inicioJanela = millis();
  }

  delayMicroseconds(500);
}