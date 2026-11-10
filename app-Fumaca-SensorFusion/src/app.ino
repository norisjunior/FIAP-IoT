// app-Fumaca-SensorFusion (provisorio: vira o app21-0-VerificaSensores)
// Caixa de fumaca: confere se os 3 sensores respondem no I2C.
//
// Placa: ESP32-C3 Super Mini (Plus, com antena externa)
// Os 3 sensores dividem o mesmo barramento I2C (os mesmos 2 fios):
//   SGP30  VIN -> 3V3   GND -> G   SDA -> GPIO4   SCL -> GPIO3
//   BME280 VCC -> 3V3   GND -> G   SDA -> GPIO4   SCL -> GPIO3   CSB -> 3V3   SDO -> G
//   SPS30  VDD -> 5V    GND -> G   SDA -> GPIO4   SCL -> GPIO3   SEL -> G
//
// LED RGB da placa (GPIO 8): verde = os 3 responderam; vermelho = falta algum.

#include <Wire.h>

#define SDA_PIN  4
#define SCL_PIN  3
#define PINO_LED 8    // LED RGB interno (WS2812)
#define BRILHO   40   // 0 a 255

// Endereco I2C de cada sensor. O do BME280 depende do pino SDO.
#define END_SGP30  0x58
#define END_SPS30  0x69
#define END_BME280 0x76   // SDO no GND (com SDO no 3V3 seria 0x77)

// O sensor responde quando o ESP32 chama o endereco dele
bool responde(byte endereco) {
  Wire.beginTransmission(endereco);
  return Wire.endTransmission() == 0;
}

// Le 1 byte de um registrador do sensor (usado para o ID do chip do BME280)
byte leRegistro(byte endereco, byte registrador) {
  Wire.beginTransmission(endereco);
  Wire.write(registrador);
  Wire.endTransmission();
  Wire.requestFrom(endereco, (byte)1);
  return Wire.available() ? Wire.read() : 0;
}

// ID do chip (registrador 0xD0): 0x60 = BME280. 0x58 = BMP280, que e parecido,
// usa o mesmo endereco, mas NAO mede umidade (as vezes vem vendido como BME280).
#define ID_BME280 0x60

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000); // 100 kHz: o maximo que o SPS30 aceita
  Serial.println("Caixa de fumaca: verificando os sensores no I2C");
}

void loop() {
  // 1) Varre o barramento e mostra todos os enderecos que responderam
  Serial.print("Enderecos encontrados:");
  int total = 0;
  for (byte e = 1; e < 127; e++) {
    if (responde(e)) {
      Serial.printf(" 0x%02X", e);
      total++;
    }
  }
  if (total == 0) {
    Serial.print(" nenhum (confira 3V3, G, SDA no GPIO4 e SCL no GPIO3)");
  }
  Serial.println();

  // 2) Confere os 3 sensores da caixa
  bool sgp = responde(END_SGP30);
  bool sps = responde(END_SPS30);
  bool bme = responde(END_BME280) && leRegistro(END_BME280, 0xD0) == ID_BME280;

  if (sgp) Serial.println("  SGP30  (0x58) ok");
  else     Serial.println("  SGP30  (0x58) NAO respondeu: VIN no 3V3? SDA e SCL trocados?");

  if (sps) Serial.println("  SPS30  (0x69) ok");
  else     Serial.println("  SPS30  (0x69) NAO respondeu: VDD no 5V? SEL no G?");

  if (bme)                 Serial.println("  BME280 (0x76) ok");
  else if (responde(END_BME280))
    Serial.printf("  0x76 responde, mas o chip nao e BME280 (ID 0x%02X; 0x58 = BMP280, sem umidade)\n",
                  leRegistro(END_BME280, 0xD0));
  else if (responde(0x77)) Serial.println("  BME280 respondeu em 0x77: ligue o SDO no G");
  else                     Serial.println("  BME280 (0x76) NAO respondeu: VCC no 3V3? CSB no 3V3?");

  // 3) LED: verde se os 3 responderam, vermelho se falta algum
  if (sgp && sps && bme) {
    neopixelWrite(PINO_LED, 0, BRILHO, 0);
  } else {
    neopixelWrite(PINO_LED, BRILHO, 0, 0);
  }

  Serial.println();
  delay(3000);
}
