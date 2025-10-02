/* *Includes* */
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "ESP32Sensors.hpp"

/* ---- Config Sensores ---- */
const static uint8_t SCL_PIN   = 19;
const static uint8_t SDA_PIN   = 18;
const static uint8_t DHT_PIN   = 26;
const static uint8_t DHT_MODEL = DHT22;
const static uint8_t LED_PIN   = 27;

/* ---- Config Wi‑Fi ---- */
const char* WIFI_SSID     = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
WiFiClient wifiClient; // Define client WiFi

/* ---- Config MQTT ---- */
#define MQTT_SERVER     "host.wokwi.internal"   //10.63.106.231 - IP do Raspberry Pi
#define MQTT_PORT       1883
#define MQTT_PUB_TOPIC  "FIAPIoT/aula09/noris/motor/dados"
#define MQTT_SUB_TOPIC  "FIAPIoT/aula09/noris/motor/cmd"
// ATENÇÃO: ClientID DEVE SER ÚNICO NO BROKER!
#define MQTT_CLIENT_ID  "IoTDeviceNoris001"
PubSubClient mqttClient(wifiClient);

/* ---- Controle de intervalo de envio ---- */
const int INTERVALO_COLETA = 3000;  // 3 s
int proximaLeitura = 0;

/* ---- Controle do motor ---- */
bool motorLigado = true;

/* ---- Protótipos ---- */
void conectarWiFi();
void conectarMQTT();
void callbackMQTT(char* topic, byte* payload, unsigned int length);
bool enviarDadosColetados();

/* ---- MAIN (setup e loop) ---- */
void setup() {
  Serial.begin(115200);
    
  Serial.println("\n\n===================================");
  Serial.println("  SISTEMA DE MONITORAMENTO MOTOR  ");
  Serial.println("===================================");
  Serial.println("Demonstração: Fog / Near Edge");
  Serial.println("===================================\n");

  //Inicializa todos os sensores
  Serial.println("Inicializando sensores...");
  ESP32Sensors::beginAll(DHT_PIN, DHT_MODEL, SCL_PIN, SDA_PIN, LED_PIN);

  // Liga LED indicando motor funcionando
  ESP32Sensors::LED::on();
  motorLigado = true;
  Serial.println("LED ligado (motor operando)");

  //Conecta no WiFi
  conectarWiFi();
  
  //Configura MQTT e conecta no Broker
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(callbackMQTT);
  conectarMQTT();

  Serial.println("\nSistema embarcado inicializado!");
  Serial.println("Envio de dados a cada 5 segundos...\n");
}

void loop() {
  if (!mqttClient.connected()) {
      conectarMQTT();
  }
  mqttClient.loop();

  if (millis() < proximaLeitura) return;   // Aguarda próximo ciclo
  proximaLeitura = millis() + INTERVALO_COLETA;

  bool dadosEnviados = enviarDadosColetados();
  if (dadosEnviados) {
      Serial.println("\nSUCESSO: Dados enviados para Near Edge / MQTT Broker");
  } else {
      Serial.println("\nFALHA: Erro ao enviar dados");
  }

  delay(100);
}



// ===================================================================
// FUNÇÕES
// ===================================================================

// ======================== Conectar ao WiFi =========================
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

// ==================== Conectar ao MQTT Broker ======================
void conectarMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Conectando ao MQTT Broker" + String(MQTT_SERVER));

    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      Serial.println("--> Conectado ao MQTT Broker!");
      // Inscreve no tópico de comandos
      mqttClient.subscribe(MQTT_SUB_TOPIC);
      Serial.print("Inscrito no tópico: ");
      Serial.println(MQTT_SUB_TOPIC);

    } else {
      Serial.print(" Falha, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }  
}

// ===== Callback MQTT - Processa dados quando forem recebidos =======
void callbackMQTT(char* topic, byte* payload, unsigned int length) {
  // Converte o payload em String para facilitar a comparação
  String comando;
  for (unsigned int i = 0; i < length; i++) {
    comando += (char)payload[i];
  }

  // Exibe dados no Serial Monitor
  Serial.println("\n========================================");
  Serial.println("COMANDO RECEBIDO DA FOG/NEAR EDGE:");
  Serial.println("========================================");
  Serial.print("Tópico: ");
  Serial.println(topic);
  Serial.print("Mensagem: ");
  Serial.println(comando);

  comando.toUpperCase();

  // Se o comando for desligar, apaga o LED (motor desliga)
  if (comando == "OFF") {
    ESP32Sensors::LED::off();
    motorLigado = false;
    Serial.println("COMANDO: desligar motor. LED apagado.");
  }
  // Se o comando for ligar, acende o LED
  else if (comando == "ON") {
    ESP32Sensors::LED::on();
    motorLigado = true;
    Serial.println("COMANDO: ligar motor. LED aceso.");
  }
  // Outros comandos podem ser tratados aqui
}

// ===== Envia dados coletados =======
bool enviarDadosColetados() {
  // Lê sensor de ambiente (DHT22)
  ESP32Sensors::Ambiente::AMBIENTE dadosAmbiente = ESP32Sensors::Ambiente::medirAmbiente();
  
  // Lê acelerômetro (MPU6050)
  sensors_event_t dadosAccel = ESP32Sensors::Accel::medirAccel();

  // Exibe dados no Serial Monitor
  Serial.println("\n========================================");
  Serial.println("DADOS COLETADOS (Far Edge/Extreme Edge):");
  Serial.println("========================================");
  
  if (!dadosAmbiente.valido) {
    Serial.println("Erro na leitura do DHT22");
  }

  /* --- Log no Serial Monitor --- */
  Serial.println("Dados coletados: ");
  Serial.printf("[AMBIENTE] - Temp: %.2f °C | Umid: %.2f %% | HI: %.2f °C",
                 dadosAmbiente.temp, dadosAmbiente.umid, dadosAmbiente.ic);
  Serial.println("");
  Serial.printf("[ACELERAÇÃO] - x: %.2f m/s² | y: %.2f m/s² | z: %.2f m/s²",
                dadosAccel.acceleration.x, dadosAccel.acceleration.y, dadosAccel.acceleration.z);
  Serial.println("");
  Serial.printf("[MOTOR STATUS] - %s", motorLigado ? "Ligado" : "Desligado");
  Serial.println("");

  
  // Prepara JSON para envio via MQTT
  JsonDocument doc;
  
  doc["device"] = MQTT_CLIENT_ID;
  doc["temp"] = dadosAmbiente.temp;
  doc["umid"] = dadosAmbiente.umid;
  doc["ic"] = dadosAmbiente.ic;
  doc["accel_x"] = dadosAccel.acceleration.x;
  doc["accel_y"] = dadosAccel.acceleration.y;
  doc["accel_z"] = dadosAccel.acceleration.z;
  doc["motor_status"] = motorLigado;

  // Serializa e envia JSON
  String buffer;
  serializeJson(doc, buffer);
  
  bool envio = mqttClient.publish(MQTT_PUB_TOPIC, buffer.c_str());
  
  return envio;
}

