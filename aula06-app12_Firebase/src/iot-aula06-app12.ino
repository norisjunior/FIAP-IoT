/* ==== INCLUDES ===================================================== */
#include <WiFi.h>
#include <Firebase_ESP_Client.h>      // Biblioteca "Firebase ESP Client" (Mobizt)
#include "ESP32Sensors.hpp"          // LED, Distância (HC-SR04) e Ambiente (DHT22)

/* ==== CREDENCIAIS =================================================== */
const char* WIFI_SSID     = "Wokwi-GUEST";   // Rede pública do simulador
const char* WIFI_PASSWORD = "";

//AIzaSyCqSUfVwtK-c6owoEsWqR3q2PZZhP9KLbU
const char* API_KEY       = "";
const char* DB_URL        = "https://fiap-iot-2025-default-rtdb.firebaseio.com/"; // sem barra final
//profnoris.junior@fiap.com.br
const char* USER_EMAIL    = "";
//FIAPIoT20251234
const char* USER_PASSWORD = "";

/* ==== OBJETOS GLOBAIS (Firebase) =================================== */
FirebaseData   fbdo;
FirebaseAuth   auth;
FirebaseConfig config;

/* ==== FUNÇÃO AUXILIAR: gravação no Realtime DB ============== */
void enviaParaFirebase(float dist_cm, bool dist_alarme, float temp, float umid, float ic) {
  // Garante conexão ativa
  if (!Firebase.ready()) {
    Serial.println("Firebase não está pronto! Medições não enviadas.");
    return;
  }

  const String base = "/iotAula";             // Nó principal
  Firebase.RTDB.setFloat(&fbdo, base + "/distancia_cm",  dist_cm);
  Firebase.RTDB.setFloat(&fbdo, base + "/distancia_alarme",  dist_alarme);
  Firebase.RTDB.setFloat(&fbdo, base + "/temperatura_C", temp);
  Firebase.RTDB.setFloat(&fbdo, base + "/umidade_%",     umid);
  Firebase.RTDB.setFloat(&fbdo, base + "/indice_calor_C", ic);
}

/* ==== VARIÁVEIS DE CONTROLE ====================================== */
const uint32_t INTERVALO_COLETA = 5000;   // 5 segundos
uint32_t proximaLeitura     = 0;

/* ==== SETUP ========================================================= */
void setup() {
  Serial.begin(115200);

  ESP32Sensors::beginAll();

  // Wi-Fi ---------------------------------------------------------------
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando-se ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(350);
  }
  Serial.println("");
  Serial.println("Wi-Fi conectado.");

  // Firebase -----------------------------------------------------------
  config.api_key      = API_KEY;
  config.database_url = DB_URL;
  auth.user.email     = USER_EMAIL;
  auth.user.password  = USER_PASSWORD;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  ESP32Sensors::LED::off();           // Garante LED apagado no início
}

/* ==== LOOP PRINCIPAL =============================================== */
void loop() {
  if (millis() < proximaLeitura) return;   // Aguarda próximo ciclo
  proximaLeitura = millis() + INTERVALO_COLETA;

  // Leitura dos sensores ------------------------------------------
  ESP32Sensors::Distancia::DISTANCIA fireDist = ESP32Sensors::Distancia::medirDistancia();
  ESP32Sensors::Ambiente::AMBIENTE fireAmb = ESP32Sensors::Ambiente::medirAmbiente();
  
  if (!fireAmb.valido) {
    Serial.println("[ERRO] Leitura DHT inválida.");
    return;                                   // Pula ciclo se houver erro
  }

  // Feedback no Serial ---------------------------------------------
  Serial.printf("Dist: %.2f cm | Dist_alarme: %d | T: %.2f °C | U: %.2f %% | IC: %.2f °C\n",
                fireDist.cm, fireDist.alarmar, fireAmb.temp, fireAmb.umid, fireAmb.ic);
  Serial.println("");

  // Envio ao Firebase com LED de status -----------------------------
  ESP32Sensors::LED::on();
  enviaParaFirebase(fireDist.cm, fireDist.alarmar, fireAmb.temp, fireAmb.umid, fireAmb.ic);
  ESP32Sensors::LED::off();
}
