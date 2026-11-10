// app21-1-LeituraIsolada
// Caixa de fumaca: cada sensor sozinho, no plotter (Teleplot), 1 leitura por segundo.
// Pronto quando: o alcool mexe so no SGP30; o talco mexe so no SPS30.
//
// Ligacao: a mesma do app21-0 (os 3 sensores no I2C, SDA no GPIO4 e SCL no GPIO3).
// LED RGB da placa: verde = os 3 sensores iniciaram; vermelho = falta algum (rode o app21-0).

#include <Wire.h>
#include <Adafruit_SGP30.h>
#include <SensirionI2cSps30.h>
#include <Adafruit_BME280.h>

#define SDA_PIN  4
#define SCL_PIN  3
#define PINO_LED 8    // LED RGB interno (WS2812)
#define BRILHO   40   // 0 a 255

Adafruit_SGP30    sgp;  // gas: TVOC, eCO2 e dois valores crus (H2 e etanol)
SensirionI2cSps30 sps;  // particulas: massa (PM) e contagem (NC)
Adafruit_BME280   bme;  // ambiente: temperatura e umidade

bool sgpOk = false;
bool spsOk = false;
bool bmeOk = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000); // 100 kHz: o maximo que o SPS30 aceita

  // SGP30
  sgpOk = sgp.begin(&Wire);

  // SPS30: para a medicao anterior (se a placa reiniciou com ele ligado) e comeca de novo
  sps.begin(Wire, SPS30_I2C_ADDR_69);
  sps.stopMeasurement();
  delay(100);
  spsOk = (sps.startMeasurement(SPS30_OUTPUT_FORMAT_OUTPUT_FORMAT_FLOAT) == 0);

  // BME280 (SDO no G = endereco 0x76)
  bmeOk = bme.begin(0x76, &Wire);

  Serial.printf("SGP30 %s | SPS30 %s | BME280 %s\n",
                sgpOk ? "ok" : "FALHOU", spsOk ? "ok" : "FALHOU", bmeOk ? "ok" : "FALHOU");

  if (sgpOk && spsOk && bmeOk) {
    neopixelWrite(PINO_LED, 0, BRILHO, 0); // verde
  } else {
    neopixelWrite(PINO_LED, BRILHO, 0, 0); // vermelho
    Serial.println("Falta sensor: rode o app21-0 para ver qual. Os outros continuam.");
  }
}

// O SGP30 precisa ser lido a cada 1 s; o SPS30 tem leitura nova a cada 1 s.
const int AMOSTRA_MS = 1000; // 1000 ms = 1 Hz

unsigned long tempoAnterior = 0;

void loop() {
  if (millis() - tempoAnterior >= AMOSTRA_MS) {
    tempoAnterior = millis();

    // SGP30 (gas)
    // Nos primeiros ~15 s ele devolve TVOC = 0 e eCO2 = 400: esta aquecendo.
    // raw_h2 e raw_ethanol sao resistencia: CAEM quando o gas sobe.
    if (sgpOk && sgp.IAQmeasure() && sgp.IAQmeasureRaw()) {
      Serial.printf(">tvoc: %u\n>eco2: %u\n", sgp.TVOC, sgp.eCO2);
      Serial.printf(">raw_h2: %u\n>raw_ethanol: %u\n", sgp.rawH2, sgp.rawEthanol);
    }

    // SPS30 (particulas)
    // pm = massa, em ug/m3 | nc = contagem, em particulas por cm3
    float pm1_0, pm2_5, pm4_0, pm10, nc0_5, nc1_0, nc2_5, nc4_0, nc10, tamanho;
    if (spsOk && sps.readMeasurementValuesFloat(pm1_0, pm2_5, pm4_0, pm10,
                                                nc0_5, nc1_0, nc2_5, nc4_0, nc10,
                                                tamanho) == 0) {
      Serial.printf(">pm2_5: %.1f\n>pm10: %.1f\n>nc0_5: %.1f\n", pm2_5, pm10, nc0_5);
    }

    // BME280 (ambiente)
    if (bmeOk) {
      Serial.printf(">temp: %.1f\n>umid: %.1f\n", bme.readTemperature(), bme.readHumidity());
    }
  }
}
