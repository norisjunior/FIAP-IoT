/* app17-9-accfeaturesinflux — features de aceleração por janela (1 s @ 100 Hz), publicadas via MQTT (JSON). */
/*
ALVO: ESP32-C6 (ex.: ESP32-C6-DevKitC-1). Requer o core Arduino-ESP32 3.x —
ver platformio.ini (platform `pioarduino`). Os GPIOs 2/4/18/21/22/23 usados
aqui são válidos no C6; o "LED onboard" da DevKitC-1 é o LED RGB (GPIO8),
acionado via RGB_BUILTIN.

PARA USAR NO WOKWI:
- Ajustar as credenciais WiFi e o IP do MQTT_SERVER
- Ajustar #define MPU_TYPE:
  - #define MPU_TYPE MPU6050
- Remover/comentar a linha `mpu.calibrateAccelGyro(&calib);`
*/

#include <Arduino.h>
#include "FastIMU.h"
#include <Wire.h>
#include <math.h>
#include <time.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

/* ---- Rede: use (A) Wokwi OU (B) ESP32 físico ---- */
// ---- (A) Wokwi (padrão) ----
// const char* WIFI_SSID     = "Wokwi-GUEST";
// const char* WIFI_PASSWORD = "";
// #define MQTT_SERVER "host.wokwi.internal"

// ---- (B) ESP32 físico ----
const char* WIFI_SSID     = "IoTNJ";
const char* WIFI_PASSWORD = "Th1ng$IoT";
#define MQTT_SERVER "10.93.11.155"   // IP da máquina com a IoT-platform

WiFiClient wifiClient;

/* ---- MQTT ---- */
#define MQTT_PORT      1883
#define MQTT_PUB_TOPIC "FIAPIoT/motor/features"
#define MQTT_CLIENT_ID "IoTDevJanelaFixa001"
PubSubClient mqttClient(wifiClient);

/* ---- Relógio (NTP) ---- */
const char* NTP_SERVER_1 = "pool.ntp.org";
const char* NTP_SERVER_2 = "time.google.com";
uint64_t epochBaseMs        = 0;
uint32_t millisNaSync       = 0;
bool     relogioSincronizado = false;

/* ---- Pinos (ESP32-C6) ---- */
#define BTN_COLETA   12   // inicia/para a coleta
#define BTN_ANOMALIA 13   // seleciona NORMAL/ANOMALIA
#define LED_PIN       7   // LED externo
#define SDA_PIN       5
#define SCL_PIN       4

// "LED onboard": na ESP32-C6-DevKitC-1 é o LED RGB WS2812 (GPIO8). O core 3.x
// intercepta digitalWrite(RGB_BUILTIN, HIGH/LOW) e acende/apaga o RGB em branco.
#ifdef RGB_BUILTIN
  #define LED_ONBOARD RGB_BUILTIN
#else
  #define LED_ONBOARD 8
#endif

/* ---- Sensor (MPU6050 ou MPU6500) ---- */
#define MPU_TYPE MPU6050
MPU_TYPE mpu;

calData calib = { 0 };

bool coletando     = false;
bool anomaliaAtiva = false;   // false = NORMAL, true = ANOMALIA

int ultimoBotaoColeta   = HIGH;
int ultimoBotaoAnomalia = HIGH;

unsigned long ultimoDebounceColeta   = 0;
unsigned long ultimoDebounceAnomalia = 0;
const unsigned long debounceMs = 300;

/* ---- Pisca do LED (condição ANOMALIA) ---- */
uint32_t ultimoPiscaLed = 0;
bool ledAceso = false;
const int PISCA_MS = 250;

const int TAMANHO_JANELA = 100;   // 100 amostras @ 100 Hz = 1 s
float ax_buf[TAMANHO_JANELA];
float ay_buf[TAMANHO_JANELA];
float az_buf[TAMANHO_JANELA];

int indice = 0;
uint32_t tempoAnterior = 0;
const int AMOSTRA_MS = 10; // 100 Hz

/* ---- Protótipos ---- */
void aplicarLeds(bool aceso);
void conectarWiFi();
void sincronizarRelogio();
uint64_t agoraEpochMs();
void conectarMQTT();
void publicarFeatures(const char* label, uint64_t ts_epoch_ms,
                      float mx, float my, float mz,
                      float sx, float sy, float sz,
                      float rx, float ry, float rz, float rmag);

/* ---- LEDs ---- */
// Escreve nos dois LEDs só quando o estado muda. Importante no C6: o LED
// onboard é RGB endereçável (WS2812) — reescrever a cada iteração do loop
// geraria flicker e pequenos atrasos (a rotina do WS2812 desabilita IRQ).
void aplicarLeds(bool aceso) {
  static int ultimo = -1;
  if (ultimo == (int)aceso) return;
  ultimo = aceso;
  digitalWrite(LED_PIN,     aceso ? HIGH : LOW);
  digitalWrite(LED_ONBOARD, aceso ? HIGH : LOW);
}

/* ---- Features ---- */
float calcMean(float arr[], int n) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += arr[i];
  return soma / n;
}

float calcStd(float arr[], int n) {
  float media = calcMean(arr, n);
  float soma = 0;
  for (int i = 0; i < n; i++) soma += (arr[i] - media) * (arr[i] - media);
  return sqrt(soma / n);
}

float calcRMS(float arr[], int n) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += arr[i] * arr[i];
  return sqrt(soma / n);
}

float calcRMSMagnitude(float axArr[], float ayArr[], float azArr[], int n) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += axArr[i]*axArr[i] + ayArr[i]*ayArr[i] + azArr[i]*azArr[i];
  return sqrt(soma / n);
}

/* ============================== SETUP ============================== */
void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);
  if (mpu.init(calib, 0x68) != 0) {
    Serial.println("Erro: MPU não encontrado");
    while (1);
  }
  mpu.setAccelRange(8);
  mpu.setAccelLPF(50);   // anti-aliasing p/ amostragem a 100 Hz (Nyquist 50 Hz)

  Serial.println("Mantenha o sensor parado e nivelado para calibrar...");
  delay(2000);
  mpu.calibrateAccelGyro(&calib);   // habilite no ESP32 físico; trava no Wokwi (FIFO ausente)
  mpu.init(calib, 0x68);

  Serial.println("MPU iniciado");

  pinMode(BTN_COLETA,   INPUT_PULLUP);
  pinMode(BTN_ANOMALIA, INPUT_PULLUP);
  pinMode(LED_PIN,      OUTPUT);
  pinMode(LED_ONBOARD,  OUTPUT);
  aplicarLeds(true);

  conectarWiFi();
  sincronizarRelogio();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setKeepAlive(60);
  mqttClient.setSocketTimeout(30);
  mqttClient.setBufferSize(512);

  Serial.println("Sistema pronto.");
  Serial.println("  Botão 21: inicia/para a coleta");
  Serial.println("  Botão 18: seleciona NORMAL/ANOMALIA (só com a coleta parada)");
  Serial.println("  LED aceso fixo = NORMAL | LED piscando = ANOMALIA");
  Serial.println("  NORMAL = MPU parado | ANOMALIA = movimente o MPU durante a coleta");
  Serial.printf("  Tópico MQTT: %s\r\n\r\n", MQTT_PUB_TOPIC);
  Serial.println("ts_epoch_ms,label,mean_ax,mean_ay,mean_az,std_ax,std_ay,std_az,rms_ax,rms_ay,rms_az,rms_mag");
}

/* ============================== LOOP =============================== */
void loop() {
  if (!mqttClient.connected()) {
    conectarMQTT();
  }
  mqttClient.loop();

  // --- Botão coleta (inicia/para) ---
  int leituraColeta = digitalRead(BTN_COLETA);
  if (ultimoBotaoColeta == HIGH && leituraColeta == LOW) {
    if (millis() - ultimoDebounceColeta > debounceMs) {
      coletando = !coletando;
      if (coletando) {
        indice = 0;
        tempoAnterior = millis();
        Serial.printf("Coleta INICIADA (condição: %s)\r\n",
                      anomaliaAtiva ? "ANOMALIA - movimente o MPU" : "NORMAL - deixe o MPU parado");
      } else {
        Serial.println("Coleta PARADA");
      }
      ultimoDebounceColeta = millis();
    }
  }
  ultimoBotaoColeta = leituraColeta;

  // --- Botão anomalia (seleciona condição) ---
  int leituraAnomalia = digitalRead(BTN_ANOMALIA);
  if (ultimoBotaoAnomalia == HIGH && leituraAnomalia == LOW) {
    if (millis() - ultimoDebounceAnomalia > debounceMs) {
      if (!coletando) {
        anomaliaAtiva = !anomaliaAtiva;
        Serial.printf("Condição selecionada: %s\r\n",
                      anomaliaAtiva ? "ANOMALIA (MPU em movimento)" : "NORMAL (MPU parado)");
      } else {
        Serial.println("Pare a coleta (botão 21) antes de trocar a condição.");
      }
      ultimoDebounceAnomalia = millis();
    }
  }
  ultimoBotaoAnomalia = leituraAnomalia;

  // --- LED indica a condição (externo GPIO4 + RGB onboard) ---
  if (anomaliaAtiva) {
    if (millis() - ultimoPiscaLed >= PISCA_MS) {
      ultimoPiscaLed = millis();
      ledAceso = !ledAceso;
      aplicarLeds(ledAceso);
    }
  } else {
    aplicarLeds(true);
    ledAceso = true;
  }

  if (!coletando) return;

  // --- Coleta IMU a 100 Hz ---
  if (millis() - tempoAnterior >= AMOSTRA_MS) {
    // Avança em passos fixos de AMOSTRA_MS (e não "= millis()"): assim o atraso
    // de um ciclo não empurra o próximo e a taxa não escorrega abaixo de 100 Hz.
    tempoAnterior += AMOSTRA_MS;
    // Se ainda estamos mais de uma amostra atrasados (reconexão MQTT, publish
    // lento), não adianta amostrar em rajada para recuperar: as amostras sairiam
    // sem espaçamento real. Recomeça do agora.
    if (millis() - tempoAnterior >= AMOSTRA_MS) tempoAnterior = millis();
    AccelData accel;
    mpu.update();
    mpu.getAccel(&accel);
    ax_buf[indice] = accel.accelX;
    ay_buf[indice] = accel.accelY;
    az_buf[indice] = accel.accelZ;
    indice++;

    if (indice >= TAMANHO_JANELA) {
      const char* label = anomaliaAtiva ? "ligado_anomalia" : "ligado_normal";

      uint64_t ts_epoch_ms = agoraEpochMs();

      float mx   = calcMean(ax_buf, TAMANHO_JANELA);
      float my   = calcMean(ay_buf, TAMANHO_JANELA);
      float mz   = calcMean(az_buf, TAMANHO_JANELA);
      float sx   = calcStd(ax_buf,  TAMANHO_JANELA);
      float sy   = calcStd(ay_buf,  TAMANHO_JANELA);
      float sz   = calcStd(az_buf,  TAMANHO_JANELA);
      float rx   = calcRMS(ax_buf,  TAMANHO_JANELA);
      float ry   = calcRMS(ay_buf,  TAMANHO_JANELA);
      float rz   = calcRMS(az_buf,  TAMANHO_JANELA);
      float rmag = calcRMSMagnitude(ax_buf, ay_buf, az_buf, TAMANHO_JANELA);

      Serial.printf("%llu,%s,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\r\n",
                    (unsigned long long)ts_epoch_ms, label,
                    mx, my, mz, sx, sy, sz, rx, ry, rz, rmag);

      publicarFeatures(label, ts_epoch_ms, mx, my, mz, sx, sy, sz, rx, ry, rz, rmag);

      indice = 0;
    }
  }
}

/* ---- WiFi ---- */
void conectarWiFi() {
  Serial.printf("Conectando ao WiFi %s", WIFI_SSID);
  // TxPower reduzido: margem contra brownout/reboot ao ligar o rádio quando a
  // alimentação é fraca (USB de PC, cabo ruim). Numa fonte estável pode subir.
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_2dBm);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println("");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

/* ---- Relógio (NTP) ---- */
void sincronizarRelogio() {
  configTime(0, 0, NTP_SERVER_1, NTP_SERVER_2);   // UTC (offset 0)
  Serial.print("Sincronizando relógio via NTP");
  struct tm tm;
  for (int i = 0; i < 16 && !relogioSincronizado; i++) {
    if (getLocalTime(&tm, 500)) relogioSincronizado = true;
    else { Serial.print('.'); delay(500); }
  }
  if (relogioSincronizado) {
    epochBaseMs  = (uint64_t)time(nullptr) * 1000ULL;
    millisNaSync = millis();
    Serial.printf(" OK (%04d-%02d-%02d %02d:%02d:%02d UTC)\r\n",
                  tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                  tm.tm_hour, tm.tm_min, tm.tm_sec);
  } else {
    Serial.println(" FALHOU. ts_epoch_ms vira uptime; a ponte usa o horário de recepção.");
  }
}

uint64_t agoraEpochMs() {
  return epochBaseMs + (uint64_t)(millis() - millisNaSync);
}

/* ---- MQTT ---- */
void conectarMQTT() {
  while (!mqttClient.connected()) {
    Serial.printf("Conectando ao MQTT Broker %s...", MQTT_SERVER);
    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      Serial.println(" Conectado!");
    } else {
      Serial.printf(" Falha rc=%d. Tentando em 5s...\r\n", mqttClient.state());
      delay(5000);
    }
  }
}

void publicarFeatures(const char* label, uint64_t ts_epoch_ms,
                      float mx, float my, float mz,
                      float sx, float sy, float sz,
                      float rx, float ry, float rz, float rmag) {
  JsonDocument doc;
  doc["device"]       = MQTT_CLIENT_ID;
  doc["label"]        = label;
  doc["ts_epoch_ms"]  = ts_epoch_ms;
  doc["mean_ax"] = serialized(String(mx, 3));
  doc["mean_ay"] = serialized(String(my, 3));
  doc["mean_az"] = serialized(String(mz, 3));
  doc["std_ax"]  = serialized(String(sx, 3));
  doc["std_ay"]  = serialized(String(sy, 3));
  doc["std_az"]  = serialized(String(sz, 3));
  doc["rms_ax"]  = serialized(String(rx, 3));
  doc["rms_ay"]  = serialized(String(ry, 3));
  doc["rms_az"]  = serialized(String(rz, 3));
  doc["rms_mag"] = serialized(String(rmag, 3));

  String buffer;
  serializeJson(doc, buffer);

  Serial.print("PAYLOAD MQTT: ");
  Serial.println(buffer.c_str());

  bool ok = mqttClient.publish(MQTT_PUB_TOPIC, buffer.c_str());
  Serial.println(ok ? "MQTT: enviado com sucesso" : "MQTT: falha no envio");
}
