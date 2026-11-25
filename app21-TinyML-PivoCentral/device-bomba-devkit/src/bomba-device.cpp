/* bomba-device — a bomba do pivô: obedece à pessoa, à nuvem e à placa, nessa ordem */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

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
#define MQTT_CLIENT_ID "BombaDoPivo"                    // o nome desta bomba
#define PIVO_PADRAO    "IoTDevMultiClasse001"           // a torre que a bomba obedece ao ligar
#define MQTT_TOPIC_OBEDECER "FIAPIoT/pivo/bomba/obedecer"   // troca a torre: a mensagem é o nome dela
PubSubClient mqttClient(wifiClient);
uint32_t ultimaTentativaMQTT = 0;

/* ---- A torre que a bomba obedece agora ---- */
String pivo = PIVO_PADRAO;
String topicoOrdens() { return String("FIAPIoT/pivo/") + pivo + "/cmd/+"; }   // local (placa), cloud (nuvem), human (pessoa)
String topicoEstado() { return String("FIAPIoT/pivo/") + pivo + "/estado"; }  // o que a bomba fez, e quem mandou

/* ---- Pinos ---- */
#define LED_BOMBA   27   // aceso = bomba ligada
#define SERVO_PIN   25   // a válvula: 0° fechada, 90° aberta
#define LED_ONBOARD  2   // aceso = conectado ao broker
Servo valvula;

/* ---- Quanto tempo vale cada ordem ---- */
const uint32_t VALIDADE_PLACA  = 90UL * 1000;             // 90 s: a placa repete a ordem a cada 30 s
const uint32_t VALIDADE_NUVEM  = 2UL * 60 * 60 * 1000;    // 2 h: a previsão olha 2 h à frente
const uint32_t VALIDADE_PESSOA = 30UL * 60 * 1000;        // 30 min: depois, volta o automático

/* ---- A última ordem de cada um ---- */
struct Ordem {
  bool     valida = false;   // existe uma ordem guardada?
  bool     ligar  = false;   // true = ligar a bomba
  uint32_t chegou = 0;       // millis() da chegada
  String   motivo;
};
Ordem placa, nuvem, pessoa;

/* ---- O que a bomba está fazendo ---- */
bool   bombaLigada     = false;
String quemManda       = "ninguem";
String motivoAtual     = "sem ordem";
bool   estadoPublicado = false;
uint32_t ultimaConferencia = 0;

/* ---- Protótipos ---- */
void receberOrdem(char* topic, byte* payload, unsigned int length);
void trocarPivo(String novo);
void guardarOrdem(Ordem& ordem, const char* quem, JsonDocument& doc);
bool vale(const Ordem& ordem, uint32_t validade);
void aplicarOrdens();
void publicarEstado();
void conectarWiFi();
void conectarMQTT();

/* ============================== SETUP ============================== */
void setup() {
  Serial.begin(115200);

  pinMode(LED_BOMBA,   OUTPUT);
  pinMode(LED_ONBOARD, OUTPUT);
  digitalWrite(LED_BOMBA,   LOW);
  digitalWrite(LED_ONBOARD, LOW);

  // Teste da válvula: abre e fecha
  valvula.attach(SERVO_PIN);
  valvula.write(90); digitalWrite(LED_BOMBA, HIGH); delay(800);
  valvula.write(0);  digitalWrite(LED_BOMBA, LOW);

  conectarWiFi();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setKeepAlive(60);
  mqttClient.setSocketTimeout(30);
  mqttClient.setBufferSize(512);
  mqttClient.setCallback(receberOrdem);

  Serial.println("Bomba pronta. Quem manda: a pessoa, depois a nuvem, depois a placa.");
  Serial.printf("  Ordens em: %s\r\n", topicoOrdens().c_str());
  Serial.printf("  Estado em: %s\r\n", topicoEstado().c_str());
  Serial.printf("  Trocar a torre: %s\r\n\r\n", MQTT_TOPIC_OBEDECER);
}

/* ============================== LOOP =============================== */
void loop() {
  if (!mqttClient.connected()) {
    conectarMQTT();
  }
  mqttClient.loop();

  digitalWrite(LED_ONBOARD, mqttClient.connected() ? HIGH : LOW);

  // Uma vez por segundo: alguma ordem venceu?
  if (millis() - ultimaConferencia >= 1000) {
    ultimaConferencia = millis();
    aplicarOrdens();
    if (!estadoPublicado) publicarEstado();   // o broker estava fora na última mudança
  }
}

/* ---- Chegou uma ordem: o fim do tópico diz de quem ---- */
void receberOrdem(char* topic, byte* payload, unsigned int length) {
  String topico = topic;

  // A troca de torre: a mensagem é só o nome
  if (topico == MQTT_TOPIC_OBEDECER) {
    String nome;
    for (unsigned int i = 0; i < length; i++) nome += (char) payload[i];
    trocarPivo(nome);
    return;
  }

  // Ordem da torre anterior, chegada durante a troca: ignorada
  if (!topico.startsWith(String("FIAPIoT/pivo/") + pivo + "/cmd/")) return;

  JsonDocument doc;
  if (deserializeJson(doc, payload, length)) {
    Serial.println("Ordem com JSON invalido: ignorada");
    return;
  }

  if      (topico.endsWith("/local")) guardarOrdem(placa,  "placa",  doc);
  else if (topico.endsWith("/cloud")) guardarOrdem(nuvem,  "nuvem",  doc);
  else if (topico.endsWith("/human")) guardarOrdem(pessoa, "pessoa", doc);
  else return;

  aplicarOrdens();
  Serial.printf("   quem manda agora: %s\r\n", quemManda.c_str());
}

/* ---- Troca a torre que a bomba obedece (a vez de outro grupo) ---- */
void trocarPivo(String novo) {
  novo.trim();
  if (novo.length() == 0) novo = PIVO_PADRAO;   // mensagem vazia: volta à torre padrão
  if (novo.indexOf('/') >= 0 || novo.indexOf('+') >= 0 || novo.indexOf('#') >= 0) {
    Serial.printf("Nome de torre invalido: \"%s\"\r\n", novo.c_str());
    return;
  }
  if (novo == pivo) return;

  // A torre antiga fica sabendo que a bomba saiu
  bombaLigada = false;
  quemManda   = "ninguem";
  motivoAtual = "a bomba passou para " + novo;
  digitalWrite(LED_BOMBA, LOW);
  valvula.write(0);
  publicarEstado();

  mqttClient.unsubscribe(topicoOrdens().c_str());
  pivo = novo;
  mqttClient.subscribe(topicoOrdens().c_str());

  // As ordens guardadas eram da torre antiga: começa do zero
  placa.valida  = false;
  nuvem.valida  = false;
  pessoa.valida = false;
  motivoAtual = "agora obedece a " + pivo;
  publicarEstado();
  Serial.printf(">> A bomba agora obedece a %s (desligada ate a primeira ordem)\r\n", pivo.c_str());
}

/* ---- Guarda a ordem de um deles (ou apaga, se ele devolveu o controle) ---- */
void guardarOrdem(Ordem& ordem, const char* quem, JsonDocument& doc) {
  if (doc["devolver"] | false) {
    ordem.valida = false;
    Serial.printf("Ordem da %s: DEVOLVER o controle\r\n", quem);
    return;
  }
  if (!doc["bomba"].is<bool>()) {
    Serial.printf("Ordem da %s sem \"bomba\" (true ou false): ignorada\r\n", quem);
    return;
  }

  ordem.valida = true;
  ordem.ligar  = doc["bomba"];
  ordem.chegou = millis();
  ordem.motivo = doc["motivo"] | "";
  Serial.printf("Ordem da %s: %s (%s)\r\n", quem,
                ordem.ligar ? "LIGAR" : "DESLIGAR", ordem.motivo.c_str());
}

/* ---- Uma ordem vale se existe e ainda não venceu ---- */
bool vale(const Ordem& ordem, uint32_t validade) {
  return ordem.valida && millis() - ordem.chegou < validade;
}

/* ---- Quem manda: a pessoa, depois a nuvem, depois a placa ---- */
void aplicarOrdens() {
  bool   ligar  = false;
  String quem   = "ninguem";
  String motivo = "nenhuma ordem valida";

  if (vale(pessoa, VALIDADE_PESSOA)) {
    ligar = pessoa.ligar;  quem = "pessoa";  motivo = pessoa.motivo;
  } else if (vale(nuvem, VALIDADE_NUVEM)) {
    ligar = nuvem.ligar;   quem = "nuvem";   motivo = nuvem.motivo;
  } else if (vale(placa, VALIDADE_PLACA)) {
    ligar = placa.ligar;   quem = "placa";   motivo = placa.motivo;
  }

  if (ligar == bombaLigada && quem == quemManda) return;   // nada mudou

  bombaLigada = ligar;
  quemManda   = quem;
  motivoAtual = motivo;
  digitalWrite(LED_BOMBA, ligar ? HIGH : LOW);
  valvula.write(ligar ? 90 : 0);
  Serial.printf(">> BOMBA %s - quem manda: %s (%s)\r\n",
                ligar ? "LIGADA" : "DESLIGADA", quem.c_str(), motivo.c_str());
  publicarEstado();
}

/* ---- Publica o que a bomba fez, e quem mandou ---- */
void publicarEstado() {
  JsonDocument doc;
  doc["dispositivo"] = MQTT_CLIENT_ID;
  doc["pivo"]        = pivo;
  doc["bomba"]       = bombaLigada;
  doc["quem_manda"]  = quemManda;    // pessoa, nuvem, placa ou ninguem
  doc["motivo"]      = motivoAtual;

  String buffer;
  serializeJson(doc, buffer);
  estadoPublicado = mqttClient.publish(topicoEstado().c_str(), buffer.c_str());
}

/* ---- WiFi: tenta por 10 s e segue ---- */
void conectarWiFi() {
  Serial.printf("Conectando ao WiFi %s", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_2dBm);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setSleep(false);
  uint32_t inicio = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - inicio < 10000) {
    delay(500);
    Serial.print('.');
  }
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Sem WiFi: a bomba fica desligada ate chegar uma ordem.");
  }
}

/* ---- MQTT: uma tentativa a cada 5 s; ao conectar, assina as ordens e a troca de torre ---- */
void conectarMQTT() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (millis() - ultimaTentativaMQTT < 5000) return;
  ultimaTentativaMQTT = millis();

  Serial.printf("Conectando ao MQTT Broker %s...", MQTT_SERVER);
  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println(" Conectado!");
    mqttClient.subscribe(topicoOrdens().c_str());
    mqttClient.subscribe(MQTT_TOPIC_OBEDECER);
    publicarEstado();
  } else {
    Serial.printf(" Falha rc=%d. Nova tentativa em 5 s\r\n", mqttClient.state());
  }
}
