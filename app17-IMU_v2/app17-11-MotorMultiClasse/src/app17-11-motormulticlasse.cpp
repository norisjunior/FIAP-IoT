/* =====================================================================
 * app17-11-motormulticlasse.cpp — 5 classes do motor (Forma 1, hardware fisico)
 *
 * O QUE FAZ
 *   Le aceleracao (MPU6500) a 500 Hz, agrupa em JANELA FIXA de 1000 amostras
 *   (2 s), calcula 12 features NO PROPRIO ESP32 e imprime 1 linha CSV por
 *   janela na Serial. Se WiFi/broker estiverem disponiveis, publica a MESMA
 *   janela por MQTT tambem — mas a Serial funciona sozinha, sem rede.
 *
 * MONTAGEM: MPU6500 + mini motor com helice + 2 botoes + LED externo + LED
 *   onboard. Projetado para HARDWARE FISICO — a inclinacao real do conjunto
 *   (gabarito 3D com as posicoes) e o ponto do experimento; nao ha
 *   equivalente fiel no simulador Wokwi para as classes de inclinacao.
 *
 * A SEQUENCIA DE 5 CLASSES (fixa, ciclica):
 *   desligado -> operando -> inclinado_frente -> inclinado_tras -> anomalia -> (repete)
 *
 *   Botao 1 (26): liga/desliga o motor (toggle).
 *   Botao 2 (25): dispara UMA rodada completa da classe atual (30 janelas =
 *     60 s): settle de 1 s -> coleta -> para sozinho -> avanca a sequencia.
 *     NAO e toggle — um toque so, sem pular etapa.
 *
 *   "rodada" incrementa a cada VOLTA COMPLETA pelas 5 classes, nao a cada
 *   toque do botao 2 — e o group do LeaveOneGroupOut no notebook 2.6.
 *
 *   Guarda: antes de iniciar, o firmware confere se o motor esta no estado
 *   que a classe atual espera (desligado = motor parado; as outras 4 = motor
 *   ligado) e RECUSA iniciar se nao bater. O firmware NUNCA confere a
 *   inclinacao pelo sensor — so estado de atuador que ele mesmo comanda.
 *   Validar pelo acelerometro seria vazamento: usar o sensor pra rotular o
 *   dado que o modelo vai aprender a classificar.
 *
 *   Antes da classe 5 (anomalia) e ao voltar da 5 pra 1, o monitor avisa em
 *   destaque para colar/remover a fita da helice — sem isso, "operando" com
 *   fita vira "anomalia" disfarcada na proxima volta.
 *
 * AMOSTRAGEM: 500 Hz, PROVISORIO. Sem filtro anti-aliasing (DLPF) de proposito
 *   (ver setup()): a banda cheia do sensor fica disponivel, ao custo de deixar
 *   qualquer energia acima de Nyquist (250 Hz) entrar rebatida na janela.
 *   RECONFIRMAR a 500 Hz depois de medir o conteudo real do sinal (FFT do raw
 *   do app17-10) — se houver energia relevante acima de 250 Hz, subir FS_HZ.
 *
 * DERIVADO DE app17-9 (edge/janela). PINAGEM: SDA=19 SCL=18 | BTN_MOTOR=26
 *   (declara o "motor" ligado/desligado -- ver MONTAGEM) | BTN_COLETA=25 |
 *   LED=27 (externo, coleta ativa) | LED onboard=2 (motor ligado). Sem pino
 *   de motor: o "motor" agora e o celular vibrando na mao do operador.
 * ===================================================================== */

#include <Arduino.h>
#include "FastIMU.h"
#include <Wire.h>
#include <math.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

/* ====================== Config de rede (opcional) ======================
 * O firmware funciona INTEIRO so pela Serial, sem rede nenhuma — a conexao
 * abaixo tem tentativas limitadas (nao trava o boot) e o loop principal
 * segue rodando (amostragem, botoes, CSV na Serial) esteja o WiFi/broker
 * disponivel ou nao.
 *
 * (A) WOKWI — PADRAO. (B) ESP32 FISICO: comente A, descomente B. */

// ---- (A) Wokwi (padrao) ----
const char* WIFI_SSID     = "IoTNJ";
const char* WIFI_PASSWORD = "Th1ng$IoT";
#define MQTT_SERVER "10.93.11.69"

// ---- (B) ESP32 fisico ----
// const char* WIFI_SSID     = "SUA_REDE_WIFI";
// const char* WIFI_PASSWORD = "SUA_SENHA";
// #define MQTT_SERVER "192.168.0.100"   // IP da maquina com a IoT-platform

WiFiClient wifiClient;
#define MQTT_PORT      1883
#define MQTT_PUB_TOPIC "FIAPIoT/motor/multiclasse"
#define MQTT_CLIENT_ID "IoTDeviceNorisMotorMultiClasse001"
PubSubClient mqttClient(wifiClient);
bool wifiDisponivel = false;   // false = segue so na Serial (Fase 1)

/* ---- Pinos ---- */
#define SDA_PIN      19
#define SCL_PIN      18
#define BTN_MOTOR    26
#define BTN_COLETA   25
#define LED_PIN      27   // externo: aceso = coleta em andamento
#define LED_ONBOARD   2   // onboard: aceso = motor ligado

/* Sensor: hardware real do experimento e o MPU6500. Se for o MPU6050
 * original (GY-521), troque o #define abaixo. */
#define MPU_TYPE MPU6500 // troque para MPU6050 se estiver usando o sensor original
MPU_TYPE mpu;

calData calib = { 0 }; // preenchida pela calibracao no setup()

/* ---- Sequencia ciclica de 5 classes ---- */
const char* SEQUENCIA[]      = { "desligado", "operando", "inclinado_frente",
                                 "inclinado_tras", "anomalia" };
const bool  MOTOR_ESPERADO[] = { false, true, true, true, true };
const int   N_CLASSES = 5;

int indiceClasse = 0;   // posicao atual na sequencia (0..4)
int rodada        = 1;  // incrementa a cada VOLTA COMPLETA pelas 5 classes

const int JANELAS_POR_RODADA = 30;    // 30 janelas de 2 s = 60 s por rodada
const int SETTLE_MS          = 1000;  // mao do operador saindo do conjunto

/* ---- "Motor" (agora o celular vibrando na mao, sem pino fisico) ----
 * motorLigado so reflete o que o operador declara no botao 1: o firmware nao
 * aciona nada, so guarda o estado pra validar a classe (MOTOR_ESPERADO). */
bool motorLigado = false;
int ultimoBotaoMotor = HIGH;
unsigned long ultimoDebounceMotor = 0;
const unsigned long DEBOUNCE_MS = 300;

/* ---- Botao coleta ---- */
int ultimoBotaoColeta = HIGH;
unsigned long ultimoDebounceColeta = 0;

/* ---- Estado da coleta ---- */
bool aguardandoSettle = false;
uint32_t inicioSettle = 0;
bool coletando = false;
int janelaNaRodada = 0;

/* ---- Amostragem: 500 Hz, janela de 2 s ---- */
const int FS_HZ         = 500;                 // PROVISORIO — ver cabecalho
const int AMOSTRA_MS    = 1000 / FS_HZ;         // 2 ms
const int TAMANHO_JANELA = FS_HZ * 2;           // 1000 amostras = 2 s

float ax_buf[TAMANHO_JANELA];
float ay_buf[TAMANHO_JANELA];
float az_buf[TAMANHO_JANELA];
float mag_buf[TAMANHO_JANELA];

int indiceAmostra = 0;
uint32_t tempoAnterior = 0;
uint32_t millisInicioJanela = 0;

/* ---- Prototipos ---- */
void conectarWiFi();
void tentarMQTT();
void avancarSequencia();
void imprimirProximoPasso();
void publicarJanela(int janelaIdx, float fsReal,
                    float mx, float my, float mz,
                    float sx, float sy, float sz, float rmag,
                    float stdMag, float p2p, float crest, float kurt, float zcr);

/* =========================== Features =========================== */
/* mean/std/rms: mesmas formulas do app17-5/8/9 (divisao por N, ddof=0). */
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

/* As 5 novas (item 3 do plano), sobre a magnitude mag = sqrt(ax²+ay²+az²). */
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

// Curtose populacional (viesada, ddof=0) — mais simples que a de bias
// corrigido do pandas usada nos notebooks; aqui so precisa ser consistente
// consigo mesma (o notebook 2.6 le a feature pronta, nao recalcula).
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
    Serial.println("Erro: MPU nao encontrado");
    while (1);
  }
  mpu.setAccelRange(8);
  // Sem filtro anti-aliasing (DLPF) de proposito: a banda cheia do sensor
  // fica disponivel para capturar o maximo de conteudo do sinal. Trade-off:
  // energia acima de Nyquist (FS_HZ/2) volta rebatida (aliasing) pra dentro
  // da janela -- pra features estatisticas (RMS/std/etc.) isso costuma ser
  // aceitavel, mas nao e "mais informacao limpa", e sim mais energia (parte
  // dela distorcida) entrando na janela.
  mpu.setAccelODR(FS_HZ); // acompanha a taxa de leitura, evita amostra duplicada
  Serial.println("MPU iniciado");

  Serial.println("Mantenha o sensor parado e nivelado para calibrar...");
  delay(2000);
  mpu.calibrateAccelGyro(&calib);
  mpu.init(calib, 0x68);

  pinMode(BTN_MOTOR,   INPUT_PULLUP);
  pinMode(BTN_COLETA,  INPUT_PULLUP);
  pinMode(LED_PIN,     OUTPUT);
  pinMode(LED_ONBOARD, OUTPUT);
  digitalWrite(LED_PIN,     LOW);
  digitalWrite(LED_ONBOARD, LOW);

  conectarWiFi();   // tolerante a falha: poucas tentativas, nunca trava o boot
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setKeepAlive(60);
  mqttClient.setSocketTimeout(30);
  mqttClient.setBufferSize(512);

  Serial.println();
  Serial.println("Sistema pronto. (app17-11 -- 5 classes do motor, Forma 1)");
  Serial.println("  Botao 26: liga/desliga o motor");
  Serial.println("  Botao 25: dispara uma rodada da classe atual (30 janelas, ~60 s)");
  Serial.printf("  Topico MQTT (se disponivel): %s\n", MQTT_PUB_TOPIC);
  Serial.println();
  Serial.println("classe,rodada,janela,fs_real,mean_ax,mean_ay,mean_az,std_ax,std_ay,std_az,"
                 "rms_mag,std_mag,p2p_mag,crest_mag,kurt_mag,zcr_mag");
  imprimirProximoPasso();
}

/* ============================== LOOP =============================== */
void loop() {
  if (wifiDisponivel) mqttClient.loop();

  // --- Botao motor (toggle). Bloqueado durante settle/coleta: trocar o
  // motor no meio de uma rodada corromperia a janela. ---
  int leituraMotor = digitalRead(BTN_MOTOR);
  if (ultimoBotaoMotor == HIGH && leituraMotor == LOW) {
    if (millis() - ultimoDebounceMotor > DEBOUNCE_MS) {
      if (coletando || aguardandoSettle) {
        Serial.println("Coleta em andamento -- aguarde a rodada terminar para trocar o motor.");
      } else {
        motorLigado = !motorLigado;
        digitalWrite(LED_ONBOARD, motorLigado ? HIGH : LOW);
        Serial.println(motorLigado ? "Motor LIGADO" : "Motor DESLIGADO");
      }
      ultimoDebounceMotor = millis();
    }
  }
  ultimoBotaoMotor = leituraMotor;

  // --- Botao coleta: toque unico dispara UMA rodada completa. ---
  int leituraColeta = digitalRead(BTN_COLETA);
  if (ultimoBotaoColeta == HIGH && leituraColeta == LOW) {
    if (millis() - ultimoDebounceColeta > DEBOUNCE_MS) {
      if (coletando || aguardandoSettle) {
        // ja em andamento: ignora o toque (nao ha pular etapa nem reiniciar)
      } else {
        bool motorOk = (motorLigado == MOTOR_ESPERADO[indiceClasse]);
        if (!motorOk) {
          Serial.printf("ERRO: classe '%s' espera motor %s, mas o motor esta %s. "
                        "Ajuste no botao 1 e tente de novo.\n",
                        SEQUENCIA[indiceClasse],
                        MOTOR_ESPERADO[indiceClasse] ? "LIGADO" : "DESLIGADO",
                        motorLigado ? "LIGADO" : "DESLIGADO");
        } else {
          aguardandoSettle = true;
          inicioSettle = millis();
          digitalWrite(LED_PIN, HIGH);
          Serial.printf("Rodada armada (classe '%s'). Settle de %d ms...\n",
                        SEQUENCIA[indiceClasse], SETTLE_MS);
        }
      }
      ultimoDebounceColeta = millis();
    }
  }
  ultimoBotaoColeta = leituraColeta;

  // --- Fim do settle: comeca a coleta de fato ---
  if (aguardandoSettle && (millis() - inicioSettle >= SETTLE_MS)) {
    aguardandoSettle = false;
    coletando = true;
    janelaNaRodada = 0;
    indiceAmostra = 0;
    tempoAnterior = millis();
    millisInicioJanela = millis();
    Serial.printf("# INICIO classe=%s rodada=%02d motor=%d\n",
                  SEQUENCIA[indiceClasse], rodada, motorLigado ? 1 : 0);
  }

  // --- Amostragem a 500 Hz, so durante a coleta ---
  if (coletando) {
    if (millis() - tempoAnterior >= AMOSTRA_MS) {
      // Avanca em passos fixos de AMOSTRA_MS (e nao "= millis()"): assim o
      // atraso de um ciclo nao empurra o proximo e a taxa nao escorrega.
      tempoAnterior += AMOSTRA_MS;
      if (millis() - tempoAnterior >= AMOSTRA_MS) tempoAnterior = millis();

      // FastIMU nao expoe um "tem dado novo?" publico como o read() do
      // FlixPeriph retornava; por isso o ODR do sensor foi setado pra bater
      // com FS_HZ (setup), pra o loop nao perguntar mais rapido do que o
      // sensor produz amostra nova.
      AccelData accel;
      mpu.update();
      mpu.getAccel(&accel);
      ax_buf[indiceAmostra] = accel.accelX;
      ay_buf[indiceAmostra] = accel.accelY;
      az_buf[indiceAmostra] = accel.accelZ;
      indiceAmostra++;

      if (indiceAmostra >= TAMANHO_JANELA) {
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

        Serial.printf("%s,%02d,%d,%.1f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
                      SEQUENCIA[indiceClasse], rodada, janelaNaRodada + 1, fsReal,
                      mx, my, mz, sx, sy, sz, rmag,
                      stdMag, p2p, crest, kurt, zcr);

        if (wifiDisponivel) {
          tentarMQTT();
          publicarJanela(janelaNaRodada + 1, fsReal, mx, my, mz, sx, sy, sz, rmag,
                        stdMag, p2p, crest, kurt, zcr);
        }

        janelaNaRodada++;
        indiceAmostra = 0;
        millisInicioJanela = millis();

        if (janelaNaRodada >= JANELAS_POR_RODADA) {
          Serial.printf("# FIM rodada=%02d janelas=%d\n", rodada, JANELAS_POR_RODADA);
          coletando = false;
          digitalWrite(LED_PIN, LOW);
          avancarSequencia();
        }
      }
    }
  }
}

/* =========================== Sequencia =============================== */
void avancarSequencia() {
  indiceClasse++;
  if (indiceClasse >= N_CLASSES) {
    indiceClasse = 0;
    rodada++;
  }
  imprimirProximoPasso();
}

void imprimirProximoPasso() {
  Serial.println();
  Serial.println("==================================================");
  Serial.printf("PROXIMA [%d/%d] %-18s rodada=%02d   motor: %s\n",
                indiceClasse + 1, N_CLASSES, SEQUENCIA[indiceClasse], rodada,
                MOTOR_ESPERADO[indiceClasse] ? "LIGADO" : "DESLIGADO");

  if (indiceClasse == 4) {
    // ultima classe da sequencia = anomalia
    Serial.println(">>> COLE A FITA NA HELICE AGORA <<<");
    Serial.println("    depois toque o botao 2");
  } else if (indiceClasse == 0 && rodada > 1) {
    // acabou de fechar uma volta completa (passou pela anomalia)
    Serial.println(">>> REMOVA A FITA DA HELICE <<<");
    Serial.println("    depois desligue o motor e toque o botao 2");
  } else {
    Serial.println("> posicione no gabarito e toque o botao 2");
  }
  Serial.println("==================================================");
}

/* =========================== Rede (opcional) =========================== */
void conectarWiFi() {
  Serial.printf("Conectando ao WiFi %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  // Tentativas limitadas (nao trava o boot): a Fase 1 (so Serial) precisa
  // funcionar mesmo sem WiFi/broker nenhum por perto.
  for (int tentativas = 0; tentativas < 20 && WiFi.status() != WL_CONNECTED; tentativas++) {
    delay(500);
    Serial.print('.');
  }
  wifiDisponivel = (WiFi.status() == WL_CONNECTED);
  Serial.println();
  if (wifiDisponivel) {
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi indisponivel -- seguindo so pela Serial (sem MQTT).");
  }
}

void tentarMQTT() {
  if (!wifiDisponivel || mqttClient.connected()) return;
  Serial.printf("Conectando ao MQTT %s...", MQTT_SERVER);
  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println(" conectado!");
  } else {
    Serial.printf(" falha rc=%d (Serial continua normalmente)\n", mqttClient.state());
  }
}

void publicarJanela(int janelaIdx, float fsReal,
                    float mx, float my, float mz,
                    float sx, float sy, float sz, float rmag,
                    float stdMag, float p2p, float crest, float kurt, float zcr) {
  if (!mqttClient.connected()) return;

  JsonDocument doc;
  doc["device"]   = MQTT_CLIENT_ID;
  doc["classe"]   = SEQUENCIA[indiceClasse];
  doc["rodada"]   = rodada;
  doc["janela"]   = janelaIdx;
  doc["fs_real"]  = serialized(String(fsReal, 1));
  doc["mean_ax"]  = serialized(String(mx, 3));
  doc["mean_ay"]  = serialized(String(my, 3));
  doc["mean_az"]  = serialized(String(mz, 3));
  doc["std_ax"]   = serialized(String(sx, 3));
  doc["std_ay"]   = serialized(String(sy, 3));
  doc["std_az"]   = serialized(String(sz, 3));
  doc["rms_mag"]  = serialized(String(rmag, 3));
  doc["std_mag"]  = serialized(String(stdMag, 3));
  doc["p2p_mag"]  = serialized(String(p2p, 3));
  doc["crest_mag"] = serialized(String(crest, 3));
  doc["kurt_mag"] = serialized(String(kurt, 3));
  doc["zcr_mag"]  = serialized(String(zcr, 3));

  String buffer;
  serializeJson(doc, buffer);
  mqttClient.publish(MQTT_PUB_TOPIC, buffer.c_str());
}
