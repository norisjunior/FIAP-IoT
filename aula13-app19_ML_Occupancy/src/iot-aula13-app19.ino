/* ==== INCLUDES ===================================================== */
#include <WiFi.h>
#include <HTTPClient.h>
#include "ESP32Sensors.hpp"   // Ambiente (DHT22), LED, LDR (Lux) e CO2 (ppm)


/* ==== Configurações de Hardware =================================================== */
const uint8_t DHT_PIN   = 26;
const uint8_t DHT_MODEL = DHT22;
const uint8_t LED_PIN   = 21;
const uint8_t LDR_PIN   = 35;
const uint8_t CO2_PIN   = 34;

/* ==== WI-FI =================================================== */
const char* WIFI_SSID     = "Wokwi-GUEST";   // Rede pública do simulador
const char* WIFI_PASSWORD = "";

// InfluxDB Cloud - Configurações
const char* INFLUX_URL    = "https://us-east-1-1.aws.cloud2.influxdata.com/api/v2/write";
//e044ac59f07be199
const char* INFLUX_ORG    = "e044ac59f07be199";     // Substituir pela sua organização
//IoTSensores
const char* INFLUX_BUCKET = "IoTSensores";        // Nome do bucket criado
//Tg1KYC6sbSaO_YMorJwEOEXTOYHjL9exDbkavgwD0cw5mfWXlkudjyhL3elfh6wjpti1Em0714nAHBcz8CqVqg==
//KXTPf0peaYQU-QMGu-yJNWwVbBLNoUMmNwBBsrfcnK5GseDHLs_QZx7hNW4sToLnp1qeEXu5CwUq6rwf30FcXQ==
const char* INFLUX_TOKEN  = "KXTPf0peaYQU-QMGu-yJNWwVbBLNoUMmNwBBsrfcnK5GseDHLs_QZx7hNW4sToLnp1qeEXu5CwUq6rwf30FcXQ==";      // Token de autenticação

/* ==== IDENTIDADE DO DISPOSITIVO =================================== */
const char* DISPOSITIVO = "Noris_ESP32_Aula13"; // tag para análise

/* ==== CONSTANTES =================================== */
static unsigned long lastMs = 0;
const unsigned long INTERVAL = 30000; // 30s entre envios


/* ==== MOCK: HumidityRatio ========================================= */
float humidityRatioMock() {
  static bool seeded = false;
  if (!seeded) { randomSeed(esp_random()); seeded = true; }
  const float MIN_HR = 0.002674f;
  const float MAX_HR = 0.006476f;
  long r = random(0, 10000);        // 0..9999
  float u = r / 9999.0f;            // 0..1
  return MIN_HR + u * (MAX_HR - MIN_HR);
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
  } else {
    Serial.println("Falha ao conectar Wi-Fi.");
  }
}

String buildLineProtocol() {
  ESP32Sensors::Ambiente::AMBIENTE  amb = ESP32Sensors::Ambiente::medirAmbiente();
  ESP32Sensors::LDR::DADOS_LDR      luz = ESP32Sensors::LDR::ler();
  ESP32Sensors::CO2::DADOS_CO2      co2 = ESP32Sensors::CO2::ler();

  if (!amb.valido || !luz.valido || !co2.valido) {
    Serial.println("Leituras inválidas: sem envio de dados!");
    return String("");
  }

  float hr = humidityRatioMock();

  // Campos exigidos: Temperature; Humidity; Light; CO2; HumidityRatio
  String dados = "ML_occupancy,dispositivo=" + String(DISPOSITIVO);
  dados += " ";
  dados += "Temperature="    + String(amb.temp, 2);
  dados += ",Humidity="      + String(amb.umid, 2);
  dados += ",Light="         + String(luz.lux, 1);
  dados += ",CO2="           + String(co2.ppm, 1);
  dados += ",HumidityRatio=" + String(hr, 6);
  return dados;
}

bool postToInflux(const String& line) {
  if (line.isEmpty()) return false;

  String url = String(INFLUX_URL) + "?org=" + INFLUX_ORG + "&bucket=" + INFLUX_BUCKET + "&precision=s";
  HTTPClient http;
  http.begin(url);
  http.addHeader("Authorization", String("Token ") + INFLUX_TOKEN);
  http.addHeader("Content-Type", "text/plain; charset=utf-8");

  int code = http.POST(line);
  String resp = http.getString();
  http.end();

  Serial.printf("[InfluxDB] POST %d -> %s\n", code, resp.c_str()); Serial.println("");
  return code >= 200 && code < 300;
}

/* ==== SETUP / LOOP ================================================ */
void setup() {
  Serial.begin(115200);
  ESP32Sensors::beginAll(DHT_PIN, DHT_MODEL, LED_PIN, LDR_PIN, CO2_PIN);
  conectarWiFi();
}

void loop() {
  unsigned long now = millis();

  if (now - lastMs >= INTERVAL) {
    Serial.println("Preparando dados para envio ao InfluxDB");
    String buffer = buildLineProtocol();
    if (!buffer.isEmpty()) {
      Serial.println("Início da transmissão...");
      ESP32Sensors::LED::on();
      bool ok = postToInflux(buffer);
      if (ok) { 
        Serial.println(String("Enviado: \n") + buffer);
      } else {
        Serial.println("Falha no envio ao InfluxDB.");
      }
      ESP32Sensors::LED::off();
    }
    lastMs = now; // reprograma próximo envio
  }
  delay(100);
}
