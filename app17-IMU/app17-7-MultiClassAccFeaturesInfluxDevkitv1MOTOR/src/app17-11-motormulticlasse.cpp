/* app17-11-motormulticlasse — features de aceleração por janela (1 s @ 100 Hz), 4 classes
   cíclicas (motor sempre ligado), publicadas via MQTT (JSON). Evolução multiclasse do
   app17-6 (binário: parado vs anomalia). */
/*
ALVO: ESP32 DevKit v1 (board `esp32dev`) com MOTOR REAL — mini motor 3 V acionado
por ponte H (MOTOR_INA/MOTOR_INB), montado no gabarito 3D com as posições de
inclinação. O firmware liga o motor junto com a coleta e o desliga quando a coleta
para (inclusive na parada automática ao completar META_JANELAS): as 4 classes são
todas com o motor girando — o que muda entre elas é a postura no gabarito e a
massa na hélice.

Este rig não tem LED externo: a classe atual sai só no Monitor Serial.

PARA USAR NO WOKWI:
- Ajustar as credenciais WiFi e o IP do MQTT_SERVER (ou usar as linhas comentadas do Wokwi abaixo)
- Ajustar #define MPU_TYPE (o rig usa MPU6500)
- Remover/comentar a linha `mpu.calibrateAccelGyro(&calib);` (trava no Wokwi, FIFO ausente)
- O motor e as classes de inclinação não têm equivalente fiel no simulador; serve
  para testar botões, LED onboard e publicação MQTT.
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

/* ---- Pinos ---- */
#define SDA_PIN      22
#define SCL_PIN      23
#define BTN_COLETA   21   // inicia/para a coleta
#define BTN_CLASSE   18   // avança para a próxima classe (só com a coleta parada)
#define LED_ONBOARD   2   // LED onboard: aceso = coleta em andamento

// Ponte H do mini motor 3 V: gira enquanto a coleta roda. As versões sem motor
// deste app usam o GPIO 4 para o LED que pisca o índice da classe; aqui o 4 é do
// motor e não há LED externo.
#define MOTOR_INA     4
#define MOTOR_INB    19

/* ---- Sensor (MPU6050 ou MPU6500) ---- */
// O rig do motor usa o MPU6500; troque para MPU6050 se montar o GY-521.
#define MPU_TYPE MPU6500
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
void acionarMotor(bool ligado);
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
  pinMode(LED_ONBOARD, OUTPUT);
  digitalWrite(LED_ONBOARD, LOW);

  pinMode(MOTOR_INA, OUTPUT);
  pinMode(MOTOR_INB, OUTPUT);
  acionarMotor(false);

  conectarWiFi();
  sincronizarRelogio();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setKeepAlive(60);
  mqttClient.setSocketTimeout(30);
  mqttClient.setBufferSize(512);

  Serial.println("Sistema pronto.");
  Serial.println("  Botão 21: inicia/para a coleta");
  Serial.println("  Botão 18: avança para a próxima classe (só com a coleta parada)");
  Serial.println("  A coleta para sozinha ao completar 30 janelas da classe atual.");
  Serial.println("  LED onboard aceso = coleta em andamento (o motor gira junto)");
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
        acionarMotor(true);
        Serial.printf("Coleta INICIADA (classe: %s)\r\n", SEQUENCIA[indiceClasse]);
      } else {
        digitalWrite(LED_ONBOARD, LOW);
        acionarMotor(false);
        Serial.println("Coleta PARADA (motor desligado)");
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
        acionarMotor(false);
        Serial.printf(">>> %d janelas coletadas para %s (rodada %02d). Motor desligado: reposicione o gabarito e aperte o botao 18 para a proxima classe.\r\n",
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
  imprimirClasseAtual();
}

void imprimirClasseAtual() {
  Serial.printf("Classe selecionada: %s (rodada %02d)\r\n", SEQUENCIA[indiceClasse], rodada);
}

/* ---- Motor (ponte H) ---- */
// Sentido unico: INA alto / INB baixo gira; ambos baixos param por inercia
// (coast). Nao usamos freio (ambos altos) para nao sacudir o gabarito bem na
// hora em que a janela seguinte comecaria.
void acionarMotor(bool ligado) {
  digitalWrite(MOTOR_INA, ligado ? HIGH : LOW);
  digitalWrite(MOTOR_INB, LOW);
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
