/* app17-11-motormulticlasse — features de aceleração por janela (2 s @ 500 Hz), 5 classes cíclicas, publicadas via MQTT (JSON). */
/*
PARA USAR NO WOKWI:
- Não há equivalente físico fiel no simulador para as classes de inclinação
  (gabarito 3D); este app foi pensado para hardware físico.
- Ajustar #define MPU_TYPE (padrão MPU6050) e remover/comentar
  `mpu.calibrateAccelGyro(&calib);` (trava no Wokwi, FIFO ausente).
*/

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

/* ---- Pinos ---- */
#define SDA_PIN      22
#define SCL_PIN      23
#define BTN_COLETA   21   // inicia/para a coleta
#define BTN_CLASSE   18   // avança para a próxima classe (só com a coleta parada)
#define LED_PIN       4   // LED externo: aceso = coleta em andamento
#define LED_ONBOARD   2   // LED onboard: aceso = coleta em andamento

/* ---- Sensor (MPU6050 ou MPU6500) ---- */
#define MPU_TYPE MPU6050
MPU_TYPE mpu;

calData calib = { 0 };

/* ---- Sequência cíclica de classes ---- */
const char* SEQUENCIA[] = { "desligado", "operando", "inclinado_frente",
                            "inclinado_tras", "anomalia" };
const int   N_CLASSES = 5;

int indiceClasse = 0;   // posição atual na sequência (0..N_CLASSES-1)
int rodada        = 1;  // incrementa a cada volta completa pela sequência (group do LeaveOneGroupOut)

bool coletando = false;

int ultimoBotaoColeta = HIGH;
int ultimoBotaoClasse = HIGH;
unsigned long ultimoDebounceColeta = 0;
unsigned long ultimoDebounceClasse = 0;
const unsigned long debounceMs = 300;

/* ---- Amostragem: 500 Hz, janela de 2 s ---- */
const int FS_HZ          = 500;
const int AMOSTRA_MS     = 1000 / FS_HZ;      // 2 ms
const int TAMANHO_JANELA = FS_HZ * 2;         // 1000 amostras = 2 s

float ax_buf[TAMANHO_JANELA];
float ay_buf[TAMANHO_JANELA];
float az_buf[TAMANHO_JANELA];
float mag_buf[TAMANHO_JANELA];

int indice = 0;
uint32_t tempoAnterior = 0;
uint32_t millisInicioJanela = 0;
int janelaAtual = 0;   // conta janelas desde que a coleta desta classe começou

/* ---- Protótipos ---- */
void conectarWiFi();
void conectarMQTT();
void avancarClasse();
void imprimirClasseAtual();
void publicarJanela(int janelaIdx, float fsReal,
                    float mx, float my, float mz,
                    float sx, float sy, float sz, float rmag,
                    float stdMag, float p2p, float crest, float kurt, float zcr);

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

float calcCrest(float arr[], int n, float media, float desvio) {
  if (desvio <= 0) return 0.0;
  float pico = 0;
  for (int i = 0; i < n; i++) {
    float d = fabs(arr[i] - media);
    if (d > pico) pico = d;
  }
  return pico / desvio;
}

// Curtose populacional (viesada, ddof=0) -- só precisa ser consistente
// consigo mesma (o notebook 2.6 lê a feature pronta, não recalcula).
float calcKurtosis(float arr[], int n, float media, float desvio) {
  if (desvio <= 0) return 0.0;
  float soma4 = 0;
  for (int i = 0; i < n; i++) {
    float d = arr[i] - media;
    soma4 += d * d * d * d;
  }
  float m4 = soma4 / n;
  return (m4 / (desvio * desvio * desvio * desvio)) - 3.0;
}

// Taxa de cruzamento por zero da componente AC (arr - media).
float calcZCR(float arr[], int n, float media) {
  int cruzamentos = 0;
  for (int i = 1; i < n; i++) {
    bool sinalAnterior = (arr[i-1] - media) >= 0;
    bool sinalAtual    = (arr[i]   - media) >= 0;
    if (sinalAnterior != sinalAtual) cruzamentos++;
  }
  return (float)cruzamentos / (n - 1);
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
  // Sem filtro anti-aliasing (DLPF) de propósito: a banda cheia do sensor
  // fica disponível para capturar o máximo de conteúdo do sinal. Trade-off:
  // energia acima de Nyquist (FS_HZ/2) volta rebatida (aliasing) pra dentro
  // da janela -- pra features estatísticas (RMS/std/etc.) isso costuma ser
  // aceitável, mas não é "mais informação limpa", é mais energia (parte
  // dela distorcida) entrando na janela.
  mpu.setAccelODR(FS_HZ); // acompanha a taxa de leitura, evita amostra duplicada

  Serial.println("Mantenha o sensor parado e nivelado para calibrar...");
  delay(2000);
  mpu.calibrateAccelGyro(&calib);   // habilite no ESP32 físico; trava no Wokwi (FIFO ausente)
  mpu.init(calib, 0x68);

  Serial.println("MPU iniciado");

  pinMode(BTN_COLETA,  INPUT_PULLUP);
  pinMode(BTN_CLASSE,  INPUT_PULLUP);
  pinMode(LED_PIN,     OUTPUT);
  pinMode(LED_ONBOARD, OUTPUT);
  digitalWrite(LED_PIN,     LOW);
  digitalWrite(LED_ONBOARD, LOW);

  conectarWiFi();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setKeepAlive(60);
  mqttClient.setSocketTimeout(30);
  mqttClient.setBufferSize(512);

  Serial.println("Sistema pronto.");
  Serial.println("  Botão 21: inicia/para a coleta");
  Serial.println("  Botão 18: avança para a próxima classe (só com a coleta parada)");
  Serial.printf("  Tópico MQTT: %s\r\n\r\n", MQTT_PUB_TOPIC);
  Serial.println("classe,rodada,janela,fs_real,mean_ax,mean_ay,mean_az,std_ax,std_ay,std_az,"
                 "rms_mag,std_mag,p2p_mag,crest_mag,kurt_mag,zcr_mag");
  imprimirClasseAtual();
}

/* ============================== LOOP =============================== */
void loop() {
  conectarMQTT();
  mqttClient.loop();

  // --- Botão coleta (inicia/para) ---
  int leituraColeta = digitalRead(BTN_COLETA);
  if (ultimoBotaoColeta == HIGH && leituraColeta == LOW) {
    if (millis() - ultimoDebounceColeta > debounceMs) {
      coletando = !coletando;
      if (coletando) {
        indice = 0;
        janelaAtual = 0;
        tempoAnterior = millis();
        millisInicioJanela = millis();
        digitalWrite(LED_PIN,     HIGH);
        digitalWrite(LED_ONBOARD, HIGH);
        Serial.printf("Coleta INICIADA (classe: %s)\r\n", SEQUENCIA[indiceClasse]);
      } else {
        digitalWrite(LED_PIN,     LOW);
        digitalWrite(LED_ONBOARD, LOW);
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
        Serial.println("Pare a coleta (botão 21) antes de trocar a classe.");
      }
      ultimoDebounceClasse = millis();
    }
  }
  ultimoBotaoClasse = leituraClasse;

  if (!coletando) return;

  // --- Coleta IMU a FS_HZ ---
  if (millis() - tempoAnterior >= AMOSTRA_MS) {
    // Avança em passos fixos de AMOSTRA_MS (e não "= millis()"): assim o
    // atraso de um ciclo não empurra o próximo e a taxa não escorrega.
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
      float fsReal = TAMANHO_JANELA * 1000.0f / (millis() - millisInicioJanela);

      float mx = calcMean(ax_buf, TAMANHO_JANELA);
      float my = calcMean(ay_buf, TAMANHO_JANELA);
      float mz = calcMean(az_buf, TAMANHO_JANELA);
      float sx = calcStd(ax_buf, TAMANHO_JANELA, mx);
      float sy = calcStd(ay_buf, TAMANHO_JANELA, my);
      float sz = calcStd(az_buf, TAMANHO_JANELA, mz);
      float rmag = calcRMSMagnitude(ax_buf, ay_buf, az_buf, TAMANHO_JANELA);

      calcMagnitude(ax_buf, ay_buf, az_buf, mag_buf, TAMANHO_JANELA);
      float mMag = calcMean(mag_buf, TAMANHO_JANELA);
      float stdMag = calcStd(mag_buf, TAMANHO_JANELA, mMag);
      float p2p = calcPtP(mag_buf, TAMANHO_JANELA);
      float crest = calcCrest(mag_buf, TAMANHO_JANELA, mMag, stdMag);
      float kurt = calcKurtosis(mag_buf, TAMANHO_JANELA, mMag, stdMag);
      float zcr = calcZCR(mag_buf, TAMANHO_JANELA, mMag);

      janelaAtual++;

      Serial.printf("%s,%02d,%d,%.1f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\r\n",
                    SEQUENCIA[indiceClasse], rodada, janelaAtual, fsReal,
                    mx, my, mz, sx, sy, sz, rmag,
                    stdMag, p2p, crest, kurt, zcr);

      publicarJanela(janelaAtual, fsReal, mx, my, mz, sx, sy, sz, rmag,
                    stdMag, p2p, crest, kurt, zcr);

      indice = 0;
      millisInicioJanela = millis();
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
  imprimirClasseAtual();
}

void imprimirClasseAtual() {
  Serial.printf("Classe selecionada: %s (rodada %02d)\r\n", SEQUENCIA[indiceClasse], rodada);
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

void publicarJanela(int janelaIdx, float fsReal,
                    float mx, float my, float mz,
                    float sx, float sy, float sz, float rmag,
                    float stdMag, float p2p, float crest, float kurt, float zcr) {
  JsonDocument doc;
  doc["device"]    = MQTT_CLIENT_ID;
  doc["classe"]    = SEQUENCIA[indiceClasse];
  doc["rodada"]    = rodada;
  doc["janela"]    = janelaIdx;
  doc["fs_real"]   = serialized(String(fsReal, 1));
  doc["mean_ax"]   = serialized(String(mx, 3));
  doc["mean_ay"]   = serialized(String(my, 3));
  doc["mean_az"]   = serialized(String(mz, 3));
  doc["std_ax"]    = serialized(String(sx, 3));
  doc["std_ay"]    = serialized(String(sy, 3));
  doc["std_az"]    = serialized(String(sz, 3));
  doc["rms_mag"]   = serialized(String(rmag, 3));
  doc["std_mag"]   = serialized(String(stdMag, 3));
  doc["p2p_mag"]   = serialized(String(p2p, 3));
  doc["crest_mag"] = serialized(String(crest, 3));
  doc["kurt_mag"]  = serialized(String(kurt, 3));
  doc["zcr_mag"]   = serialized(String(zcr, 3));

  String buffer;
  serializeJson(doc, buffer);

  Serial.print("PAYLOAD MQTT: ");
  Serial.println(buffer.c_str());

  bool ok = mqttClient.publish(MQTT_PUB_TOPIC, buffer.c_str());
  Serial.println(ok ? "MQTT: enviado com sucesso" : "MQTT: falha no envio");
}
