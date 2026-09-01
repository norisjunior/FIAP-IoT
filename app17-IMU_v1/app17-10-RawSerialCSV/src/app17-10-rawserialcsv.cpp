/* =====================================================================
 * app17-10-rawserialcsv.cpp  —  FORMA 2: coleta RAW via Serial
 *
 * O QUE FAZ
 *   Le aceleracao (MPU6050/MPU6500) a 100 Hz e imprime SOMENTE "ax,ay,az"
 *   na Serial, continuamente. NAO calcula features e NAO usa MQTT/WiFi.
 *   Quem poe o timestamp e a label e o script Python (coletor_raw.py),
 *   que grava o CSV "timestamp,ax,ay,az,label" pedido na Sprint 3.
 *
 *   Pipeline (Aula 14): sinal raw -> Serial -> Python (timestamp+label) -> CSV
 *   -> (no Colab) janela -> features -> modelo.
 *
 * POR QUE GUARDAR O RAW (Forma 2 x Forma 1)
 *   A Forma 1 (app17-9) ja manda as features prontas (eficiente, mas perde o
 *   sinal). Aqui guardamos o bruto: no Colab da para VER o sinal inteiro,
 *   recortar o trecho exato do fenomeno e re-janelar a vontade.
 *
 * MONTAGEM (minima): apenas ESP32 + MPU6050 sobre a mesa. SEM motor, SEM
 *   botoes, SEM LED — o streaming e continuo.
 *   - NORMAL   = MPU6050 parado sobre a mesa (motor parado).
 *   - ANOMALIA = chacoalhar o ESP32/MPU (no Wokwi, sacudir o MPU no painel).
 *   A condicao (normal/anomalo) e escolhida no coletor_raw.py, ao iniciar
 *   cada rodada de coleta.
 *
 * DERIVADO DE app17-8 / app17-9: mesma pinagem do I2C, mesma lib (FlixPeriph),
 *   m/s², amostragem por millis() (nunca delay()). Removido tudo de features,
 *   MQTT, WiFi, NTP, botoes e LED.
 *
 * COMO O PYTHON LE A SERIAL
 *   - ESP32 fisico: coletor_raw.py abre a COM (ex.: COM6). Feche o Serial
 *     Monitor antes — so um programa abre a porta por vez.
 *   - Wokwi (simulador no VS Code): nao ha porta COM real; exponha a Serial
 *     via RFC2217 no wokwi.toml ("rfc2217ServerPort = 4000") e o Python
 *     conecta em "rfc2217://localhost:4000".
 *
 * ITEM DA SPRINT
 *   Sprint 3 - item 1 (coleta 100 Hz, ax,ay,az no Monitor Serial).
 *
 * PINAGEM: SDA=19  SCL=18.  Aceleracao em m/s² (Z em repouso ~ 9,81).
 * ===================================================================== */

#include <Arduino.h>
#include <FlixPeriph.h>
#include <MPU6500.h>
#include <Wire.h>

/* ---- Pinos (montagem minima, so o MPU) ---- */
#define SDA_PIN 19
#define SCL_PIN 18

/* Sensor: o codigo de aula usa MPU6500 (FlixPeriph). Se o seu sensor for o
 * MPU6050 original (GY-521), troque a linha abaixo por:  MPU6050 mpu(Wire);
 * A interface do FlixPeriph e a mesma. Aceleracao retornada em m/s². */
MPU6500 mpu(Wire);

uint32_t tempoAnterior = 0;
const int AMOSTRA_MS = 10;   // 100 Hz

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);
  if (!mpu.begin()) {
    Serial.println("Erro: MPU nao encontrado");
    while (1);
  }
  mpu.setAccelRange(IMUInterface::ACCEL_RANGE_8G);

  // Cabecalho da coluna. O coletor_raw.py ignora linhas nao numericas,
  // entao esta linha nao atrapalha o CSV.
  Serial.println("ax,ay,az");
}

void loop() {
  // Amostragem por millis() (nunca delay()): mantem ~100 Hz sem travar o loop.
  if (millis() - tempoAnterior >= AMOSTRA_MS) {
    tempoAnterior = millis();

    float ax, ay, az;
    mpu.read();
    mpu.getAccel(ax, ay, az);   // m/s²

    // Apenas os tres eixos, separados por virgula (formato da Sprint 3).
    Serial.printf("%.3f,%.3f,%.3f\n", ax, ay, az);
  }
}
