#include <FlixPeriph.h>
// Remova o comentario abaixo se for utilizar
// o sensor MPU6500 (vendido como MPU6050 generico)
// #include <MPU6500.h>
#include <Wire.h>
#include <math.h>

#define SCLPIN 27
#define SDAPIN 26
#define LEDPIN 25

#define INTERVALO_AMOSTRA_MS 20 // 50 Hz
#define TAMANHO_JANELA 100      // 100 amostras = 2 segundos

// Caso use o MPU6500, descomente a linha a seguir e comente
// a linha do MPU6050
// MPU6500 mpu(Wire);
MPU6050 mpu(Wire);

int janela = 0;
int amostras = 0;
unsigned long tempoAnterior = 0;
bool estadoLED = false;

float somaX = 0;
float somaY = 0;
float somaZ = 0;
float somaX2 = 0;
float somaY2 = 0;
float somaZ2 = 0;
float somaMagnitude = 0;

void zerarJanela() {
    amostras = 0;
    somaX = 0;
    somaY = 0;
    somaZ = 0;
    somaX2 = 0;
    somaY2 = 0;
    somaZ2 = 0;
    somaMagnitude = 0;
}

void setup() {
    Serial.begin(115200);
    pinMode(LEDPIN, OUTPUT);

    Wire.begin(SDAPIN, SCLPIN);

    if (mpu.begin() < 0) {
        Serial.println("Erro: MPU nao encontrado");
        while (1) {
            delay(10);
        }
    }

    Serial.println("Sensor MPU iniciado com sucesso");
    delay(1000);
    Serial.println("Dispositivo IoT ESP32 - features de acelerometro por janela");
    Serial.println("janela,n_amostras,media_x,media_y,media_z,std_x,std_y,std_z,mag_media,rms_x,rms_y,rms_z,rms_mag");

    tempoAnterior = millis();
}

void loop() {
    if (millis() - tempoAnterior >= INTERVALO_AMOSTRA_MS) {
        tempoAnterior = millis();

        float ax, ay, az;

        mpu.read();
        mpu.getAccel(ax, ay, az);

        somaX += ax;
        somaY += ay;
        somaZ += az;

        somaX2 += ax * ax;
        somaY2 += ay * ay;
        somaZ2 += az * az;

        somaMagnitude += sqrt((ax * ax) + (ay * ay) + (az * az));
        amostras++;

        if (amostras == TAMANHO_JANELA) {
            janela++;

            float mediaX = somaX / TAMANHO_JANELA;
            float mediaY = somaY / TAMANHO_JANELA;
            float mediaZ = somaZ / TAMANHO_JANELA;

            float mediaX2 = somaX2 / TAMANHO_JANELA;
            float mediaY2 = somaY2 / TAMANHO_JANELA;
            float mediaZ2 = somaZ2 / TAMANHO_JANELA;

            float stdX = sqrt(max(0.0f, mediaX2 - (mediaX * mediaX)));
            float stdY = sqrt(max(0.0f, mediaY2 - (mediaY * mediaY)));
            float stdZ = sqrt(max(0.0f, mediaZ2 - (mediaZ * mediaZ)));

            float magMedia = somaMagnitude / TAMANHO_JANELA;
            float rmsX = sqrt(mediaX2);
            float rmsY = sqrt(mediaY2);
            float rmsZ = sqrt(mediaZ2);
            float rmsMag = sqrt(mediaX2 + mediaY2 + mediaZ2);

            Serial.printf(
                "%d,%d,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n",
                janela,
                TAMANHO_JANELA,
                mediaX,
                mediaY,
                mediaZ,
                stdX,
                stdY,
                stdZ,
                magMedia,
                rmsX,
                rmsY,
                rmsZ,
                rmsMag
            );

            estadoLED = !estadoLED;
            digitalWrite(LEDPIN, estadoLED);

            zerarJanela();
        }
    }
}
