// app16-3-Fusao1Hz
// Caixa de fumaca: o coletor do dataset. Uma linha por segundo, com as colunas do Kaggle.
// O botao BOOT marca o estimulo (o rotulo) e abre uma rodada nova.
// Pronto quando: o painel do coleta.py mostra as curvas e o coleta-*.csv ganha 1 linha por segundo.
//
// Ligacao: a mesma do app16-0 (os 3 sensores no I2C, SDA no GPIO20 e SCL no GPIO21).
// LED RGB da placa: a cor do estimulo; vermelho piscando = falta sensor (rode o app16-0).

#include <Wire.h>
#include <Adafruit_SGP30.h>
#include <SensirionI2cSps30.h>
#include <Adafruit_BME280.h>

#define SDA_PIN  20
#define SCL_PIN  21
#define PINO_LED 8    // LED RGB interno (WS2812)
#define BRILHO   40   // 0 a 255
#define BOTAO    9    // botao BOOT da placa

Adafruit_SGP30    sgp;  // gas: TVOC, eCO2 e dois valores crus (H2 e etanol)
SensirionI2cSps30 sps;  // particulas: massa (PM) e contagem (NC)
Adafruit_BME280   bme;  // ambiente: temperatura, umidade e pressao

// Estimulo: o que esta perto da caixa, e a cor do LED (R, G, B). Os mesmos do app16-2.
const char* ESTIMULO[] = { "ar", "alcool", "vapor", "fumaca", "chama" };
const int   N_ESTIMULOS = 5;
const uint8_t COR_ESTIMULO[N_ESTIMULOS][3] = {
  {  0, 40,  0 },   // ar: verde
  { 40, 30,  0 },   // alcool: amarelo
  { 25,  0, 40 },   // vapor: roxo
  { 40,  0,  0 },   // fumaca: vermelho
  {  0,  0, 40 },   // chama: azul
};

// Ordem do teste (alcool por ultimo: o SGP30 demora minutos para voltar)
const int SEQUENCIA[] = { 0, 2, 0, 4, 0, 3, 0, 1 };
const int N_PASSOS = 8;

int passo  = 0;  // posicao na SEQUENCIA
int rodada = 1;  // cada aperto do BOOT abre uma rodada nova (a rodada 1 e o aquecimento)
int ultimoBotao = HIGH;
unsigned long ultimoDebounce = 0;
const unsigned long debounceMs = 300;

// Mostra o estimulo e a rodada no Serial e no LED
void mostraEstimulo() {
  int e = SEQUENCIA[passo];
  Serial.printf("=== ESTIMULO: %s (rodada %d) ===\n", ESTIMULO[e], rodada);
  neopixelWrite(PINO_LED, COR_ESTIMULO[e][0], COR_ESTIMULO[e][1], COR_ESTIMULO[e][2]);
}

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
  pinMode(BOTAO, INPUT_PULLUP);

  // SGP30
  bool sgpOk = sgp.begin(&Wire);

  // SPS30: para a medicao anterior (se a placa reiniciou com ele ligado) e comeca de novo
  sps.begin(Wire, SPS30_I2C_ADDR_69);
  sps.stopMeasurement();
  delay(100);
  bool spsOk = (sps.startMeasurement(SPS30_OUTPUT_FORMAT_OUTPUT_FORMAT_FLOAT) == 0);

  // BME280 (SDO no G = endereco 0x76)
  bool bmeOk = bme.begin(0x76, &Wire);

  // Falta sensor: o dataset precisa dos 3 (vermelho piscando; vermelho fixo e fumaca)
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

  Serial.println("Sensores ok. A rodada 1 e o aquecimento: espere a caixa estabilizar e aperte BOOT.");
  mostraEstimulo();
}

// O SGP30 precisa ser lido a cada 1 s; o SPS30 tem leitura nova a cada 1 s.
const int AMOSTRA_MS = 1000; // 1000 ms = 1 Hz

unsigned long tempoAnterior = 0;
int amostra = 0; // +1 a cada segundo; um numero que falta no CSV e uma leitura que falhou

void loop() {
  // Botao BOOT: proximo estimulo, rodada nova
  int leitura = digitalRead(BOTAO);
  if (ultimoBotao == HIGH && leitura == LOW) {
    if (millis() - ultimoDebounce > debounceMs) {
      passo = (passo + 1) % N_PASSOS;
      rodada++;
      mostraEstimulo();
      ultimoDebounce = millis();
    }
  }
  ultimoBotao = leitura;

  if (millis() - tempoAnterior >= AMOSTRA_MS) {
    tempoAnterior = millis();
    amostra++;

    // 1) BME280: temperatura, umidade e pressao (a biblioteca entrega Pa; o Kaggle usa hPa)
    float temp    = bme.readTemperature();
    float umid    = bme.readHumidity();
    float pressao = bme.readPressure() / 100.0f;
    bool leuBme = !isnan(temp) && !isnan(umid) && !isnan(pressao);

    // 2) SGP30: a umidade do BME280 corrige a leitura de gas
    // Nos primeiros ~15 s ele devolve TVOC = 0 e eCO2 = 400: esta aquecendo.
    // raw_h2 e raw_ethanol sao resistencia: CAEM quando o gas sobe.
    if (leuBme) {
      sgp.setHumidity(umidadeAbsoluta(temp, umid));
    }
    bool leuSgp = sgp.IAQmeasure() && sgp.IAQmeasureRaw();

    // 3) SPS30: particulas (pm = massa em ug/m3; nc = contagem por cm3)
    float pm1_0, pm2_5, pm4_0, pm10, nc0_5, nc1_0, nc2_5, nc4_0, nc10, tamanho;
    bool leuSps = (sps.readMeasurementValuesFloat(pm1_0, pm2_5, pm4_0, pm10,
                                                  nc0_5, nc1_0, nc2_5, nc4_0, nc10,
                                                  tamanho) == 0);

    // 4) A linha: so sai com os 3 sensores lidos; a ordem das colunas e a do Kaggle
    if (!leuBme || !leuSgp || !leuSps) {
      Serial.printf("Erro de leitura na amostra %d: SGP30 %s | SPS30 %s | BME280 %s\n", amostra,
                    leuSgp ? "ok" : "FALHOU", leuSps ? "ok" : "FALHOU", leuBme ? "ok" : "FALHOU");
      return;
    }
    Serial.printf(">amostra: %d\n>rodada: %d\n", amostra, rodada);
    Serial.printf(">temp: %.2f\n>umid: %.2f\n", temp, umid);
    Serial.printf(">tvoc: %u\n>eco2: %u\n", sgp.TVOC, sgp.eCO2);
    Serial.printf(">raw_h2: %u\n>raw_ethanol: %u\n", sgp.rawH2, sgp.rawEthanol);
    Serial.printf(">pressao: %.2f\n", pressao);
    Serial.printf(">pm1_0: %.2f\n>pm2_5: %.2f\n", pm1_0, pm2_5);
    Serial.printf(">nc0_5: %.2f\n>nc1_0: %.2f\n>nc2_5: %.2f\n", nc0_5, nc1_0, nc2_5);
    Serial.printf(">pm10: %.2f\n", pm10);

    // Rotulo: 0 ar, 1 alcool, 2 vapor, 3 fumaca, 4 chama (fecha a linha no coleta.py)
    Serial.printf(">estimulo: %d\n", SEQUENCIA[passo]);
  }
}
