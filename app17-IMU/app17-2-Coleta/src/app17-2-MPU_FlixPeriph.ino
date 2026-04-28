#include <FlixPeriph.h>
//Remova o comentário abaixo se for utilizar
//o sensor MPU6500 (vendido como MPU6050 genérico)
//#include <MPU6500.h>
#include <Wire.h>

#define SCLPIN 27
#define SDAPIN 26
#define LEDPIN 25

// Caso use o MPU6500, descomente a linha a seguir e comente
//a linha do MPU6050
//MPU6500 mpu(Wire);
MPU6050 mpu(Wire);

int contador = 0;

void setup() {
    Serial.begin(115200);
    pinMode(LEDPIN, OUTPUT);

    Wire.begin(SDAPIN, SCLPIN);

    if(mpu.begin() < 0) {
        Serial.println("Erro: MPU não encontrado");
        while (1) {
            // Bloqueia o ESP32
        };
    }

    Serial.println("Sensor MPU inciado com sucesso");
    delay(1000);
    Serial.println("Dispositivo IoT ESP32 detector de vibração");
    Serial.println("id,x,y,z,status");
}

void loop() {
    contador++;

    float ax, ay, az;

    mpu.read();
    mpu.getAccel(ax, ay, az);

    Serial.printf("%d,%.2f,%.2f,%.2f,%s",contador,ax,ay,az);
    Serial.println("");
    delay(1);
}