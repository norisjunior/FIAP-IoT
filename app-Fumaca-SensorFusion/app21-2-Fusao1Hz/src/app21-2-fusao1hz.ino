// app21-2-Fusao1Hz
// Caixa de fumaca: os 3 sensores numa linha so, 1 linha por segundo.
// Uma linha = um exemplo do dataset (no app21-3 a mesma linha vira o JSON do MQTT).
// Pronto quando: o Serial imprime o cabecalho e depois 1 linha por segundo.
//
// Ligacao: a mesma do app21-0 (os 3 sensores no I2C, SDA no GPIO4 e SCL no GPIO3).
// LED RGB da placa: amarelo = aquecendo; verde = medindo; vermelho = falta sensor.

#include <Wire.h>
#include <Adafruit_SGP30.h>
#include <SensirionI2cSps30.h>
#include <Adafruit_BME280.h>

#define SDA_PIN  4
#define SCL_PIN  3
#define PINO_LED 8    // LED RGB interno (WS2812)
#define BRILHO   40   // 0 a 255

Adafruit_SGP30    sgp;  // gas
SensirionI2cSps30 sps;  // particulas
Adafruit_BME280   bme;  // temperatura e umidade

// Aquecimento: o SGP30 fica fixo em TVOC = 0 e eCO2 = 400 por ~15 s,
// e o ventilador do SPS30 leva uns segundos para estabilizar.
// Nesse tempo o programa le os sensores, mas nao imprime a linha.
const int AQUECIMENTO_S = 20;

// Umidade absoluta (mg/m3) a partir da temperatura e da umidade relativa.
// E a conta do datasheet do SGP30: ele usa esse numero para corrigir a leitura de gas.
uint32_t umidadeAbsoluta(float temp, float umid) {
  float gm3 = 216.7f * ((umid / 100.0f) * 6.112f * exp((17.62f * temp) / (243.12f + temp))
              / (273.15f + temp));
  return (uint32_t)(1000.0f * gm3);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000); // 100 kHz: o maximo que o SPS30 aceita

  bool sgpOk = sgp.begin(&Wire);

  sps.begin(Wire, SPS30_I2C_ADDR_69);
  sps.stopMeasurement(); // se a placa reiniciou com ele medindo
  delay(100);
  bool spsOk = (sps.startMeasurement(SPS30_OUTPUT_FORMAT_OUTPUT_FORMAT_FLOAT) == 0);

  bool bmeOk = bme.begin(0x76, &Wire);

  if (!sgpOk || !spsOk || !bmeOk) {
    neopixelWrite(PINO_LED, BRILHO, 0, 0); // vermelho
    while (1) {
      Serial.printf("Erro: SGP30 %s | SPS30 %s | BME280 %s. Rode o app21-0.\n",
                    sgpOk ? "ok" : "FALHOU", spsOk ? "ok" : "FALHOU", bmeOk ? "ok" : "FALHOU");
      delay(3000);
    }
  }

  neopixelWrite(PINO_LED, BRILHO, BRILHO, 0); // amarelo: aquecendo
  Serial.printf("Sensores ok. Aquecendo por %d s...\n", AQUECIMENTO_S);
}

// O SGP30 precisa ser lido a cada 1 s; o SPS30 tem leitura nova a cada 1 s.
const int AMOSTRA_MS = 1000; // 1000 ms = 1 Hz

unsigned long tempoAnterior = 0;
int segundos = 0; // segundos desde que ligou
int amostra  = 0; // linhas impressas

void loop() {
  if (millis() - tempoAnterior >= AMOSTRA_MS) {
    tempoAnterior = millis();
    segundos++;

    // 1) BME280: temperatura e umidade
    float temp = bme.readTemperature();
    float umid = bme.readHumidity();
    bool leuBme = !isnan(temp) && !isnan(umid);

    // 2) SGP30: a umidade do BME280 corrige a leitura de gas
    if (leuBme) {
      sgp.setHumidity(umidadeAbsoluta(temp, umid));
    }
    bool leuSgp = sgp.IAQmeasure() && sgp.IAQmeasureRaw();

    // 3) SPS30: particulas (pm = massa em ug/m3; nc = contagem por cm3)
    float pm1_0, pm2_5, pm4_0, pm10, nc0_5, nc1_0, nc2_5, nc4_0, nc10, tamanho;
    bool leuSps = (sps.readMeasurementValuesFloat(pm1_0, pm2_5, pm4_0, pm10,
                                                  nc0_5, nc1_0, nc2_5, nc4_0, nc10,
                                                  tamanho) == 0);

    // 4) Imprime a linha (depois do aquecimento)
    if (!leuBme || !leuSgp || !leuSps) {
      Serial.println("Erro de leitura: confira os fios do I2C");
    } else if (segundos > AQUECIMENTO_S) {
      if (amostra == 0) {
        neopixelWrite(PINO_LED, 0, BRILHO, 0); // verde: medindo
        Serial.println("amostra,pm1_0,pm2_5,pm10,nc0_5,nc1_0,nc2_5,"
                       "raw_h2,raw_ethanol,tvoc,eco2,temp,umid");
      }
      amostra++;
      Serial.printf("%d,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%u,%u,%u,%u,%.1f,%.1f\n",
                    amostra, pm1_0, pm2_5, pm10, nc0_5, nc1_0, nc2_5,
                    sgp.rawH2, sgp.rawEthanol, sgp.TVOC, sgp.eCO2, temp, umid);
    }
  }
}
