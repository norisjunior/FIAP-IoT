/* *Includes* */
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "ESP32Sensors.hpp"

/* ---- Config Sensores ---- */
const static uint8_t LED_PIN   = 27;

/* ---- Config Wi‑Fi ---- */
const char* WIFI_SSID     = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
WiFiClient wifiClient; // Define client WiFi

/* ---- Config MQTT ---- */
#define MQTT_SERVER     "host.wokwi.internal"
#define MQTT_PORT       1883
#define MQTT_SUB_TOPIC  "FIAPIoT/smartagro/cmd"
// ATENÇÃO: ClientID DEVE SER ÚNICO NO BROKER!
#define MQTT_CLIENT_ID  "IoTDevice002Bomba"
PubSubClient mqttClient(wifiClient);

/* ---- Controle de intervalo de envio ---- */
const int INTERVALO_COLETA = 3000;  // 3 s
int proximaLeitura = 0;

/* ---- Controle do motor ---- */
bool bombaLigada = true;

/* ---- Protótipos ---- */
void conectarWiFi();
void conectarMQTT();
void callbackMQTT(char* topic, byte* payload, unsigned int length);

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
  ESP32Sensors::beginAll(LED_PIN);

  // Liga LED indicando motor funcionando
  ESP32Sensors::LED::on();
  bombaLigada = true;
  Serial.println("LED ligado (bomba operando)");

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
    bombaLigada = false;
    Serial.println("COMANDO: desligar bomba. LED apagado.");
  }
  // Se o comando for ligar, acende o LED
  else if (comando == "ON") {
    ESP32Sensors::LED::on();
    bombaLigada = true;
    Serial.println("COMANDO: ligar bomba. LED aceso.");
  }
  // Outros comandos podem ser tratados aqui
}



