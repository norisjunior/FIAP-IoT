/* app17-10-motormulticlasse — features de aceleração por janela (1 s @ 100 Hz), 4 classes, publicadas via MQTT (JSON). ESP32-C6. */

#include <Arduino.h>
#include "FastIMU.h"
#include <Wire.h>
#include <math.h>
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
#define MQTT_PUB_TOPIC "FIAPIoT/motor/multiclasse"
#define MQTT_CLIENT_ID "IoTDevMultiClasse001"
PubSubClient mqttClient(wifiClient);

/* ---- Pinos (ESP32-C6) ---- */
#define SDA_PIN       5
#define SCL_PIN       4
#define BTN_COLETA   12   // inicia/para a coleta
#define BTN_CLASSE   13   // avança para a próxima classe (só com a coleta parada)
#define LED_PIN       7   // LED externo: índice da classe
#define LED_RGB       8   // LED RGB da placa

/* ---- Sensor (MPU6050 ou MPU6500) ---- */
#define MPU_TYPE MPU6500 // ou MPU6500 (WHO_AM_I = 0x70)
MPU_TYPE mpu;

calData calib = { 0 };

/* ---- Sequência de classes ---- */
const char* SEQUENCIA[] = { "operando", "inclinado_frente", "inclinado_tras", "anomalia" };
const int   N_CLASSES = 4;

// Cor do LED RGB (R, G, B): branco esperando, a cor da classe na coleta
const uint8_t COR_ESPERA[3] = { 40, 40, 40 };   // branco
const uint8_t COR_CLASSE[N_CLASSES][3] = {
  {  0, 40,  0 },   // operando: verde
  { 40, 30,  0 },   // inclinado_frente: amarelo
  { 25,  0, 40 },   // inclinado_tras: roxo
  { 40,  0,  0 },   // anomalia: vermelho
};

int indiceClasse = 0;   // posição na sequência
int rodada        = 1;  // volta completa pela sequência

bool coletando = false;
const int META_JANELAS = 30;   // janelas por classe

int ultimoBotaoColeta = HIGH;
int ultimoBotaoClasse = HIGH;
unsigned long ultimoDebounceColeta = 0;
unsigned long ultimoDebounceClasse = 0;
const unsigned long debounceMs = 300;

/* ---- LED de classe ---- */
const uint32_t LED_ON_MS    = 150;
const uint32_t LED_OFF_MS   = 200;
const uint32_t LED_PAUSA_MS = 1200;

int      ledPiscadasFeitas = 0;
bool     ledClasseAceso    = false;
uint32_t ledUltimaMudanca  = 0;

/* ---- Amostragem: 100 Hz, janela de 1 s ---- */
const int FS_HZ          = 100;
const int AMOSTRA_MS     = 1000 / FS_HZ;      // 10 ms
const int TAMANHO_JANELA = FS_HZ;             // 100 amostras = 1 s

float ax_buf[TAMANHO_JANELA];
float ay_buf[TAMANHO_JANELA];
float az_buf[TAMANHO_JANELA];
float mag_buf[TAMANHO_JANELA];

int indice = 0;
uint32_t tempoAnterior = 0;
int janelaAtual = 0;   // janelas da classe atual

/* ---- Protótipos ---- */
void conectarWiFi();
void conectarMQTT();
void avancarClasse();
void imprimirClasseAtual();
void atualizarLedClasse();
void atualizarLed();
void publicarJanela(const char* label, unsigned long ts_ms, int rodadaAtual, int janelaIdx,
                    float mx, float my, float mz,
                    float sx, float sy, float sz,
                    float stdMag, float p2p);

/* =========================== Features =========================== */
float calcMean(float arr[], int n) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += arr[i];
  return soma / n;
}

float calcStd(float arr[], int n, float media) {
  float soma = 0;
  for (int i = 0; i < n; i++) soma += (arr[i] - media) * (arr[i] - media);
  return sqrt(soma / n);
}

void calcMagnitude(float axArr[], float ayArr[], float azArr[], float magArr[], int n) {
  for (int i = 0; i < n; i++) {
    magArr[i] = sqrt(axArr[i]*axArr[i] + ayArr[i]*ayArr[i] + azArr[i]*azArr[i]);
  }
}

float calcPtP(float arr[], int n) {
  float mn = arr[0], mx = arr[0];
  for (int i = 1; i < n; i++) {
    if (arr[i] < mn) mn = arr[i];
    if (arr[i] > mx) mx = arr[i];
  }
  return mx - mn;
}

/* ============================== SETUP ============================== */
void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);
  if (mpu.init(calib, 0x68) != 0) {
    Serial.println("Erro: MPU não encontrado");
    while (1);
  }

  Serial.println("Deixe o motor na posicao inicial/de uso e nao o movimente durante a calibracao...");
  delay(2000);
  // Calibração (no Wokwi, comente esta linha)
  mpu.calibrateAccelGyro(&calib);
  mpu.init(calib, 0x68);
  // Fundo de escala ±8 g
  mpu.setAccelRange(8);

  Serial.println("MPU iniciado");

  pinMode(BTN_COLETA,  INPUT_PULLUP);
  pinMode(BTN_CLASSE,  INPUT_PULLUP);
  pinMode(LED_PIN,     OUTPUT);
  digitalWrite(LED_PIN,     LOW);
  ledUltimaMudanca = millis();
  atualizarLed();

  conectarWiFi();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setKeepAlive(60);
  mqttClient.setSocketTimeout(30);
  mqttClient.setBufferSize(512);

  Serial.println("Sistema pronto.");
  Serial.println("  Botão 12: inicia/para a coleta");
  Serial.println("  Botão 13: avança para a próxima classe (só com a coleta parada)");
  Serial.println("  A coleta para sozinha ao completar 30 janelas da classe atual.");
  Serial.println("  LED RGB: branco = esperando | na coleta: verde operando, amarelo frente, roxo tras, vermelho anomalia");
  Serial.println("  LED externo: pisca N vezes = índice da classe atual (1..4), em loop");
  Serial.printf("  Tópico MQTT: %s\r\n\r\n", MQTT_PUB_TOPIC);
  Serial.println("ts_ms,label,rodada,janela,mean_ax,mean_ay,mean_az,std_ax,std_ay,std_az,std_mag,p2p_mag");
  imprimirClasseAtual();
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
        atualizarLed();
        Serial.printf("Coleta INICIADA (classe: %s)\r\n", SEQUENCIA[indiceClasse]);
      } else {
        atualizarLed();
        Serial.println("Coleta PARADA");
      }
      ultimoDebounceColeta = millis();
    }
  }
  ultimoBotaoColeta = leituraColeta;

  // --- Botão classe (avança na sequência, só com a coleta parada) ---
  int leituraClasse = digitalRead(BTN_CLASSE);
  if (ultimoBotaoClasse == HIGH && leituraClasse == LOW) {
    if (millis() - ultimoDebounceClasse > debounceMs) {
      if (!coletando) {
        avancarClasse();
      } else {
        Serial.println("Pare a coleta (botão 12) antes de trocar a classe.");
      }
      ultimoDebounceClasse = millis();
    }
  }
  ultimoBotaoClasse = leituraClasse;

  // --- LED de classe ---
  atualizarLedClasse();

  if (!coletando) return;

  // --- Coleta IMU a 100 Hz ---
  if (millis() - tempoAnterior >= AMOSTRA_MS) {
    tempoAnterior += AMOSTRA_MS;
    if (millis() - tempoAnterior >= AMOSTRA_MS) tempoAnterior = millis();

    AccelData accel;
    mpu.update();
    mpu.getAccel(&accel);
    ax_buf[indice] = accel.accelX;
    ay_buf[indice] = accel.accelY;
    az_buf[indice] = accel.accelZ;
    indice++;

    if (indice >= TAMANHO_JANELA) {
      const char* label = SEQUENCIA[indiceClasse];
      unsigned long ts_ms = millis();   // ms desde o boot

      float mx = calcMean(ax_buf, TAMANHO_JANELA);
      float my = calcMean(ay_buf, TAMANHO_JANELA);
      float mz = calcMean(az_buf, TAMANHO_JANELA);
      float sx = calcStd(ax_buf, TAMANHO_JANELA, mx);
      float sy = calcStd(ay_buf, TAMANHO_JANELA, my);
      float sz = calcStd(az_buf, TAMANHO_JANELA, mz);

      calcMagnitude(ax_buf, ay_buf, az_buf, mag_buf, TAMANHO_JANELA);
      float mMag   = calcMean(mag_buf, TAMANHO_JANELA);
      float stdMag = calcStd(mag_buf, TAMANHO_JANELA, mMag);
      float p2p    = calcPtP(mag_buf, TAMANHO_JANELA);

      janelaAtual++;

      Serial.printf("%lu,%s,%02d,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\r\n",
                    ts_ms, label, rodada, janelaAtual,
                    mx, my, mz, sx, sy, sz, stdMag, p2p);

      publicarJanela(label, ts_ms, rodada, janelaAtual,
                    mx, my, mz, sx, sy, sz, stdMag, p2p);

      indice = 0;

      if (janelaAtual >= META_JANELAS) {
        coletando = false;
        atualizarLed();
        Serial.printf(">>> %d janelas coletadas para %s (rodada %02d). Reposicione o sensor e aperte o botao 13 para a proxima classe.\r\n",
                      META_JANELAS, label, rodada);
      }
    }
  }
}

/* =========================== Sequência =============================== */
void avancarClasse() {
  indiceClasse++;
  if (indiceClasse >= N_CLASSES) {
    indiceClasse = 0;
    rodada++;
  }
  janelaAtual = 0;

  // Reinicia o LED de classe
  ledPiscadasFeitas = 0;
  ledClasseAceso    = false;
  digitalWrite(LED_PIN, LOW);
  ledUltimaMudanca = millis();

  imprimirClasseAtual();
}

void imprimirClasseAtual() {
  Serial.printf("Classe selecionada: %s (rodada %02d)\r\n", SEQUENCIA[indiceClasse], rodada);
}

/* ---- LED RGB ---- */
void atualizarLed() {
  const uint8_t* cor = coletando ? COR_CLASSE[indiceClasse] : COR_ESPERA;
  rgbLedWrite(LED_RGB, cor[0], cor[1], cor[2]);
}

/* ---- LED de classe: N piscadas e pausa ---- */
void atualizarLedClasse() {
  int totalPiscadas = indiceClasse + 1;
  uint32_t agora = millis();

  if (ledClasseAceso) {
    if (agora - ledUltimaMudanca >= LED_ON_MS) {
      digitalWrite(LED_PIN, LOW);
      ledClasseAceso = false;
      ledUltimaMudanca = agora;
      ledPiscadasFeitas++;
    }
  } else {
    uint32_t intervalo = (ledPiscadasFeitas >= totalPiscadas) ? LED_PAUSA_MS : LED_OFF_MS;
    if (agora - ledUltimaMudanca >= intervalo) {
      if (ledPiscadasFeitas >= totalPiscadas) ledPiscadasFeitas = 0;
      digitalWrite(LED_PIN, HIGH);
      ledClasseAceso = true;
      ledUltimaMudanca = agora;
    }
  }
}

/* ---- WiFi ---- */
void conectarWiFi() {
  Serial.printf("Conectando ao WiFi %s", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_2dBm);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setSleep(false);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println("");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
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

void publicarJanela(const char* label, unsigned long ts_ms, int rodadaAtual, int janelaIdx,
                    float mx, float my, float mz,
                    float sx, float sy, float sz,
                    float stdMag, float p2p) {
  JsonDocument doc;
  doc["device"] = MQTT_CLIENT_ID;
  doc["label"]  = label;
  doc["rodada"] = rodadaAtual;
  doc["janela"] = janelaIdx;
  doc["ts_ms"]  = ts_ms;
  doc["mean_ax"] = serialized(String(mx, 3));
  doc["mean_ay"] = serialized(String(my, 3));
  doc["mean_az"] = serialized(String(mz, 3));
  doc["std_ax"]  = serialized(String(sx, 3));
  doc["std_ay"]  = serialized(String(sy, 3));
  doc["std_az"]  = serialized(String(sz, 3));
  doc["std_mag"] = serialized(String(stdMag, 3));
  doc["p2p_mag"] = serialized(String(p2p, 3));

  String buffer;
  serializeJson(doc, buffer);

  Serial.print("PAYLOAD MQTT: ");
  Serial.println(buffer.c_str());

  bool ok = mqttClient.publish(MQTT_PUB_TOPIC, buffer.c_str());
  Serial.println(ok ? "MQTT: enviado com sucesso" : "MQTT: falha no envio");
}
