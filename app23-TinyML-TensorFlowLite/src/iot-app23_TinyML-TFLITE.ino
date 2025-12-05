/* ===========================================================================
 * ESP32 TinyML - Irrigação Inteligente
 * 
 * Sistema de irrigação autônomo com rede neural embarcada.
 * O modelo TFLite decide se a bomba deve ser ligada com base em:
 *   - dryness: nível de secura do solo (0-1023)
 *   - temp: temperatura ambiente (°C)
 * ===========================================================================*/

/* ==== INCLUDES ============================================================ */
#include "ESP32Sensors.hpp"
#include "modelo_irrig_inteligente.h"

// TensorFlow Lite via ArduTFLite
#include <ArduTFLite.h>

/* ==== Configurações de Hardware =========================================== */
const uint8_t DHT_PIN     = 26;
const uint8_t DHT_MODEL   = DHT22;
const uint8_t LED_PIN     = 21;
const uint8_t DRYNESS_PIN = 34;
const uint8_t SERVO_PIN   = 13;

/* ==== TensorFlow Lite - Variáveis Globais ================================= */
#define ARENA_SIZE (8 * 1024)
uint8_t tensorArena[ARENA_SIZE];

/* ==== Controle de Tempo =================================================== */
unsigned long lastMs = 0;
const unsigned long INTERVAL = 5000;

/* ==== SETUP =============================================================== */
void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n==========================================");
    Serial.println("  ESP32 TinyML - IRRIGAÇÃO INTELIGENTE");
    Serial.println("==========================================\n");

    // Inicializar sensores (usa suas abstrações .hpp)
    ESP32Sensors::beginAll(DHT_PIN, DHT_MODEL, LED_PIN, DRYNESS_PIN, SERVO_PIN);
    Serial.println("Sensores OK!");

    // --- INICIALIZAR TENSORFLOW LITE ---
    Serial.println("Carregando modelo TFLite...");

    if (!modelInit(modelo_irrig_inteligente_tflite, tensorArena, ARENA_SIZE)) {
        Serial.println("ERRO: Falha ao carregar modelo!");
        while(1);
    }

    Serial.println("Modelo TFLite carregado com sucesso!");
    Serial.println("\n>> Sistema pronto!\n");
    delay(2000);
}

/* ==== LOOP ================================================================ */
void loop() {
    if (millis() - lastMs < INTERVAL) {
        delay(100);
        return;
    }
    lastMs = millis();

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
    bool deveIrrigar    = (probabilidade >= 0.5f);

    // 5. LIGAR OU NÃO LIGAR A BOMBA D'ÁGUA
    if (deveIrrigar) {
        ESP32Sensors::LED::on();
        ESP32Sensors::MiniServo::ligar();
    } else {
        ESP32Sensors::LED::off();
        ESP32Sensors::MiniServo::desligar();
    }

    // 6. EXIBIR NO SERIAL
    Serial.println("\n========== INFERÊNCIA TinyML ==========");
    Serial.printf("Dryness: %.1f | Temp: %.1f°C", dry.valor, amb.temp); Serial.println("");
    Serial.printf("Prob. Irrigar: %.1f%% -> %s",
        probabilidade * 100.0f, deveIrrigar ? "BOMBA->LIGADA" : "BOMBA->DESLIGADA"); Serial.println("");
    Serial.println("========================================\n");
}

/*
Para valores baixos e altos de secura, a decisão é clara
Porém, há um momento em que há combinação de features para
tomada de decisão (o momento mais importante e a necessidade de uso de ML):
Secura: 495.0
Quando a temperatura está em 25.0, a bomba estará desligada
Se a temperatura ultrapassar 30.0 em conjunto com a secura do solo, liga a bomba
*/