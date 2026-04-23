/*
 * Calibração do Sensor Capacitivo de Umidade do Solo
 *
 * INSTRUÇÕES:
 *   1. Segure o sensor NO AR (completamente seco) → anote o valor MAX
 *   2. Mergulhe o sensor em ÁGUA (completamente molhado) → anote o valor MIN
 *   3. Use esses valores no seu modelo de ML como limites de normalização
 *
 * Pino: 34 (mesmo do projeto principal)
 * Baud: 115200
 */

#include <Arduino.h>

#define SENSOR_PIN 34
#define AMOSTRAS   10       // media de leituras para estabilizar
#define INTERVALO  500      // ms entre leituras

int valorMin = 4095;
int valorMax = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=========================================");
    Serial.println("  CALIBRAÇÃO - Sensor Capacitivo Solo");
    Serial.println("=========================================");
    Serial.println("  SECO (ar)  → valor ALTO  = MAX");
    Serial.println("  MOLHADO    → valor BAIXO = MIN");
    Serial.println("  Pressione 'r' no Serial para resetar");
    Serial.println("=========================================\n");
}

int lerMedia() {
    long soma = 0;
    for (int i = 0; i < AMOSTRAS; i++) {
        soma += analogRead(SENSOR_PIN);
        delay(5);
    }
    return soma / AMOSTRAS;
}

void loop() {
    if (Serial.available()) {
        char c = Serial.read();
        if (c == 'r' || c == 'R') {
            valorMin = 4095;
            valorMax = 0;
            Serial.println("\n[RESET] Min/Max zerados.\n");
        }
    }

    int leitura = lerMedia();

    if (leitura < valorMin) valorMin = leitura;
    if (leitura > valorMax) valorMax = leitura;

    // Porcentagem estimada (0% = seco, 100% = molhado)
    float pct = 0.0;
    if (valorMax > valorMin) {
        pct = 100.0 * (float)(valorMax - leitura) / (float)(valorMax - valorMin);
        pct = constrain(pct, 0.0, 100.0);
    }

    Serial.print("Leitura: ");
    Serial.print(leitura);
    Serial.print("  |  MIN: ");
    Serial.print(valorMin);
    Serial.print("  |  MAX: ");
    Serial.print(valorMax);
    Serial.print("  |  Umidade estimada: ");
    Serial.print(pct, 1);
    Serial.println("%");

    delay(INTERVALO);
}
