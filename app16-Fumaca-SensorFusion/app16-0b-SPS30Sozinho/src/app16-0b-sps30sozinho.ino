// app16-0b-SPS30Sozinho
// So o SPS30, numa placa separada: identificacao, alarmes e as 10 leituras no plotter (Teleplot).
// Pronto quando: aparece o tipo 00080000, nenhum ALARME, e o vapor faz pm e nc subirem.
//
// Ligacao: SPS30 VDD -> 5V   GND -> G   SDA -> GPIO4   SCL -> GPIO3   SEL -> G
// Se der FALHOU ou ERRO: 10 kOhm do GPIO4 ao 3V3 e 10 kOhm do GPIO3 ao 3V3 (pull-up)
// LED RGB da placa: verde = medindo sem alarme; vermelho = nao responde, erro ou alarme.

#include <Wire.h>
#include <SensirionI2cSps30.h>

#define SDA_PIN  4
#define SCL_PIN  3
#define PINO_LED 8    // LED RGB interno (WS2812)
#define BRILHO   40   // 0 a 255

// Alarmes do registrador de status do SPS30
#define ALARME_ROTACAO    (1UL << 21) // ventilador rapido ou devagar demais
#define ALARME_LASER      (1UL << 5)  // laser fora da faixa
#define ALARME_VENTILADOR (1UL << 4)  // ventilador parado

SensirionI2cSps30 sps;  // particulas: massa (PM) e contagem (NC)

bool spsOk = false;

// Liga o SPS30 e mostra a identificacao (ou por que falhou)
bool iniciaSps() {
  // Para a medicao anterior (se a placa reiniciou com ele ligado)
  sps.stopMeasurement();
  delay(100);

  int8_t tipo[9] = {0};
  int8_t serie[33] = {0};
  uint8_t fwMaior = 0, fwMenor = 0;
  bool ok = (sps.readProductType(tipo, 8) == 0) &&
            (sps.readSerialNumber(serie, 32) == 0) &&
            (sps.readFirmwareVersion(fwMaior, fwMenor) == 0) &&
            (sps.startMeasurement(SPS30_OUTPUT_FORMAT_OUTPUT_FORMAT_FLOAT) == 0);

  if (ok) {
    Serial.printf("SPS30 ok | tipo %s | serie %s | firmware %u.%u\n",
                  (char*)tipo, (char*)serie, fwMaior, fwMenor);
    neopixelWrite(PINO_LED, 0, BRILHO, 0); // verde
  } else {
    Wire.beginTransmission(SPS30_I2C_ADDR_69);
    if (Wire.endTransmission() == 0) {
      Serial.println("SPS30 FALHOU: 0x69 responde, mas a leitura falha. Ponha os pull-ups.");
    } else {
      Serial.println("SPS30 FALHOU: 0x69 nao responde. Confira SEL no G, VDD no 5V, SDA no 4 e SCL no 3.");
    }
    neopixelWrite(PINO_LED, BRILHO, 0, 0); // vermelho
  }
  return ok;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(10000); // 10 kHz: sobra tempo com o pull-up fraco do ESP32

  sps.begin(Wire, SPS30_I2C_ADDR_69);
  spsOk = iniciaSps();
}

// O SPS30 tem leitura nova a cada 1 s.
const int AMOSTRA_MS = 1000; // 1000 ms = 1 Hz

unsigned long tempoAnterior = 0;

void loop() {
  // Sem SPS30: tenta de novo a cada 2 s
  if (!spsOk) {
    delay(2000);
    spsOk = iniciaSps();
    return;
  }

  if (millis() - tempoAnterior >= AMOSTRA_MS) {
    tempoAnterior = millis();

    // Leituras
    // pm = massa, em ug/m3 | nc = contagem, em particulas por cm3 | tamanho tipico, em um
    float pm1_0, pm2_5, pm4_0, pm10, nc0_5, nc1_0, nc2_5, nc4_0, nc10, tamanho;
    bool leu = (sps.readMeasurementValuesFloat(pm1_0, pm2_5, pm4_0, pm10,
                                               nc0_5, nc1_0, nc2_5, nc4_0, nc10,
                                               tamanho) == 0);
    if (leu) {
      Serial.printf(">pm1_0: %.1f\n>pm2_5: %.1f\n>pm4_0: %.1f\n>pm10: %.1f\n",
                    pm1_0, pm2_5, pm4_0, pm10);
      Serial.printf(">nc0_5: %.1f\n>nc1_0: %.1f\n>nc2_5: %.1f\n>nc4_0: %.1f\n>nc10: %.1f\n",
                    nc0_5, nc1_0, nc2_5, nc4_0, nc10);
      Serial.printf(">tamanho: %.2f\n", tamanho);
    } else {
      Serial.println("ERRO de leitura: confira os pull-ups e o cabo do SPS30.");
    }

    // Alarmes
    uint32_t status = 0;
    sps.readDeviceStatusRegister(status);
    if (status & ALARME_ROTACAO)    Serial.println("ALARME: rotacao do ventilador fora da faixa");
    if (status & ALARME_LASER)      Serial.println("ALARME: falha no laser");
    if (status & ALARME_VENTILADOR) Serial.println("ALARME: ventilador parado");

    // LED
    bool alarme = status & (ALARME_ROTACAO | ALARME_LASER | ALARME_VENTILADOR);
    if (leu && !alarme) {
      neopixelWrite(PINO_LED, 0, BRILHO, 0); // verde
    } else {
      neopixelWrite(PINO_LED, BRILHO, 0, 0); // vermelho
    }
  }
}
