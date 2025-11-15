/* ==== INCLUDES ===================================================== */
#include "ESP32Sensors.hpp"              // Ambiente (DHT22), LED, LDR (Lux) e CO2 (ppm)
#include "AIoTOccupancyRFScaler.hpp"     // StandardScaler para normalização
#include "AIoTRandomForest_mc2gen.hpp"   // Modelo Random Forest mc2gen

/* ==== Configurações de Hardware =================================================== */
const uint8_t DHT_PIN   = 26;
const uint8_t DHT_MODEL = DHT22;
const uint8_t LED_PIN   = 21;
const uint8_t LDR_PIN   = 35;
const uint8_t CO2_PIN   = 34;

/* ==== CONSTANTES =================================== */
static unsigned long lastMs = 0;
const unsigned long INTERVAL = 5000; // 5s entre inferências

/* ==== ESTATÍSTICAS DO MODELO =================================== */
struct ModelStats {
  unsigned long totalInferencias = 0;
  unsigned long salaOcupada = 0;
  unsigned long salaVazia = 0;
} stats;

/* ==== MOCK: HumidityRatio ========================================= */
float humidityRatioMock() {
  static bool seeded = false;
  if (!seeded) { randomSeed(esp_random()); seeded = true; }
  const float MIN_HR = 0.002674f;
  const float MAX_HR = 0.006476f;
  long r = random(0, 10000);
  float u = r / 9999.0f;
  return MIN_HR + u * (MAX_HR - MIN_HR);
}

/* ==== FUNÇÃO PRINCIPAL: COLETA + INFERÊNCIA LOCAL =================================== */
bool coletaDados_e_realizaInferencia() {
  // 1. COLETA DE DADOS DOS SENSORES
  ESP32Sensors::Ambiente::AMBIENTE  amb = ESP32Sensors::Ambiente::medirAmbiente();
  ESP32Sensors::LDR::DADOS_LDR      luz = ESP32Sensors::LDR::ler();
  ESP32Sensors::CO2::DADOS_CO2      co2 = ESP32Sensors::CO2::ler();

  if (!amb.valido || !luz.valido || !co2.valido) {
    Serial.println("[ERRO] Leituras inválidas dos sensores!");
    return false;
  }

  float hr = humidityRatioMock();

  // 2. EXIBIR DADOS BRUTOS
  Serial.println("\n========== INFERÊNCIA LOCAL ML ==========");
  Serial.println("[SENSORES] Dados coletados:");
  Serial.printf("  Temperature: %.2f °C", amb.temp); Serial.println("");
  Serial.printf("  Humidity: %.2f %%", amb.umid); Serial.println("");
  Serial.printf("  Light: %.1f lux", luz.lux); Serial.println("");
  Serial.printf("  CO2: %.1f ppm", co2.ppm); Serial.println("");
  Serial.printf("  HumidityRatio: %.6f", hr); Serial.println("");
  
  // 3. PREPARAR DADOS PARA O MODELO
  float dadosBrutos[5] = {
    amb.temp,    // Temperature
    amb.umid,    // Humidity
    luz.lux,     // Light
    co2.ppm,     // CO2
    hr           // HumidityRatio
  };
  
  // 4. NORMALIZAÇÃO (StandardScaler)
  float dadosPadronizados[5];
  Scaler::std(dadosBrutos, dadosPadronizados);
  
  Serial.println("\n[NORMALIZAÇÃO] Dados processados:");
  Serial.printf("  Temp: %.2f | Hum: %.2f | Light: %.2f | CO2: %.3f | HR: %.3f",
                dadosPadronizados[0], dadosPadronizados[1], 
                dadosPadronizados[2], dadosPadronizados[3], 
                dadosPadronizados[4]); Serial.println("");
  
  // 5. INFERÊNCIA COM RANDOM FOREST
  double inputModelo[5];
  for(int i = 0; i < 5; i++) {
    inputModelo[i] = (double)dadosPadronizados[i];
  }
  
  double probabilidades[2];
  RandomForest::score(inputModelo, probabilidades);
  
  // 6. INTERPRETAÇÃO DOS RESULTADOS
  // O modelo retorna probabilidades para [classe_0 (vazia), classe_1 (ocupada)]
  int predicao = (probabilidades[1] > probabilidades[0]) ? 1 : 0;
  
  const char* statusSala = (predicao == 1) ? "OCUPADA" : "VAZIA";
  
  // 7. ATUALIZAR ESTATÍSTICAS
  stats.totalInferencias++;
  if (predicao == 1) {
    stats.salaOcupada++;
    ESP32Sensors::LED::on();
  } else {
    stats.salaVazia++;
    ESP32Sensors::LED::off();
  }
  
  // 8. EXIBIR RESULTADO DA INFERÊNCIA
  Serial.println("\n[RESULTADO ML]");
  Serial.printf("  --> Sala: %s", statusSala); Serial.println("");
  Serial.printf("  --> Probabilidades: Vazia=%.1f%% | Ocupada=%.1f%%", 
                probabilidades[0] * 100, probabilidades[1] * 100); Serial.println("");
  Serial.printf("  --> LED: %s", (predicao == 1) ? "LIGADO" : "DESLIGADO"); Serial.println("\n");
  
  // 9. EXIBIR ESTATÍSTICAS ACUMULADAS
  Serial.printf("[STATS] Total: %lu | Quantas vezes ocupada: %lu (%.1f%%) | Quantas vezes vazia: %lu (%.1f%%)\n",
                stats.totalInferencias,
                stats.salaOcupada, (float)stats.salaOcupada/stats.totalInferencias * 100,
                stats.salaVazia, (float)stats.salaVazia/stats.totalInferencias * 100);  Serial.println("");
  
  Serial.println("==========================================\n");
  
  return true;
}

/* ==== SETUP / LOOP ================================================ */
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n==========================================");
  Serial.println("ESP32 - AIoT - ML EMBARCADO (100% LOCAL)");
  Serial.println("Modelo: Random Forest - Ocupação de Sala");
  Serial.println("Modo: SEM CONECTIVIDADE (offline)");
  Serial.println("Biblioteca: MC2GEN");
  Serial.println("==========================================");
  
  // Inicializar sensores
  ESP32Sensors::beginAll(DHT_PIN, DHT_MODEL, LED_PIN, LDR_PIN, CO2_PIN);
  Serial.println("Sensores inicializados!");
  Serial.println("Sistema pronto para inferências locais.\n");
  
  delay(2000);
}

void loop() {
  unsigned long now = millis();
  
  if (now - lastMs >= INTERVAL) {
    coletaDados_e_realizaInferencia();
    lastMs = now;
  }
  
  delay(100);
}