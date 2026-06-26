/* =====================================================================
 * app17-9-edgejanelafixa.cpp  —  FORMA 1: Edge Analytics no dispositivo
 *
 * O QUE FAZ
 *   Le aceleracao (MPU6050/MPU6500) a 100 Hz, agrupa em JANELA FIXA de
 *   100 amostras (1 s), calcula as features NO PROPRIO ESP32 e publica
 *   UMA mensagem MQTT (JSON) por janela. A ponte Python grava no InfluxDB.
 *
 *   Pipeline (Aula 14): sinal -> janela -> features -> MQTT -> InfluxDB.
 *
 * MONTAGEM (minima): apenas ESP32 + MPU6050 sobre a mesa. SEM motor.
 *   - NORMAL   = MPU6050 parado sobre a mesa.
 *   - ANOMALIA = tapas na mesa durante a coleta (a mesa e o "ativo").
 *
 * DERIVADO DE app17-8-MotorML. O que muda em relacao a ele:
 *   - Timestamp REAL: sincroniza a hora pela Internet (NTP, UTC) no boot e
 *     envia "ts_epoch_ms" = base_NTP + millis() decorrido (sem RTC dedicado).
 *   - SEM motor: removidos os pinos 22/23 e as funcoes do motor.
 *   - Botoes repropositados: 26 = inicia/para a coleta; 25 = seleciona
 *     NORMAL/ANOMALIA (so com a coleta parada).
 *   - Broker e sempre o Mosquitto da IoT-platform local (nada de mosquitto publico).
 *   - A tag "coleta" (normal_01, anomalia_02...) NAO entra aqui: quem
 *     adiciona e a ponte Python, que pergunta no inicio da execucao.
 *
 * ITEM DA SPRINT
 *   Forma 1 / Sprint 4 - item 1 (base analitica): features por janela,
 *   geradas no edge. Visualizacao e modelo ficam nos notebooks 1.3 / 1.5.
 *
 * PINAGEM: SDA=19 SCL=18 | BTN_COLETA=26 | BTN_ANOMALIA=25 | LED=27.
 *   Aceleracao em m/s².
 * ===================================================================== */

#include <Arduino.h>
#include <FlixPeriph.h>
#include <MPU6500.h>
#include <Wire.h>
#include <math.h>
#include <time.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

/* ====================== Config de rede ======================
 * Use UM perfil. O broker e sempre o Mosquitto da IoT-platform.
 *
 * (A) WOKWI (extensao no VS Code) — PADRAO:
 *     WiFi 'Wokwi-GUEST' (sem senha). 'host.wokwi.internal' alcanca o
 *     broker local (o WSL2 encaminha a porta 1883 para o localhost).
 *
 * (B) ESP32 FISICO (descomente o bloco B e comente o A):
 *     sua WiFi real + IP da maquina que roda a IoT-platform (WSL/PC).
 */

// ---- (A) Wokwi (padrao) ----
const char* WIFI_SSID     = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
#define MQTT_SERVER "host.wokwi.internal"

// ---- (B) ESP32 fisico ----
// const char* WIFI_SSID     = "SUA_REDE_WIFI";
// const char* WIFI_PASSWORD = "SUA_SENHA";
// #define MQTT_SERVER "192.168.0.100"   // IP da maquina com a IoT-platform

WiFiClient wifiClient;

/* ---- Config MQTT ---- */
#define MQTT_PORT      1883
#define MQTT_PUB_TOPIC "FIAPIoT/motor/features"
#define MQTT_CLIENT_ID "IoTDeviceNorisEdgeJanelaFixa001"
PubSubClient mqttClient(wifiClient);

/* ---- Relogio (NTP) ----
 * Pegamos a hora da Internet UMA vez (NTP) e guardamos a base. Depois,
 * cada janela recebe: base_epoch + (millis() - millis_na_sync). Ou seja,
 * o timestamp e o tempo REAL da Internet, "andado" pelo millis do ESP32.
 * Sem RTC dedicado: o millis basta para o intervalo de uma aula. Em sala,
 * discutir a importancia de sincronizar relogio em sistemas distribuidos. */
const char* NTP_SERVER_1 = "pool.ntp.org";
const char* NTP_SERVER_2 = "time.google.com";
uint64_t epochBaseMs       = 0;     // epoch (ms, UTC) no instante da sincronizacao
uint32_t millisNaSync      = 0;     // millis() no instante da sincronizacao
bool     relogioSincronizado = false;

/* ---- Pinos (montagem minima, sem motor) ---- */
#define BTN_COLETA   26   // inicia/para a coleta (era o botao do motor no app17-8)
#define BTN_ANOMALIA 25   // seleciona NORMAL/ANOMALIA (so com a coleta parada)
#define LED_PIN      27   // aceso = coletando ANOMALIA
#define SDA_PIN      19
#define SCL_PIN      18

/* Sensor: o codigo de aula usa MPU6500 (FlixPeriph). Se o seu sensor for
 * o MPU6050 original (GY-521), troque a linha abaixo por:  MPU6050 mpu(Wire);
 * A interface do FlixPeriph e a mesma. Aceleracao retornada em m/s². */
MPU6500 mpu(Wire);

bool coletando     = false;   // true = enviando janelas (era "motorLigado")
bool anomaliaAtiva = false;   // false = NORMAL, true = ANOMALIA

int ultimoBotaoColeta   = HIGH;
int ultimoBotaoAnomalia = HIGH;

unsigned long ultimoDebounceColeta   = 0;
unsigned long ultimoDebounceAnomalia = 0;
const unsigned long debounceMs = 300;

const int TAMANHO_JANELA = 100;   // 100 amostras @ 100 Hz = 1 s
float ax_buf[TAMANHO_JANELA];
float ay_buf[TAMANHO_JANELA];
float az_buf[TAMANHO_JANELA];

int indice = 0;
uint32_t tempoAnterior = 0;
const int AMOSTRA_MS = 10; // 100 Hz

/* ---- Prototipos ---- */
void conectarWiFi();
void sincronizarRelogio();
uint64_t agoraEpochMs();
void conectarMQTT();
void publicarFeatures(const char* label, uint64_t ts_epoch_ms,
                      float mx, float my, float mz,
                      float sx, float sy, float sz,
                      float rx, float ry, float rz, float rmag);

/* ---- Features (iguais ao app17-8 / app17-5) ---- */
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
  if (!mpu.begin()) {
    Serial.println("Erro: MPU nao encontrado");
    while (1);
  }
  mpu.setAccelRange(IMUInterface::ACCEL_RANGE_8G);
  Serial.println("MPU iniciado");

  pinMode(BTN_COLETA,   INPUT_PULLUP);
  pinMode(BTN_ANOMALIA, INPUT_PULLUP);
  pinMode(LED_PIN,      OUTPUT);
  digitalWrite(LED_PIN, LOW);

  conectarWiFi();
  sincronizarRelogio();   // hora da Internet (NTP) -> base do ts_epoch_ms

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setKeepAlive(60);
  mqttClient.setSocketTimeout(30);
  mqttClient.setBufferSize(512);

  Serial.println("Sistema pronto. (Forma 1 - janela fixa, features no edge)");
  Serial.println("  Montagem: ESP32 + MPU6050 sobre a mesa (sem motor).");
  Serial.println("  Botao 26: inicia/para a coleta");
  Serial.println("  Botao 25: seleciona NORMAL/ANOMALIA (so com a coleta parada)");
  Serial.println("  NORMAL = MPU parado | ANOMALIA = tapas na mesa durante a coleta");
  Serial.printf("  Topico MQTT: %s\n\n", MQTT_PUB_TOPIC);
  Serial.println("ts_epoch_ms,label,mean_ax,mean_ay,mean_az,std_ax,std_ay,std_az,rms_ax,rms_ay,rms_az,rms_mag");
}

/* ============================== LOOP =============================== */
void loop() {
  if (!mqttClient.connected()) {
    conectarMQTT();
  }
  mqttClient.loop();

  // --- Botao coleta (inicia/para) ---
  int leituraColeta = digitalRead(BTN_COLETA);
  if (ultimoBotaoColeta == HIGH && leituraColeta == LOW) {
    if (millis() - ultimoDebounceColeta > debounceMs) {
      coletando = !coletando;
      if (coletando) {
        indice = 0;                 // comeca uma janela limpa
        tempoAnterior = millis();   // reinicia o timing de amostragem
        digitalWrite(LED_PIN, anomaliaAtiva ? HIGH : LOW);
        Serial.printf("Coleta INICIADA (condicao: %s)\n",
                      anomaliaAtiva ? "ANOMALIA - de tapas na mesa" : "NORMAL - deixe o MPU parado");
      } else {
        digitalWrite(LED_PIN, LOW);
        Serial.println("Coleta PARADA");
      }
      ultimoDebounceColeta = millis();
    }
  }
  ultimoBotaoColeta = leituraColeta;

  // --- Botao anomalia (seleciona condicao, so com a coleta parada) ---
  int leituraAnomalia = digitalRead(BTN_ANOMALIA);
  if (ultimoBotaoAnomalia == HIGH && leituraAnomalia == LOW) {
    if (millis() - ultimoDebounceAnomalia > debounceMs) {
      if (!coletando) {
        anomaliaAtiva = !anomaliaAtiva;
        Serial.printf("Condicao selecionada: %s\n",
                      anomaliaAtiva ? "ANOMALIA (tapas na mesa)" : "NORMAL (MPU parado)");
      } else {
        Serial.println("Pare a coleta (botao 26) antes de trocar a condicao.");
      }
      ultimoDebounceAnomalia = millis();
    }
  }
  ultimoBotaoAnomalia = leituraAnomalia;

  // Sem coleta = estado "parado": nao amostra nem envia janelas.
  if (!coletando) return;

  // --- Coleta IMU a 100 Hz ---
  if (millis() - tempoAnterior >= AMOSTRA_MS) {
    tempoAnterior = millis();
    mpu.read();
    mpu.getAccel(ax_buf[indice], ay_buf[indice], az_buf[indice]);
    indice++;

    if (indice >= TAMANHO_JANELA) {
      // Sem motor: a condicao vem do botao 25 (NORMAL/ANOMALIA).
      const char* label = anomaliaAtiva ? "ligado_anomalia" : "ligado_normal";

      uint64_t ts_epoch_ms = agoraEpochMs();   // tempo real (NTP) + millis

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

      Serial.printf("%llu,%s,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
                    (unsigned long long)ts_epoch_ms, label,
                    mx, my, mz, sx, sy, sz, rx, ry, rz, rmag);

      publicarFeatures(label, ts_epoch_ms, mx, my, mz, sx, sy, sz, rx, ry, rz, rmag);

      indice = 0;
    }
  }
}

/* =========================== FUNCOES =============================== */
void conectarWiFi() {
  Serial.printf("Conectando ao WiFi %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println("");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void sincronizarRelogio() {
  // UTC (offset 0): convertemos o fuso depois, no notebook.
  configTime(0, 0, NTP_SERVER_1, NTP_SERVER_2);
  Serial.print("Sincronizando relogio via NTP");
  struct tm tm;
  for (int i = 0; i < 16 && !relogioSincronizado; i++) {
    if (getLocalTime(&tm, 500)) relogioSincronizado = true;
    else { Serial.print('.'); delay(500); }
  }
  if (relogioSincronizado) {
    epochBaseMs  = (uint64_t)time(nullptr) * 1000ULL;  // epoch (ms) na sincronizacao
    millisNaSync = millis();
    Serial.printf(" OK (%04d-%02d-%02d %02d:%02d:%02d UTC)\n",
                  tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                  tm.tm_hour, tm.tm_min, tm.tm_sec);
  } else {
    Serial.println(" FALHOU. ts_epoch_ms vira tempo relativo (uptime); a ponte usa o horario de recepcao.");
  }
}

uint64_t agoraEpochMs() {
  // base da Internet + quanto passou desde a sincronizacao (millis do ESP32).
  // Se o NTP falhou, epochBaseMs=0 e isto vira apenas o uptime em ms.
  return epochBaseMs + (uint64_t)(millis() - millisNaSync);
}

void conectarMQTT() {
  while (!mqttClient.connected()) {
    Serial.printf("Conectando ao MQTT Broker %s...", MQTT_SERVER);
    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      Serial.println(" Conectado!");
    } else {
      Serial.printf(" Falha rc=%d. Tentando em 5s...\n", mqttClient.state());
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
  doc["ts_epoch_ms"]  = ts_epoch_ms;   // tempo real (NTP, UTC, ms) + millis do ESP32
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
