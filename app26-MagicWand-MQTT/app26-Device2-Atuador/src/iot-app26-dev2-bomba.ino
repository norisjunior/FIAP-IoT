/* ==== INCLUDES ============================================================ */
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "ESP32Sensors.hpp"

/* ==== Configurações de Hardware =========================================== */
const static uint8_t BUZZER_PIN   = 15;
const static uint8_t LED_PIN      = 27;
const static uint8_t SERVO_PIN    = 13;

/* ==== WI-FI ======================+++++==================================== */
const char* WIFI_SSID     = "Wokwi-GUEST";   // Rede pública do simulador
const char* WIFI_PASSWORD = "";
WiFiClient wifiClient;

/* ==== MQTT ============================+++++++++++++======================= */
#define MQTT_HOST            "host.wokwi.internal"
#define MQTT_PORT            1883
// assina usando Wildcard, que poderá ser
// local | cloud | human
#define MQTT_TOPIC_SUB_CMDS  "FIAPIoT/smartagro/cmd/+"
#define MQTT_DEVICEID        "FIAPIoTapp26B"
PubSubClient mqttClient(wifiClient);
bool wifiConectado = false;
bool mqttConectado = false;
unsigned long ultimaTentativaReconexao = 0;
const unsigned long INTERVALO_RECONEXAO = 5000; // 5 segundos entre tentativas
// Tópicos específicos (quem enviou o comando)
#define MQTT_TOPIC_LOCAL     "FIAPIoT/smartagro/cmd/local"
#define MQTT_TOPIC_CLOUD     "FIAPIoT/smartagro/cmd/cloud"
#define MQTT_TOPIC_HUMAN     "FIAPIoT/smartagro/cmd/human"
// Controle dos comandos recebidos
String ultimoComando = "NONE";        // "ON" | "OFF" | "NONE"
String origemUltimoComando = "NENHUMA";

/* ==== Controle do motor =================================================== */
bool bombaLigada = false;

/* ==== Protótipos ========================================================== */
void conectarWiFi();
void conectarMQTT();
void callbackMQTT(char* topic, byte* payload, unsigned int length);
void atualizarBomba();

/* ==== SETUP =============================================================== */
void setup() {
  Serial.begin(115200);
    
  Serial.println("\n\n===================================");
  Serial.println("  CONTROLE DA BOMBA D'ÁGUA  ");
  Serial.println("===================================");

  //Inicializa todos os sensores
  Serial.println("Inicializando bomba d'água...");
  ESP32Sensors::beginAll(BUZZER_PIN, LED_PIN, SERVO_PIN);

  // Liga LED indicando motor funcionando
  ESP32Sensors::LED::off();
  ESP32Sensors::MiniServo::ligar();
  ESP32Sensors::Audio::ativarAlarme();
  delay(1000);
  ESP32Sensors::MiniServo::desligar();
  ESP32Sensors::Audio::desativarAlarme();
  bombaLigada = false;
  Serial.println("Bomba d'água inicializada - ESTADO INICIAL: DESLIGADA");

  //Conecta no WiFi
  conectarWiFi();
  
  //Configura MQTT e conecta no Broker
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setKeepAlive(60);     // Manter conexão viva por 60s
  mqttClient.setSocketTimeout(60); // Timeout de 60s  
  mqttClient.setBufferSize(512);   // Buffer adequado considerando o payload gerado
  mqttClient.setCallback(callbackMQTT);

  // Conecta no MQTT
  conectarMQTT();

  Serial.println("\nSistema embarcado inicializado!");
  Serial.println("Aguardando comando...\n");

}

void loop() {
  if (!mqttClient.connected()) {
      conectarMQTT();
  }
  mqttClient.loop();

  delay(100);
}



// ===================================================================
// FUNÇÕES
// ===================================================================

/* ==== WIFI ================================================================ */
void conectarWiFi() {
  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  uint8_t tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 30) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("OK! IP: ");
    Serial.println(WiFi.localIP());
    wifiConectado = true;
  } else {
    Serial.println("Falha ao conectar Wi-Fi.");
    wifiConectado = false;
  }
}

/* ==== MQTT ================================================================ */
void conectarMQTT() {
  if (!wifiConectado) {
    Serial.println("[MQTT] Sem Wi-Fi.");
    return;
  }

  Serial.print("[MQTT] Conectando ao broker ");
  Serial.print(MQTT_HOST);
  Serial.print(" ... ");

  int tentativas = 0;
  while (!mqttClient.connected() && tentativas < 5) {
    if (mqttClient.connect(MQTT_DEVICEID)) {
      Serial.println("OK!");
      mqttConectado = true;

      // Inscreve no tópico coringa
      if (mqttClient.subscribe(MQTT_TOPIC_SUB_CMDS)) {
        Serial.print("[MQTT] Inscrito em: ");
        Serial.println(MQTT_TOPIC_SUB_CMDS);
      } else {
        Serial.println("[MQTT] Falha ao se inscrever no tópico de comandos.");
      }

    } else {
      Serial.print("Falha, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" tentando de novo...");
      tentativas++;
      delay(1000);
    }
  }

  if (!mqttClient.connected()) {
    Serial.println("[MQTT] Não foi possível conectar ao broker.");
  }
}

// ===== Callback MQTT - Processa dados quando forem recebidos =======
void callbackMQTT(char* topic, byte* payload, unsigned int length) {
  String topico = String(topic);

  Serial.println("\n========================================");
  Serial.println("COMANDO RECEBIDO (JSON):");
  Serial.println("========================================");
  Serial.print("Tópico: ");
  Serial.println(topico);

  // Monta o JsonDocument a partir do payload
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload, length);

  if (error) {
    Serial.print("Erro ao realizar parse do JSON: ");
    Serial.println(error.f_str());
    return;
  }

  // Loga o JSON recebido
  Serial.print("JSON recebido: ");
  serializeJson(doc, Serial);
  Serial.println();

  // Se o JSON não tem bomba: IGNORAR
  JsonVariant statusVar = doc["bomba"];
  if (!statusVar.is<bool>()) {
      Serial.println("JSON sem 'bomba' booleano. Comando IGNORADO.");
      return;
  }
  // Lê campos do JSON
  const char* dispositivo = doc["dispositivo"] | "desconhecido";
  bool bomba = statusVar.as<bool>();  // true/false

  Serial.print("Dispositivo origem: ");
  Serial.println(dispositivo);
  Serial.print("bomba: ");
  Serial.println(bomba ? "true" : "false");

  // Converte bool para "ON"/"OFF"
  String comando = bomba ? "ON" : "OFF";

  // Identifica origam
  String origem = "ERRO_ORIGEM";

  if (topico == MQTT_TOPIC_LOCAL) origem = "LOCAL";
  else if (topico == MQTT_TOPIC_CLOUD) origem = "CLOUD";
  else if (topico == MQTT_TOPIC_HUMAN) origem = "HUMAN";

  // Atualiza o comando mais recente
  ultimoComando = comando;
  origemUltimoComando = origem;

  Serial.println("---- NOVO COMANDO ----");
  Serial.print("Origem: "); Serial.println(origem);
  Serial.print("Comando: "); Serial.println(comando);

  // Aplica o comando recebido
  atualizarBomba();
}

/* ========================================================================== */
/* Função: atualizarBomba                                                     */
/*   - Aplica o comando recebido                                              */
/* ========================================================================== */
void atualizarBomba() {
  if (ultimoComando == "NONE") {
    Serial.println("Nenhum comando recebido ainda.");
    return;
  }

  bool ligar = (ultimoComando == "ON");

  Serial.println("----------------------------------------");
  Serial.print("Aplicando comando: ");
  Serial.print(ligar ? "LIGAR" : "DESLIGAR");
  Serial.print(" | Origem: ");
  Serial.println(origemUltimoComando);
  Serial.println("----------------------------------------");

  if (ligar && !bombaLigada) {
    Serial.println("AÇÃO: Ligar bomba.");
    ESP32Sensors::LED::on();
    ESP32Sensors::MiniServo::ligar();
    bombaLigada = true;
  } else if (!ligar && bombaLigada) {
    Serial.println("AÇÃO: Desligar bomba.");
    ESP32Sensors::LED::off();
    ESP32Sensors::MiniServo::desligar();
    bombaLigada = false;
  } else {
    Serial.println("AÇÃO: Nenhuma mudança (estado já correspondente).");
  }
}