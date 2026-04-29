#include <FlixPeriph.h>
#include <MPU6500.h>
#include <Wire.h>

#define SDAPIN 22
#define SCLPIN 23

#define LEDPIN 21

void setup() {
  Serial.begin(115200);
  pinMode(LEDPIN, OUTPUT);
  Wire.begin(SDAPIN, SCLPIN);
  delay(1000);

  Serial.println("=== I2C Scanner + Identificador de IMU ===\n");

  // Fase 1: scan de todos os endereços I2C
  int encontrados = 0;
  Serial.println("Dispositivos encontrados:");
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  -> 0x%02X\n", addr);
      encontrados++;
    }
  }

  if (encontrados == 0) {
    Serial.println("  Nenhum dispositivo encontrado. Verifique as conexoes.");
    return;
  }

  Serial.printf("\nTotal: %d dispositivo(s)\n\n", encontrados);

  // Fase 2: ler WHO_AM_I no endereço 0x68 (padrão MPU)
  Wire.beginTransmission(0x68);
  if (Wire.endTransmission() != 0) {
    Serial.println("Nenhum sensor no endereco 0x68.");
    return;
  }

  Wire.beginTransmission(0x68);
  Wire.write(0x75); // registrador WHO_AM_I
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)0x68, (uint8_t)1);

  if (!Wire.available()) {
    Serial.println("Falha ao ler WHO_AM_I.");
    return;
  }

  byte whoami = Wire.read();
  Serial.printf("WHO_AM_I = 0x%02X\n", whoami);

  // Fase 3: identificar e testar com FlixPeriph
  switch (whoami) {
    case 0x68: {
      Serial.println("Chip: MPU6050 (GY-521)");
      MPU6050 imu(Wire);
      if (imu.begin() >= 0) {
        Serial.println("Status: OK via FlixPeriph MPU6050");
        imu.read();
        float ax, ay, az;
        imu.getAccel(ax, ay, az);
        Serial.printf("Teste: ax=%.2fg ay=%.2fg az=%.2fg\n", ax, ay, az);
      } else {
        Serial.println("Status: FALHA no begin()");
      }
      digitalWrite(LEDPIN, HIGH);
      delay(1000);
      break;
    }
    case 0x70: {
      Serial.println("Chip: MPU6500 (vendido como MPU6050 generico)");
      Wire.setClock(400000);
      MPU6500 imu(Wire);
      if (imu.begin() >= 0) {
        Serial.println("Status: OK via FlixPeriph MPU6500");
        imu.read();
        float ax, ay, az;
        imu.getAccel(ax, ay, az);
        Serial.printf("Teste: ax=%.2fg ay=%.2fg az=%.2fg\n", ax, ay, az);
      } else {
        Serial.println("Status: FALHA no begin()");
      }
      digitalWrite(LEDPIN, HIGH);
      delay(1000);
      break;
    }
    case 0x71:
      Serial.println("Chip: MPU9250");
      Serial.println("Use a classe MPU9250 do FlixPeriph.");
      break;
    case 0x75:
      Serial.println("Chip: MPU6515");
      Serial.println("Use a classe MPU6500 do FlixPeriph.");
      break;
    case 0xEA:
      Serial.println("Chip: ICM-20948");
      Serial.println("Use a classe ICM20948 do FlixPeriph.");
      break;
    default:
      Serial.printf("Chip desconhecido (WHO_AM_I = 0x%02X)\n", whoami);
      Serial.println("Verifique o datasheet do seu sensor.");
      break;
  }

  Serial.println("\n=== Fim ===");
}

void loop() {
}