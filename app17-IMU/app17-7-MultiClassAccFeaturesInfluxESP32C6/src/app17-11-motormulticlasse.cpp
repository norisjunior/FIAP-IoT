/* app17-11-motormulticlasse — features de aceleração por janela (1 s @ 100 Hz), 4 classes
   cíclicas (motor sempre ligado), publicadas via MQTT (JSON). Evolução multiclasse do
   app17-6 (binário: parado vs anomalia). */
/*
ALVO: ESP32-C6 (ex.: ESP32-C6-DevKitC-1). Requer o core Arduino-ESP32 3.x —
ver platformio.ini (platform `pioarduino`). Os GPIOs 4/5/7/12/13 usados aqui
sao os mesmos do app17-6; o "LED onboard" da DevKitC-1 e o LED RGB (GPIO8),
acionado via RGB_BUILTIN.

PARA USAR NO WOKWI:
- Ajustar as credenciais WiFi e o IP do MQTT_SERVER (ou usar as linhas comentadas do Wokwi abaixo)
- Ajustar #define MPU_TYPE:
  - #define MPU_TYPE MPU6050
- Remover/comentar a linha `mpu.calibrateAccelGyro(&calib);` (trava no Wokwi, FIFO ausente)
- As classes de inclinação não têm equivalente fiel no simulador (não há como
  inclinar o MPU6050 do Wokwi); serve para testar botões, LED e publicação MQTT.
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
#define MQTT_PUB_TOPIC "FIAPIoT/motor/multiclasse"
#define MQTT_CLIENT_ID "IoTDevMultiClasse001"
PubSubClient mqttClient(wifiClient);

/* ---- Relógio (NTP) ---- */
const char* NTP_SERVER_1 = "pool.ntp.org";
const char* NTP_SERVER_2 = "time.google.com";
uint64_t epochBaseMs        = 0;
uint32_t millisNaSync       = 0;
bool     relogioSincronizado = false;

/* ---- Pinos (ESP32-C6, mesmos do app17-6) ---- */
#define SDA_PIN       5
#define SCL_PIN       4
#define BTN_COLETA   12   // inicia/para a coleta
#define BTN_CLASSE   13   // avança para a próxima classe (só com a coleta parada)
#define LED_PIN       7   // LED externo: pisca N vezes = índice da classe atual

// "LED onboard": na ESP32-C6-DevKitC-1 é o LED RGB WS2812 (GPIO8). O core 3.x
// intercepta digitalWrite(RGB_BUILTIN, HIGH/LOW) e acende/apaga o RGB em branco.
// Só é escrito quando o estado da coleta muda (nunca a cada iteração do loop):
// a rotina do WS2812 desabilita IRQ e atrasaria a amostragem.
#ifdef RGB_BUILTIN
  #define LED_ONBOARD RGB_BUILTIN
#else
  #define LED_ONBOARD 8
#endif

/* ---- Sensor (MPU6050 ou MPU6500) ---- */
#define MPU_TYPE MPU6050
MPU_TYPE mpu;

calData calib = { 0 };

/* ---- Sequência cíclica de classes (motor sempre ligado, só muda a postura/intensidade) ---- */
const char* SEQUENCIA[] = { "operando", "inclinado_frente", "inclinado_tras", "anomalia" };
const int   N_CLASSES = 4;

int indiceClasse = 0;   // posição atual na sequência (0..N_CLASSES-1)
int rodada        = 1;  // incrementa a cada volta completa pela sequência (group do LeaveOneGroupOut)

bool coletando = false;
const int META_JANELAS = 30;   // a coleta para sozinha ao atingir esta contagem

int ultimoBotaoColeta = HIGH;
int ultimoBotaoClasse = HIGH;
unsigned long ultimoDebounceColeta = 0;
unsigned long ultimoDebounceClasse = 0;
const unsigned long debounceMs = 300;

/* ---- LED de classe: pisca N vezes = índice da classe (1..N_CLASSES), em loop contínuo,
        independente da coleta estar rodando ou não ---- */
const uint32_t LED_ON_MS    = 150;
const uint32_t LED_OFF_MS   = 200;
const uint32_t LED_PAUSA_MS = 1200;

int      ledPiscadasFeitas = 0;
bool     ledClasseAceso    = false;
uint32_t ledUltimaMudanca  = 0;

/* ---- Amostragem: 100 Hz, janela de 1 s (mesmo padrão do app17-6) ---- */
const int FS_HZ          = 100;
const int AMOSTRA_MS     = 1000 / FS_HZ;      // 10 ms
const int TAMANHO_JANELA = FS_HZ;             // 100 amostras = 1 s

float ax_buf[TAMANHO_JANELA];
float ay_buf[TAMANHO_JANELA];
float az_buf[TAMANHO_JANELA];
float mag_buf[TAMANHO_JANELA];

int indice = 0;
uint32_t tempoAnterior = 0;
int janelaAtual = 0;   // conta janelas desde que a coleta desta classe começou

/* ---- Protótipos ---- */
void conectarWiFi();
void sincronizarRelogio();
uint64_t agoraEpochMs();
void conectarMQTT();
void avancarClasse();
void imprimirClasseAtual();
void atualizarLedClasse();
void publicarJanela(const char* label, uint64_t ts_epoch_ms, int rodadaAtual, int janelaIdx,
                    float mx, float my, float mz,
                    float sx, float sy, float sz,
                    float stdMag, float p2p);

/* =========================== Features ===========================
   mean_ax/ay/az    -> orientação (projeção da gravidade nos 3 eixos): separa
                       inclinado_frente de inclinado_tras.
   std_ax/ay/az     -> vibração por eixo: separa operando de anomalia.
   std_mag          -> vibração total, invariante à orientação do sensor.
   p2p_mag          -> pior caso da janela (máx-mín da magnitude); complementa
                       std_mag, pois um impacto isolado move o p2p sem mover
                       muito o desvio-padrão.
   Não entram aqui: rms_* (rms² = mean² + std², não traz informação nova —
   ver app17-6), crest_mag (≈ razão entre p2p_mag e std_mag, o modelo já
   consegue combiná-las), kurt_mag/zcr_mag (conteúdo de alta frequência que
   100 Hz não enxerga) e fs_real (instrumentação de dataset, não feature). */
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
  mpu.setAccelRange(8);
  // Anti-aliasing p/ amostragem a 100 Hz (Nyquist 50 Hz): o filtro interno do
  // acelerômetro precisa cortar abaixo de 50 Hz. São duas chamadas porque
  // nenhuma delas sozinha cobre os dois chips:
  mpu.setAccelLPF(41);   // MPU6500: escreve ACCEL_CONFIG2 -> 41 Hz.
                         // MPU6050: o FastIMU não implementa, devolve -1 sem fazer nada.
  mpu.setGyroLPF(42);    // MPU6050: o DLPF é COMPARTILHADO, então é ESTA linha que
                         // filtra o acelerômetro (44 Hz). MPU6500: só o giroscópio.

  Serial.println("Deixe o motor na posicao inicial/de uso e nao o movimente durante a calibracao...");
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
  ledUltimaMudanca = millis();

  conectarWiFi();
  sincronizarRelogio();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setKeepAlive(60);
  mqttClient.setSocketTimeout(30);
  mqttClient.setBufferSize(512);

  Serial.println("Sistema pronto.");
  Serial.println("  Botão 12: inicia/para a coleta");
  Serial.println("  Botão 13: avança para a próxima classe (só com a coleta parada)");
  Serial.println("  A coleta para sozinha ao completar 30 janelas da classe atual.");
  Serial.println("  LED onboard aceso = coleta em andamento");
  Serial.println("  LED externo: pisca N vezes = índice da classe atual (1..4), em loop");
  Serial.printf("  Tópico MQTT: %s\r\n\r\n", MQTT_PUB_TOPIC);
  Serial.println("ts_epoch_ms,label,rodada,janela,mean_ax,mean_ay,mean_az,std_ax,std_ay,std_az,std_mag,p2p_mag");
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
        digitalWrite(LED_ONBOARD, HIGH);
        Serial.printf("Coleta INICIADA (classe: %s)\r\n", SEQUENCIA[indiceClasse]);
      } else {
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
        Serial.println("Pare a coleta (botão 12) antes de trocar a classe.");
      }
      ultimoDebounceClasse = millis();
    }
  }
  ultimoBotaoClasse = leituraClasse;

  // --- LED externo: pisca N vezes = índice da classe (roda sempre, coletando ou não) ---
  atualizarLedClasse();

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
      const char* label = SEQUENCIA[indiceClasse];
      uint64_t ts_epoch_ms = agoraEpochMs();

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

      Serial.printf("%llu,%s,%02d,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\r\n",
                    (unsigned long long)ts_epoch_ms, label, rodada, janelaAtual,
                    mx, my, mz, sx, sy, sz, stdMag, p2p);

      publicarJanela(label, ts_epoch_ms, rodada, janelaAtual,
                    mx, my, mz, sx, sy, sz, stdMag, p2p);

      indice = 0;

      if (janelaAtual >= META_JANELAS) {
        coletando = false;
        digitalWrite(LED_ONBOARD, LOW);
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

  // Reinicia o padrão de piscadas do zero para a nova classe.
  ledPiscadasFeitas = 0;
  ledClasseAceso    = false;
  digitalWrite(LED_PIN, LOW);
  ledUltimaMudanca = millis();

  imprimirClasseAtual();
}

void imprimirClasseAtual() {
  Serial.printf("Classe selecionada: %s (rodada %02d)\r\n", SEQUENCIA[indiceClasse], rodada);
}

/* ---- LED de classe: N piscadas curtas + pausa longa, repetindo sempre ---- */
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

void publicarJanela(const char* label, uint64_t ts_epoch_ms, int rodadaAtual, int janelaIdx,
                    float mx, float my, float mz,
                    float sx, float sy, float sz,
                    float stdMag, float p2p) {
  JsonDocument doc;
  doc["device"]      = MQTT_CLIENT_ID;
  doc["label"]       = label;
  doc["rodada"]      = rodadaAtual;
  doc["janela"]      = janelaIdx;
  doc["ts_epoch_ms"] = ts_epoch_ms;
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
