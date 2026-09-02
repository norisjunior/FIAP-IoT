/* app24-binary-inferencia — o ESP32 mede, pergunta para a nuvem e obedece.

   As features são as mesmas do app17-6 (janela de 100 amostras @ 100 Hz), mas
   aqui o dispositivo não rotula nada: publica a janela em FIAPIoT/motor/features
   e a classe prevista volta em FIAPIoT/motor/features/cmd. O LED mostra o que a
   NUVEM respondeu.

   Repare no que não existe aqui: nenhum if sobre vibração, nenhum limiar — e
   nenhum botão. O app17-6 tinha botões porque era um GERADOR DE DATASET, onde
   um humano rotulava cada janela. Este é um MONITOR de condição: quem rotula é
   o modelo, do outro lado da rede.
*/
/*
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
const char* WIFI_SSID     = "NorisIoT";
const char* WIFI_PASSWORD = "Secure10T";
#define MQTT_SERVER "172.16.10.101"   // IP da máquina com a IoT-platform

WiFiClient wifiClient;

/* ---- MQTT ---- */
#define MQTT_PORT      1883
#define MQTT_PUB_TOPIC "FIAPIoT/motor/features"       // a janela vai por aqui
#define MQTT_SUB_TOPIC "FIAPIoT/motor/features/cmd"   // a classe volta por aqui
#define MQTT_CLIENT_ID "IoTDevInferenciaBinaria001"
PubSubClient mqttClient(wifiClient);

/* ---- Relógio (NTP) ---- */
const char* NTP_SERVER_1 = "pool.ntp.org";
const char* NTP_SERVER_2 = "time.google.com";
uint64_t epochBaseMs        = 0;
uint32_t millisNaSync       = 0;
bool     relogioSincronizado = false;

/* ---- Pinos ---- */
#define LED_PIN       4   // LED externo: aceso = normal, piscando = anomalia
#define LED_ONBOARD   2   // LED onboard: aceso = conectado ao broker
#define SDA_PIN      22
#define SCL_PIN      23

/* ---- Sensor (MPU6050 ou MPU6500) ---- */
#define MPU_TYPE MPU6500
MPU_TYPE mpu;

calData calib = { 0 };

/* ---- A resposta da nuvem ----
   Vazia até a primeira resposta chegar: com o LED apagado, dá para ver a
   diferença entre "ainda não perguntei" e "a nuvem disse normal". */
String classePrevista = "";

/* ---- Pisca do LED (classe ligado_anomalia) ---- */
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
void conectarWiFi();
void sincronizarRelogio();
uint64_t agoraEpochMs();
void conectarMQTT();
void receberComando(char* topico, byte* conteudo, unsigned int tamanho);
void atualizarLed();
void publicarFeatures(uint64_t ts_epoch_ms,
                      float mx, float my, float mz,
                      float sx, float sy, float sz, float rmag);

/* ---- Features: exatamente as 7 que o modelo recebe ---- */
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
  // Anti-aliasing p/ amostragem a 100 Hz (Nyquist 50 Hz): o filtro interno do
  // acelerômetro precisa cortar abaixo de 50 Hz. São duas chamadas porque
  // nenhuma delas sozinha cobre os dois chips:
  mpu.setAccelLPF(41);   // MPU6500: escreve ACCEL_CONFIG2 -> 41 Hz.
                         // MPU6050: o FastIMU não implementa, devolve -1 sem fazer nada.
  mpu.setGyroLPF(42);    // MPU6050: o DLPF é COMPARTILHADO, então é ESTA linha que
                         // filtra o acelerômetro (44 Hz). MPU6500: só o giroscópio.

  Serial.println("Mantenha o sensor parado e nivelado para calibrar...");
  delay(2000);
  mpu.calibrateAccelGyro(&calib);   // habilite no ESP32 físico; trava no Wokwi (FIFO ausente)
  mpu.init(calib, 0x68);

  Serial.println("MPU iniciado");

  pinMode(LED_PIN,     OUTPUT);
  pinMode(LED_ONBOARD, OUTPUT);
  digitalWrite(LED_PIN,     LOW);
  digitalWrite(LED_ONBOARD, LOW);

  conectarWiFi();
  sincronizarRelogio();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(receberComando);   // é aqui que a resposta da nuvem entra
  mqttClient.setKeepAlive(60);
  mqttClient.setSocketTimeout(30);
  mqttClient.setBufferSize(512);

  Serial.println("Sistema pronto. Uma janela por segundo, sem botão nenhum.");
  Serial.printf("  Publica em: %s\r\n", MQTT_PUB_TOPIC);
  Serial.printf("  Escuta em:  %s\r\n", MQTT_SUB_TOPIC);
  Serial.println("  LED aceso fixo = ligado_normal | LED piscando = ligado_anomalia");
  Serial.println("  LED apagado = a nuvem ainda não respondeu\r\n");
}

/* ============================== LOOP =============================== */
void loop() {
  if (!mqttClient.connected()) {
    conectarMQTT();
  }
  mqttClient.loop();

  digitalWrite(LED_ONBOARD, mqttClient.connected() ? HIGH : LOW);
  atualizarLed();

  // --- Coleta IMU a 100 Hz, direto, sem esperar comando ---
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
      uint64_t ts_epoch_ms = agoraEpochMs();

      float mx   = calcMean(ax_buf, TAMANHO_JANELA);
      float my   = calcMean(ay_buf, TAMANHO_JANELA);
      float mz   = calcMean(az_buf, TAMANHO_JANELA);
      float sx   = calcStd(ax_buf,  TAMANHO_JANELA);
      float sy   = calcStd(ay_buf,  TAMANHO_JANELA);
      float sz   = calcStd(az_buf,  TAMANHO_JANELA);
      float rmag = calcRMSMagnitude(ax_buf, ay_buf, az_buf, TAMANHO_JANELA);

      publicarFeatures(ts_epoch_ms, mx, my, mz, sx, sy, sz, rmag);

      indice = 0;
    }
  }
}

/* ---- LED externo: mostra o que a NUVEM respondeu ---- */
void atualizarLed() {
  if (classePrevista == "ligado_anomalia") {
    if (millis() - ultimoPiscaLed >= PISCA_MS) {
      ultimoPiscaLed = millis();
      ledAceso = !ledAceso;
      digitalWrite(LED_PIN, ledAceso ? HIGH : LOW);
    }
  } else if (classePrevista == "ligado_normal") {
    digitalWrite(LED_PIN, HIGH);
    ledAceso = true;
  } else {
    digitalWrite(LED_PIN, LOW);   // ainda sem resposta
    ledAceso = false;
  }
}

/* ---- WiFi ---- */
void conectarWiFi() {
  Serial.printf("Conectando ao WiFi %s", WIFI_SSID);
  // TxPower reduzido: evita brownout/reboot ao ligar o rádio nesta placa.
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
    Serial.println(" FALHOU. ts_epoch_ms vira uptime.");
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
      mqttClient.subscribe(MQTT_SUB_TOPIC);
      Serial.printf("Inscrito em: %s\r\n", MQTT_SUB_TOPIC);
    } else {
      Serial.printf(" Falha rc=%d. Tentando em 5s...\r\n", mqttClient.state());
      delay(5000);
    }
  }
}

/* ---- A resposta da nuvem chega aqui ---- */
void receberComando(char* topico, byte* conteudo, unsigned int tamanho) {
  classePrevista = "";
  for (unsigned int i = 0; i < tamanho; i++) classePrevista += (char)conteudo[i];
  classePrevista.trim();

  Serial.printf("MODELO: %s\r\n", classePrevista.c_str());
}

/* ---- Publica a janela: só o que o modelo precisa ---- */
void publicarFeatures(uint64_t ts_epoch_ms,
                      float mx, float my, float mz,
                      float sx, float sy, float sz, float rmag) {
  JsonDocument doc;
  doc["device"]      = MQTT_CLIENT_ID;
  doc["ts_epoch_ms"] = ts_epoch_ms;
  doc["mean_ax"] = serialized(String(mx, 3));
  doc["mean_ay"] = serialized(String(my, 3));
  doc["mean_az"] = serialized(String(mz, 3));
  doc["std_ax"]  = serialized(String(sx, 3));
  doc["std_ay"]  = serialized(String(sy, 3));
  doc["std_az"]  = serialized(String(sz, 3));
  doc["rms_mag"] = serialized(String(rmag, 3));

  String buffer;
  serializeJson(doc, buffer);

  if (!mqttClient.publish(MQTT_PUB_TOPIC, buffer.c_str())) {
    Serial.println("MQTT: falha no envio");
  }
}
