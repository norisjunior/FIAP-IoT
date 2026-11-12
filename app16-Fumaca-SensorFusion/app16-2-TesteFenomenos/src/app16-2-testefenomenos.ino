// app16-2-TesteFenomenos
// Caixa de fumaca: os 3 sensores no painel (coleta.py) e o botao BOOT marcando o estimulo.
// Pronto quando: o vapor mexe no SPS30 e na umidade, o alcool so no SGP30, a fumaca nos dois e
// a chama na temperatura.
//
// Ligacao: a mesma do app16-0 (os 3 sensores no I2C, SDA no GPIO20 e SCL no GPIO21).
// Botao BOOT: passa para o proximo estimulo (ar, vapor, ar, chama, ar, fumaca, ar, alcool).
// LED RGB da placa: a cor do estimulo; vermelho piscando = falta sensor (rode o app16-0).

#include <Wire.h>
#include <Adafruit_SGP30.h>
#include <SensirionI2cSps30.h>
#include <Adafruit_BME280.h>

#define SDA_PIN  20
#define SCL_PIN  21
#define PINO_LED 8    // LED RGB interno (WS2812)
#define BRILHO   40   // 0 a 255
#define BOTAO    9    // ESTIMULO: botao BOOT da placa

Adafruit_SGP30    sgp;  // gas: TVOC, eCO2 e dois valores crus (H2 e etanol)
SensirionI2cSps30 sps;  // particulas: massa (PM) e contagem (NC)
Adafruit_BME280   bme;  // ambiente: temperatura e umidade

// ESTIMULO: o que esta perto da caixa, e a cor do LED (R, G, B)
const char* ESTIMULO[] = { "ar", "alcool", "vapor", "fumaca", "chama" };
const int   N_ESTIMULOS = 5;
const uint8_t COR_ESTIMULO[N_ESTIMULOS][3] = {
  {  0, 40,  0 },   // ar: verde
  { 40, 30,  0 },   // alcool: amarelo
  { 25,  0, 40 },   // vapor: roxo
  { 40,  0,  0 },   // fumaca: vermelho
  {  0,  0, 40 },   // chama: azul
};

// ESTIMULO: ordem do teste (alcool por ultimo: o SGP30 demora minutos para voltar)
const int SEQUENCIA[] = { 0, 2, 0, 4, 0, 3, 0, 1 };
const int N_PASSOS = 8;

int passo = 0;                         // ESTIMULO: posicao na SEQUENCIA
int ultimoBotao = HIGH;                // ESTIMULO
unsigned long ultimoDebounce = 0;      // ESTIMULO
const unsigned long debounceMs = 300;  // ESTIMULO

// ESTIMULO: mostra o estimulo atual no Serial e no LED
void mostraEstimulo() {
  int e = SEQUENCIA[passo];
  Serial.printf("=== ESTIMULO: %s ===\n", ESTIMULO[e]);
  neopixelWrite(PINO_LED, COR_ESTIMULO[e][0], COR_ESTIMULO[e][1], COR_ESTIMULO[e][2]);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000); // 100 kHz: o maximo que o SPS30 aceita
  pinMode(BOTAO, INPUT_PULLUP); // ESTIMULO

  // SGP30
  bool sgpOk = sgp.begin(&Wire);

  // SPS30: para a medicao anterior (se a placa reiniciou com ele ligado) e comeca de novo
  sps.begin(Wire, SPS30_I2C_ADDR_69);
  sps.stopMeasurement();
  delay(100);
  bool spsOk = (sps.startMeasurement(SPS30_OUTPUT_FORMAT_OUTPUT_FORMAT_FLOAT) == 0);

  // BME280 (SDO no G = endereco 0x76)
  bool bmeOk = bme.begin(0x76, &Wire);

  // Falta sensor: o teste precisa dos 3 (vermelho piscando; vermelho fixo e fumaca)
  if (!sgpOk || !spsOk || !bmeOk) {
    while (1) {
      Serial.printf("Erro: SGP30 %s | SPS30 %s | BME280 %s. Rode o app16-0.\n",
                    sgpOk ? "ok" : "FALHOU", spsOk ? "ok" : "FALHOU", bmeOk ? "ok" : "FALHOU");
      for (int i = 0; i < 3; i++) {
        neopixelWrite(PINO_LED, BRILHO, 0, 0);
        delay(500);
        neopixelWrite(PINO_LED, 0, 0, 0);
        delay(500);
      }
    }
  }

  Serial.println("Sensores ok. Espere 1 min e aperte BOOT para o primeiro estimulo.");
  mostraEstimulo(); // ESTIMULO
}

// O SGP30 precisa ser lido a cada 1 s; o SPS30 tem leitura nova a cada 1 s.
const int AMOSTRA_MS = 1000; // 1000 ms = 1 Hz

unsigned long tempoAnterior = 0;

void loop() {
  // ESTIMULO: botao BOOT passa para o proximo
  int leitura = digitalRead(BOTAO);
  if (ultimoBotao == HIGH && leitura == LOW) {
    if (millis() - ultimoDebounce > debounceMs) {
      passo = (passo + 1) % N_PASSOS;
      mostraEstimulo();
      ultimoDebounce = millis();
    }
  }
  ultimoBotao = leitura;

  if (millis() - tempoAnterior >= AMOSTRA_MS) {
    tempoAnterior = millis();

    // SGP30 (gas)
    // Nos primeiros ~15 s ele devolve TVOC = 0 e eCO2 = 400: esta aquecendo.
    // raw_h2 e raw_ethanol sao resistencia: CAEM quando o gas sobe.
    if (sgp.IAQmeasure() && sgp.IAQmeasureRaw()) {
      Serial.printf(">tvoc: %u\n>eco2: %u\n", sgp.TVOC, sgp.eCO2);
      Serial.printf(">raw_h2: %u\n>raw_ethanol: %u\n", sgp.rawH2, sgp.rawEthanol);
    }

    // SPS30 (particulas)
    // pm = massa, em ug/m3 | nc = contagem, em particulas por cm3
    float pm1_0, pm2_5, pm4_0, pm10, nc0_5, nc1_0, nc2_5, nc4_0, nc10, tamanho;
    if (sps.readMeasurementValuesFloat(pm1_0, pm2_5, pm4_0, pm10,
                                       nc0_5, nc1_0, nc2_5, nc4_0, nc10,
                                       tamanho) == 0) {
      Serial.printf(">pm2_5: %.1f\n>pm10: %.1f\n>nc0_5: %.1f\n", pm2_5, pm10, nc0_5);
    }

    // BME280 (ambiente)
    Serial.printf(">temp: %.1f\n>umid: %.1f\n", bme.readTemperature(), bme.readHumidity());

    // ESTIMULO: 0 ar, 1 alcool, 2 vapor, 3 fumaca, 4 chama
    Serial.printf(">estimulo: %d\n", SEQUENCIA[passo]);
  }
}
