/* ===========================================================================
 * ESP32 TinyML - Irrigação Inteligente
 * 
 * Sistema de irrigação autônomo com rede neural embarcada.
 * O modelo TFLite decide se a bomba deve ser ligada com base em:
 *   - dryness: nível de secura do solo (0-1023)
 *   - temp: temperatura ambiente (°C)
 * ===========================================================================*/

/* ==== INCLUDES ============================================================ */
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>

// Aplicação TensorFlow Lite e Sensores
#include <ArduTFLite.h>
#include "ESP32Sensors.hpp"
#include "modelo_irrig_inteligente.h"

/* ==== Configurações de Hardware =========================================== */
const uint8_t DHT_PIN     = 26;
const uint8_t DHT_MODEL   = DHT22;
const uint8_t LED_PIN     = 21;
const uint8_t DRYNESS_PIN = 34;

/* ==== WI-FI ======================+++++==================================== */
const char* WIFI_SSID     = "Wokwi-GUEST";   // Rede pública do simulador
const char* WIFI_PASSWORD = "";
WiFiClient wifiClient;

/* ==== MQTT ============================+++++++++++++======================= */
#define MQTT_HOST       "host.wokwi.internal"
#define MQTT_PORT       1883
#define MQTT_PUB_TOPIC  "FIAPIoT/smartagro/cmd/local"
#define MQTT_DEVICEID   "FIAPIoTapp28Dev1"
PubSubClient mqttClient(wifiClient);
bool wifiConectado = false;
bool mqttConectado = false;
unsigned long ultimaTentativaReconexao = 0;
const unsigned long INTERVALO_RECONEXAO = 5000; // 5 segundos entre tentativas
int proximaLeitura = 0;

/* ==== Controle de Tempo =================================================== */
unsigned long lastMs = 0;
const unsigned long INTERVAL = 30000;
static int countMeasurement = 0;

/* ==== TensorFlow Lite - Variáveis Globais ================================= */
#define ARENA_SIZE (8 * 1024)
uint8_t tensorArena[ARENA_SIZE];
// Variável global que indica se liga ou não a bomba d'água
bool deveIrrigar = false;

/* ==== Protótipos ========================================================== */
void conectarWiFi();
void conectarMQTT();
void enviarDecisaoTFLITE();

/* ==== SETUP =============================================================== */
void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n==========================================");
    Serial.println("  ESP32 TinyML - IRRIGAÇÃO INTELIGENTE");
    Serial.println("==========================================\n");

    // Inicializar sensores (usa suas abstrações .hpp)
    ESP32Sensors::beginAll(DHT_PIN, DHT_MODEL, LED_PIN, DRYNESS_PIN);
    Serial.println("Sensores OK!");

    // --- INICIALIZAR TENSORFLOW LITE ---
    Serial.println("Carregando modelo TFLite...");

    if (!modelInit(modelo_irrig_inteligente_tflite, tensorArena, ARENA_SIZE)) {
        Serial.println("ERRO: Falha ao carregar modelo!");
        while(1);
    }

    Serial.println("Modelo TFLite carregado com sucesso!");

    //Conecta no WiFi
    conectarWiFi();
    
    //Configura MQTT e conecta no Broker
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setKeepAlive(60);     // Manter conexão viva por 60s
    mqttClient.setSocketTimeout(60); // Timeout de 60s  
    mqttClient.setBufferSize(512);   // Buffer adequado considerando o payload gerado

    Serial.println("\nSistema Irrigação Inteligente inicializado!");
    Serial.printf("Envio de dados a cada %d segundos...", (INTERVAL/1000)); Serial.println("");


    Serial.println("\n>> Sistema pronto!\n");
    delay(2000);
}

/* ==== LOOP ================================================================ */
void loop() {
    unsigned long now = millis();

    if (millis() - lastMs < INTERVAL) {
        delay(100);
        return;
    }
    lastMs = millis();

    if (wifiConectado && !mqttClient.connected()) {
      conectarMQTT();
    }

    if (mqttConectado) {
      mqttClient.loop();
    } else {
      Serial.println("MQTT desconectado. Pulando ciclo de leitura.");
      delay(1000);
      return;
    }

    // 1. COLETAR DADOS DOS SENSORES
    ESP32Sensors::Ambiente::AMBIENTE amb = ESP32Sensors::Ambiente::medirAmbiente();
    ESP32Sensors::Dryness::DRY       dry = ESP32Sensors::Dryness::ler();

    if (!amb.valido || !dry.valido) {
        Serial.println("[ERRO] Leitura inválida!");
        return;
    }

    // 2. PREPARAR ENTRADA PARA O MODELO
    // Atenção: aqui vão os valores CRUS (raw), na mesma ordem do treino:
    // [ dryness_raw, temp_raw ]
    modelSetInput(dry.valor, 0);  // 0..1023 (já mapeado no .hpp)
    modelSetInput(amb.temp, 1);   // °C

    // 3. EXECUTAR INFERÊNCIA
    if (!modelRunInference()) {
        Serial.println("[ERRO] Falha na inferência!");
        return;
    }

    // 4. LER RESULTADO (saída única: probabilidade de pump=1)
    float probabilidade = modelGetOutput(0);
    deveIrrigar         = (probabilidade >= 0.5f);

    // 5. ATUAR NO LED (SIMULA A BOMBA D'ÁGUA)
    if (deveIrrigar) {
        ESP32Sensors::LED::on();
    } else {
        ESP32Sensors::LED::off();
    }

    // 6. EXIBIR NO SERIAL
    countMeasurement++;
    Serial.println("\n========== INFERÊNCIA TinyML ==========");
    Serial.printf("Medição nº %d", countMeasurement); Serial.println("");
    Serial.printf("Dryness: %.1f | Temp: %.1f°C", dry.valor, amb.temp); Serial.println("");
    Serial.printf("Prob. Irrigar: %.1f%% -> %s",
        probabilidade * 100.0f, deveIrrigar ? "BOMBA->LIGADA" : "BOMBA->DESLIGADA"); Serial.println("");
    Serial.println("========================================\n");

    //Transmitir mensagem MQTT com a decisão tomada pelo dispositivo
    enviarDecisaoTFLITE();
}


/* ==== AUXILIARES =================================================== */
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

void conectarMQTT() {
  if (!wifiConectado) return;
  
  Serial.printf("[MQTT] Conectando ao broker %s", MQTT_HOST);
  
  int tentativas = 0;
  while (!mqttClient.connected() && tentativas < 3) {
    if (mqttClient.connect(MQTT_DEVICEID)) {
      Serial.println(" --> Conectado ao MQTT Broker!");
      mqttConectado = true;
    } else {
      Serial.printf(" Falha, rc=%d", mqttClient.state());
      tentativas++;
      if (tentativas < 3) {
        Serial.println(" Tentando novamente em 3s...");
        delay(3000);
      }
    }
  }
  
  if (!mqttConectado) {
    Serial.println(" --> Falha final na conexão MQTT");
  }
}

void enviarDecisaoTFLITE() {
  /* === Montagem do JSON === */
  JsonDocument doc;
  doc["dispositivo"]   = MQTT_DEVICEID;
  doc["bomba"] = deveIrrigar;

  String payload;
  serializeJson(doc, payload);

  /* === Publicação MQTT === */
  bool envio = mqttClient.publish(MQTT_PUB_TOPIC, payload.c_str());
  Serial.println(envio ? "[MQTT] Publicado" : "[MQTT] Falha ao publicar");
}