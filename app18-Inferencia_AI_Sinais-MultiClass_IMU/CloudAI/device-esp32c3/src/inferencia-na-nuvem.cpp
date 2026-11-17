/* inferencia-na-nuvem (ESP32-C3 Super Mini, com motor de vibração) — o ESP32 publica a janela e acende a saída da classe que a nuvem devolve */

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
#define MQTT_SERVER "172.16.10.101"   // endereço do broker da aula

WiFiClient wifiClient;

/* ---- MQTT ---- */
#define MQTT_PORT      1883
#define MQTT_CLIENT_ID "IoTDevMultiClasse001"   // o nome desta placa (o mesmo do dataset)
#define MQTT_PUB_TOPIC "FIAPIoT/motor/multiclasse/" MQTT_CLIENT_ID          // a janela vai por aqui
#define MQTT_SUB_TOPIC "FIAPIoT/motor/multiclasse/" MQTT_CLIENT_ID "/cmd"   // a classe volta por aqui
PubSubClient mqttClient(wifiClient);

/* ---- Pinos (ESP32-C3 Super Mini) ---- */
#define SDA_PIN      20
#define SCL_PIN      21
#define LED_AZUL      1   // operando
#define LED_VERDE     3   // inclinado_frente
#define LED_LARANJA   4   // inclinado_tras
#define LED_VERMELHO 10   // anomalia
#define BUZZER        7   // anomalia (buzzer passivo)
#define LED_RGB       8   // LED RGB da placa: a mesma cor do LED aceso
#define MOTOR_PIN     0   // motor de vibração (pino IN do módulo)

/* ---- Cores do LED RGB (R, G, B) ---- */
#define RGB_AZUL      0,  0, 40   // operando
#define RGB_VERDE     0, 40,  0   // inclinado_frente
#define RGB_LARANJA  40, 10,  0   // inclinado_tras
#define RGB_VERMELHO 40,  0,  0   // anomalia
#define RGB_BRANCO   40, 40, 40   // conectado, sem classe
#define RGB_APAGADO   0,  0,  0   // sem broker

/* ---- Sensor (MPU6050 ou MPU6500) ---- */
#define MPU_TYPE MPU6500 // no Wokwi: MPU6050
MPU_TYPE mpu;

calData calib = { 0 };

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

/* ---- Protótipos ---- */
void conectarWiFi();
void conectarMQTT();
void receberComando(char* topico, byte* conteudo, unsigned int tamanho);
void apagarTodasAsSaidas();
void publicarJanela(float mx, float my, float mz,
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
  delay(1000);   // USB nativa: tempo para o Monitor Serial conectar

  // Saídas
  pinMode(LED_AZUL,     OUTPUT);
  pinMode(LED_VERDE,    OUTPUT);
  pinMode(LED_LARANJA,  OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(BUZZER,       OUTPUT);
  apagarTodasAsSaidas();

  // Motor desligado: a calibração precisa da placa parada
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);

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

  // Teste das saídas: cada LED com a mesma cor no RGB
  Serial.println("Testando as saidas...");
  digitalWrite(LED_AZUL,     HIGH); neopixelWrite(LED_RGB, RGB_AZUL);     delay(400);
  digitalWrite(LED_AZUL,     LOW);
  digitalWrite(LED_VERDE,    HIGH); neopixelWrite(LED_RGB, RGB_VERDE);    delay(400);
  digitalWrite(LED_VERDE,    LOW);
  digitalWrite(LED_LARANJA,  HIGH); neopixelWrite(LED_RGB, RGB_LARANJA);  delay(400);
  digitalWrite(LED_LARANJA,  LOW);
  tone(BUZZER, 500, 250);   // bipe junto com o vermelho
  digitalWrite(LED_VERMELHO, HIGH); neopixelWrite(LED_RGB, RGB_VERMELHO); delay(400);
  apagarTodasAsSaidas();

  // Motor vibrando o tempo todo, como na coleta
  digitalWrite(MOTOR_PIN, HIGH);

  conectarWiFi();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(receberComando);   // resposta da nuvem
  mqttClient.setKeepAlive(60);
  mqttClient.setSocketTimeout(30);
  mqttClient.setBufferSize(512);

  Serial.println("Sistema pronto. Uma janela por segundo, sem botão nenhum.");
  Serial.printf("  Publica em: %s\r\n", MQTT_PUB_TOPIC);
  Serial.printf("  Escuta em:  %s\r\n", MQTT_SUB_TOPIC);
  Serial.println("  Saidas (uma por classe, o RGB na mesma cor):");
  Serial.printf("    GPIO %2d  LED azul     = operando\r\n",         LED_AZUL);
  Serial.printf("    GPIO %2d  LED verde    = inclinado_frente\r\n", LED_VERDE);
  Serial.printf("    GPIO %2d  LED laranja  = inclinado_tras\r\n",   LED_LARANJA);
  Serial.printf("    GPIO %2d  LED vermelho = anomalia\r\n",         LED_VERMELHO);
  Serial.printf("    GPIO %2d  buzzer       = anomalia (bipe)\r\n",  BUZZER);
  Serial.println("  RGB branco = conectado, a nuvem ainda nao respondeu");
  Serial.println("  RGB apagado = sem broker\r\n");
}

/* ============================== LOOP =============================== */
void loop() {
  if (!mqttClient.connected()) {
    conectarMQTT();
  }
  mqttClient.loop();

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

      publicarJanela(mx, my, mz, sx, sy, sz, stdMag, p2p);

      indice = 0;
    }
  }
}

/* ---- Apaga os LEDs e o RGB ---- */
void apagarTodasAsSaidas() {
  digitalWrite(LED_AZUL,     LOW);
  digitalWrite(LED_VERDE,    LOW);
  digitalWrite(LED_LARANJA,  LOW);
  digitalWrite(LED_VERMELHO, LOW);
  digitalWrite(BUZZER,       LOW);
  neopixelWrite(LED_RGB, RGB_APAGADO);
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
  apagarTodasAsSaidas();
  while (!mqttClient.connected()) {
    Serial.printf("Conectando ao MQTT Broker %s...", MQTT_SERVER);
    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      Serial.println(" Conectado!");
      mqttClient.subscribe(MQTT_SUB_TOPIC);
      Serial.printf("Inscrito em: %s\r\n", MQTT_SUB_TOPIC);
      neopixelWrite(LED_RGB, RGB_BRANCO);
    } else {
      Serial.printf(" Falha rc=%d. Tentando em 5s...\r\n", mqttClient.state());
      delay(5000);
    }
  }
}

/* ---- Resposta da nuvem ---- */
void receberComando(char* topico, byte* conteudo, unsigned int tamanho) {
  String classe(conteudo, tamanho);
  classe.trim();

  // Mostra o que chegou
  Serial.println("--- MQTT recebido ---");
  Serial.printf("  topico:  %s\r\n", topico);
  Serial.printf("  tamanho: %u bytes\r\n", tamanho);
  Serial.printf("  payload: \"%s\"\r\n", classe.c_str());

  // Apaga tudo
  apagarTodasAsSaidas();

  // Acende a saída da classe
  if (classe == "operando") {
    digitalWrite(LED_AZUL, HIGH);
    neopixelWrite(LED_RGB, RGB_AZUL);
    Serial.println("  MODELO:  operando         -> LED azul");

  } else if (classe == "inclinado_frente") {
    digitalWrite(LED_VERDE, HIGH);
    neopixelWrite(LED_RGB, RGB_VERDE);
    Serial.println("  MODELO:  inclinado_frente -> LED verde");

  } else if (classe == "inclinado_tras") {
    digitalWrite(LED_LARANJA, HIGH);
    neopixelWrite(LED_RGB, RGB_LARANJA);
    Serial.println("  MODELO:  inclinado_tras   -> LED laranja");

  } else if (classe == "anomalia") {
    digitalWrite(LED_VERMELHO, HIGH);
    neopixelWrite(LED_RGB, RGB_VERMELHO);
    tone(BUZZER, 500, 250);   // bipe de 250 ms
    Serial.println("  MODELO:  anomalia         -> LED vermelho + bipe");

  } else {
    // Classe desconhecida: LEDs apagados, RGB branco
    neopixelWrite(LED_RGB, RGB_BRANCO);
    Serial.println("  MODELO:  classe DESCONHECIDA - LEDs apagados, RGB branco");
    Serial.println("           esperado, sem aspas e sem JSON:");
    Serial.println("           operando | inclinado_frente | inclinado_tras | anomalia");
  }

  Serial.println("---------------------");
}

/* ---- Publica a janela ---- */
void publicarJanela(float mx, float my, float mz,
                    float sx, float sy, float sz,
                    float stdMag, float p2p) {
  JsonDocument doc;
  doc["device"]  = MQTT_CLIENT_ID;
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

  if (!mqttClient.publish(MQTT_PUB_TOPIC, buffer.c_str())) {
    Serial.println("MQTT: falha no envio");
  }
}
