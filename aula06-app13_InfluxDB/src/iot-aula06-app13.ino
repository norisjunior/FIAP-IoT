/* ==== INCLUDES ===================================================== */
#include <WiFi.h>
#include <HTTPClient.h>              // Para requisições HTTP ao InfluxDB
#include "ESP32Sensors.hpp"           // LED, Distância (HC-SR04) e Ambiente (DHT22)

/* ==== CREDENCIAIS =================================================== */
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
const char* INFLUX_TOKEN  = "Tg1KYC6sbSaO_YMorJwEOEXTOYHjL9exDbkavgwD0cw5mfWXlkudjyhL3elfh6wjpti1Em0714nAHBcz8CqVqg==";      // Token de autenticação

/* ==== FUNÇÃO AUXILIAR: Envia dados para InfluxDB =================== */
void enviaParaInfluxDB(float dist_cm, bool dist_alarme, float temp, float umid, float ic) {
  // Monta a string no formato Line Protocol do InfluxDB
  // Formato: measurement,tag=value field1=value1,field2=value2 timestamp
  String dados = "sensores_iot,dispositivo=ESP32_Aula ";  // measurement e tags
  dados += "distancia_cm=" + String(dist_cm, 2) + ",";
  dados += "distancia_alarme=" + String(dist_alarme ? 1 : 0) + ",";
  dados += "temp=" + String(temp, 2) + ",";
  dados += "umid=" + String(umid, 2) + ",";
  dados += "IC=" + String(ic, 2);
  
  // Cria cliente HTTP
  HTTPClient http;
  
  // Monta a URL completa com parâmetros
  String urlCompleta = String(INFLUX_URL) + "?org=" + INFLUX_ORG + "&bucket=" + INFLUX_BUCKET;
  
  // Configura a requisição
  http.begin(urlCompleta);
  http.addHeader("Authorization", "Token " + String(INFLUX_TOKEN));
  http.addHeader("Content-Type", "text/plain");
  
  // Envia os dados via POST
  int httpCode = http.POST(dados);
  
  // Verifica resposta
  if (httpCode == 204) {  // 204 = No Content (sucesso no InfluxDB)
    Serial.println("Dados enviados com sucesso ao InfluxDB!");
  } else {
    Serial.print("[ERRO] ao enviar para InfluxDB. Código HTTP: ");
    Serial.println(httpCode);
    if (httpCode > 0) {
      Serial.println("Resposta: " + http.getString());
    }
  }
  
  // Encerra conexão HTTP
  http.end();
}

/* ==== VARIÁVEIS DE CONTROLE ========================================= */
const uint32_t INTERVALO_COLETA = 5000;   // 5 segundos
uint32_t proximaLeitura = 0;

/* ==== SETUP ========================================================= */
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ESP32 + InfluxDB - Aula IoT ===\n");

  // Inicializa sensores
  ESP32Sensors::beginAll();

  // Conecta ao Wi-Fi --------------------------------------------------
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando-se ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(350);
  }
  Serial.println("Wi-Fi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  
  // Informações sobre InfluxDB ----------------------------------------
  Serial.println("\n=== Configuração InfluxDB ===");
  Serial.print("Bucket: ");
  Serial.println(INFLUX_BUCKET);
  Serial.print("Organização: ");
  Serial.println(INFLUX_ORG);
  Serial.println("=============================\n");

  ESP32Sensors::LED::off();  // Garante LED apagado no início
}

/* ==== LOOP PRINCIPAL ================================================ */
void loop() {
  // Aguarda próximo ciclo de coleta
  if (millis() < proximaLeitura) return;
  proximaLeitura = millis() + INTERVALO_COLETA;

  Serial.println("\n--- Nova Leitura ---");

  // Leitura dos sensores ----------------------------------------------
  ESP32Sensors::Distancia::DISTANCIA leituraDistancia = ESP32Sensors::Distancia::medirDistancia();
  ESP32Sensors::Ambiente::AMBIENTE leituraAmbiente = ESP32Sensors::Ambiente::medirAmbiente();
  
  // Verifica se a leitura do DHT é válida
  if (!leituraAmbiente.valido) {
    Serial.println("[ERRO] Leitura DHT22 inválida. Pulando ciclo.");
    return;
  }

  // Exibe dados no Serial Monitor -------------------------------------
  Serial.println("Dados Coletados:");
  Serial.printf("-> Distância: %.2f cm %s\n", 
                leituraDistancia.cm, 
                leituraDistancia.alarmar ? "(ALARME!)" : "(OK)"); Serial.println("");
  Serial.printf("-> Temperatura: %.2f °C\n", leituraAmbiente.temp); Serial.println("");
  Serial.printf("-> Umidade: %.2f %%\n", leituraAmbiente.umid); Serial.println("");
  Serial.printf("-> Índice de Calor: %.2f °C\n", leituraAmbiente.ic); Serial.println("");

  // Envia ao InfluxDB com indicação visual via LED --------------------
  Serial.print("Enviando para InfluxDB... ");
  ESP32Sensors::LED::on();  // Liga LED durante transmissão
  
  enviaParaInfluxDB(
    leituraDistancia.cm, 
    leituraDistancia.alarmar,
    leituraAmbiente.temp, 
    leituraAmbiente.umid, 
    leituraAmbiente.ic
  );
  
  ESP32Sensors::LED::off(); // Desliga LED após transmissão
}