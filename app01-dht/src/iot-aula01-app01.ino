#include <DHT.h>

//Definições - GPIO
#define LEDPIN_VERMELHO 25
#define LEDPIN_VERDE 27

// Estrutura DHT
#define DHTPIN 26
#define DHTMODE DHT22
DHT dht(DHTPIN, DHTMODE);

void piscaLED() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(LEDPIN_VERDE, HIGH);
    delay(1000);
    digitalWrite(LEDPIN_VERDE, LOW);
    delay(1000);
  }
}

void setup() {
  //Inicializar sensores
  pinMode(LEDPIN_VERDE, OUTPUT);
  pinMode(LEDPIN_VERMELHO, OUTPUT);
  dht.begin();

  Serial.begin(115200);
}

void loop() {
  bool acenderLed = false;

  float temp = dht.readTemperature();
  float umid = dht.readHumidity();

  if (isnan(temp) || isnan(umid)) {
    Serial.println("Falha ao ler do sensor DHT!");
    delay(2000);
    return;
  }

  float heatIndex = dht.computeHeatIndex(temp, umid, false);

  Serial.print("\nTemperatura: "); Serial.print(temp);
  Serial.print(" °C, Umidade: "); Serial.print(umid);
  Serial.print(" %, Índice de calor: "); Serial.print(heatIndex); Serial.println(" °C");

  if ( (heatIndex > 25) ) {
    Serial.println("Condições ambientais inadequadas");
    acenderLed = true;
  } else {
    Serial.println("Condições ambientais dentro da normalidade");
    acenderLed = false;
  }
  digitalWrite(LEDPIN_VERMELHO, acenderLed ? HIGH : LOW);
  //ou
  //digitalWrite(LEDPIN_VERMELHO, acenderLed);

  delay(2000); // Aguarda 2 segundos antes da próxima leitura (sensor DHT)

  // Pisca o LED para indicar que começará a próxima leitura
  piscaLED();
  delay(1000);
}

