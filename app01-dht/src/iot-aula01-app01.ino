#include <DHT.h>

//Definições - GPIO
#define LEDPIN_VERMELHO 25
#define LEDPIN_VERDE 27

// Estrutura DHT
#define DHTPIN 26
#define DHTMODE DHT22
DHT dht(DHTPIN, DHTMODE);

uint32_t INTERVALO_COLETA = 2100;
uint32_t tempoAgora = 0;

void setup() {
  //Inicializar sensores
  pinMode(LEDPIN_VERDE, OUTPUT);
  pinMode(LEDPIN_VERMELHO, OUTPUT);
  dht.begin();

  Serial.begin(115200);
}

void loop() {
  if (millis() >= tempoAgora + INTERVALO_COLETA) {
    tempoAgora = millis();

    bool acenderLed = false;

    float temp = dht.readTemperature();
    float umid = dht.readHumidity();

    if (isnan(temp) || isnan(umid)) {
      Serial.println("Falha ao ler do sensor DHT!");
      delay(2000);
      return;
    }

    float heatIndex = dht.computeHeatIndex(temp, umid, false);

    Serial.printf("\nT: %.2f | U: %.2f | IC: %.2f\r\n", temp, umid, heatIndex);

    if ( (heatIndex > 25) ) {
      Serial.println("[WARN] Condições ambientais inadequadas");
      acenderLed = true;
    } else {
      Serial.println("[OK] Condições ambientais dentro da normalidade");
      acenderLed = false;
    }
    digitalWrite(LEDPIN_VERMELHO, acenderLed ? HIGH : LOW);
    //ou
    //digitalWrite(LEDPIN_VERMELHO, acenderLed);
  }
  
}

