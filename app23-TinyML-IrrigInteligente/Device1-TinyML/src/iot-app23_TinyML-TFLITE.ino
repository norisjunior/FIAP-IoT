/* ==== INCLUDES ===================================================== */
#include "ESP32Sensors.hpp"              // Ambiente (DHT22), LED, LDR (Lux) e CO2 (ppm)

/* ==== Configurações de Hardware =================================================== */
const uint8_t DHT_PIN     = 26;
const uint8_t DHT_MODEL   = DHT22;
const uint8_t LED_PIN     = 21;
const uint8_t DRYNESS_PIN = 32;

/* ==== CONSTANTES =================================== */
static unsigned long lastMs = 0;
const unsigned long INTERVAL = 5000; // 5s entre inferências

/* ==== FUNÇÃO PRINCIPAL: COLETA + INFERÊNCIA LOCAL =================================== */
bool coletaDados_e_realizaInferencia() {
  // 1. COLETA DE DADOS DOS SENSORES
  ESP32Sensors::Ambiente::AMBIENTE  amb = ESP32Sensors::Ambiente::medirAmbiente();
  ESP32Sensors::Dryness::DRY        dry = ESP32Sensors::Dryness::ler();

  if (!amb.valido || !dry.valido) {
    Serial.println("[ERRO] Leituras inválidas dos sensores!");
    return false;
  }

  // 2. EXIBIR DADOS BRUTOS
  Serial.println("\n========== INFERÊNCIA LOCAL ML ==========");
  Serial.println("[SENSORES] Dados coletados:");
  Serial.printf("  Temperatura: %.2f °C", amb.temp); Serial.println("");
  Serial.printf("  Dryness: %.2f", dry.valor); Serial.println("");
  

  
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
  ESP32Sensors::beginAll(DHT_PIN, DHT_MODEL, LED_PIN, DRYNESS_PIN);
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