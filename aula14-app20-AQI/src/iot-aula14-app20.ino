/* *Includes* */
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "ESP32Sensors.hpp"


/* ---- Config Wi‑Fi ---- */
const char* WIFI_SSID     = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
WiFiClient wifiClient; // Define client WiFi

/* ---- Config MQTT ---- */
#define MQTT_SERVER     "test.mosquitto.org"   //
#define MQTT_PORT       1883
#define MQTT_PUB_TOPIC  "FIAPIoT/aula14/noris/aqi/dados"
// ATENÇÃO: ClientID DEVE SER ÚNICO NO BROKER!
#define MQTT_CLIENT_ID  "IoTDeviceNorisAQI001"
PubSubClient mqttClient(wifiClient);

/* ---- Controle de intervalo de envio ---- */
const int INTERVALO_COLETA = 10000;  // 10 s
int proximaLeitura = 0;

/* ---- Protótipos ---- */
void conectarWiFi();
void conectarMQTT();
bool enviarDadosColetados();

/* ---- MAIN (setup e loop) ---- */
void setup() {
  Serial.begin(115200);
    
  Serial.println("\n\n===================================");
  Serial.println("  SISTEMA DE MONITORAMENTO AQI    ");
  Serial.println("===================================");
  Serial.println("Demonstração: Sensores de Qualidade do Ar");
  Serial.println("===================================\n");

  //Inicializa todos os sensores
  Serial.println("Inicializando sensores AQI...");
  ESP32Sensors::beginAll();

  Serial.println("Sistema AQI ativo");

  //Conecta no WiFi
  conectarWiFi();
  
  //Configura MQTT e conecta no Broker
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  conectarMQTT();

  Serial.println("\nSistema AQI inicializado!");
  Serial.printf("Envio de dados a cada %d segundos...", (INTERVALO_COLETA/1000)); Serial.println("");
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
      Serial.println("\nSUCESSO: Dados AQI enviados para o MQTT Broker");
  } else {
      Serial.println("\nFALHA: Erro ao enviar dados AQI");
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
    } else {
      Serial.print(" Falha, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }  
}

// ===== Envia dados coletados =======
bool enviarDadosColetados() {
  // Lê sensores de qualidade do ar (AQI)
  ESP32Sensors::AQI::DADOSAQI dadosAQI = ESP32Sensors::AQI::medirAQI();

  // Exibe dados no Serial Monitor
  Serial.println("\n========================================");
  Serial.println("DADOS AQI COLETADOS (Far Edge/Extreme Edge):");
  Serial.println("========================================");
  
  /* --- Log no Serial Monitor --- */
  Serial.println("Dados de Qualidade do Ar coletados: ");
  Serial.printf("[PARTICULADOS] - PM2.5: %.2f μg/m³ | PM10: %.2f μg/m³",
                 dadosAQI.PM25, dadosAQI.PM10);
  Serial.println("");
  Serial.printf("[ÓXIDOS NITROGÊNIO] - NO: %.2f μg/m³ | NO2: %.2f μg/m³ | NOx: %.2f μg/m³",
                dadosAQI.NO, dadosAQI.NO2, dadosAQI.NOx);
  Serial.println("");
  Serial.printf("[GASES] - NH3: %.2f μg/m³ | CO: %.2f mg/m³ | SO2: %.2f μg/m³ | O3: %.2f μg/m³",
                dadosAQI.NH3, dadosAQI.CO, dadosAQI.SO2, dadosAQI.O3);
  Serial.println("");
  Serial.printf("[COVs] - Benzene: %.2f μg/m³ | Toluene: %.2f μg/m³ | Xylene: %.2f μg/m³",
                dadosAQI.Benzene, dadosAQI.Toluene, dadosAQI.Xylene);
  Serial.println("");

  
  // Prepara JSON para envio via MQTT
  JsonDocument doc;
  
  doc["device"] = MQTT_CLIENT_ID;
  doc["PM25"] = dadosAQI.PM25;
  doc["PM10"] = dadosAQI.PM10;
  doc["NO"] = dadosAQI.NO;
  doc["NO2"] = dadosAQI.NO2;
  doc["NOx"] = dadosAQI.NOx;
  doc["NH3"] = dadosAQI.NH3;
  doc["CO"] = dadosAQI.CO;
  doc["SO2"] = dadosAQI.SO2;
  doc["O3"] = dadosAQI.O3;
  doc["Benzene"] = dadosAQI.Benzene;
  doc["Toluene"] = dadosAQI.Toluene;
  doc["Xylene"] = dadosAQI.Xylene;

  // Serializa e envia JSON
  String buffer;
  serializeJson(doc, buffer);

  //DEBUGs:
  // TAMANHO DO PAYLOAD
  Serial.print("DEBUG: Tamanho do payload: ");
  Serial.println(buffer.length());
  
  // ESTADO DA CONEXÃO  
  if (!mqttClient.connected()) {
      Serial.print("DEBUG: MQTT desconectado. Estado: ");
      Serial.println(mqttClient.state());
      switch(mqttClient.state()) {
          case -4: Serial.println("Connection Timeout"); break;
          case -3: Serial.println("Connection Lost"); break;
          case -2: Serial.println("Connect Failed"); break;
          case -1: Serial.println("Disconnected"); break;
          case 0:  Serial.println("Connected"); break;
          case 1:  Serial.println("Bad Protocol"); break;
          case 2:  Serial.println("Bad Client ID"); break;
          case 3:  Serial.println("Unavailable"); break;
          case 4:  Serial.println("Bad Credentials"); break;
          case 5:  Serial.println("Unauthorized"); break;
      }
      return false;
  }

  // TAMANHO DO PAYLOAD
  Serial.print("Tamanho do JSON: ");
  Serial.println(buffer.length());
  if (buffer.length() > 1024) {
      Serial.println("AVISO: Payload muito grande!");
  }  

  // IMPRIME O JSON CRIADO:
  Serial.print("PAYLOAD MQTT:");
  Serial.println(buffer.c_str());

  
  bool envio = mqttClient.publish(MQTT_PUB_TOPIC, buffer.c_str());
  
  return envio;
}