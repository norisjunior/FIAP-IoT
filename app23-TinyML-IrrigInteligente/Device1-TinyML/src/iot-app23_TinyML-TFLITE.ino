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

// TensorFlow Lite
#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

/* ==== Configurações de Hardware =========================================== */
const uint8_t DHT_PIN     = 26;
const uint8_t DHT_MODEL   = DHT22;
const uint8_t LED_PIN     = 21;
const uint8_t DRYNESS_PIN = 32;

/* ==== TensorFlow Lite - Variáveis Globais ================================= */
const int kTensorArenaSize = 8 * 1024;
uint8_t* tensorArena = nullptr;
tflite::AllOpsResolver resolver;
const tflite::Model* modelo = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* tensorEntrada = nullptr;
TfLiteTensor* tensorSaida = nullptr;

/* ==== Controle de Tempo =================================================== */
unsigned long lastMs = 0;
const unsigned long INTERVAL = 5000;

/* ==== Estatísticas ======================================================== */
int totalInferencias = 0;
int bombaLigada = 0;

/* ==== SETUP =============================================================== */
void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n==========================================");
    Serial.println("  ESP32 TinyML - IRRIGAÇÃO INTELIGENTE");
    Serial.println("==========================================\n");

    // Inicializar sensores
    ESP32Sensors::beginAll(DHT_PIN, DHT_MODEL, LED_PIN, DRYNESS_PIN);
    Serial.println("Sensores OK!");

    // --- INICIALIZAR TENSORFLOW LITE ---
    Serial.println("Carregando modelo TFLite...");

    // 1. Alocar arena de memória
    tensorArena = (uint8_t*)heap_caps_malloc(kTensorArenaSize, MALLOC_CAP_8BIT);
    if (!tensorArena) {
        Serial.println("ERRO: Falha ao alocar memória!");
        while(1);
    }

    // 2. Carregar modelo
    modelo = tflite::GetModel(modelo_irrig_inteligente);
    if (modelo->version() != TFLITE_SCHEMA_VERSION) {
        Serial.println("ERRO: Versão do modelo incompatível!");
        while(1);
    }

    // 3. Criar interpretador
    static tflite::MicroInterpreter staticInterpreter(modelo, resolver, tensorArena, kTensorArenaSize);
    interpreter = &staticInterpreter;

    // 4. Alocar tensores
    if (interpreter->AllocateTensors() != kTfLiteOk) {
        Serial.println("ERRO: Falha ao alocar tensores!");
        while(1);
    }

    // 5. Obter ponteiros de entrada/saída
    tensorEntrada = interpreter->input(0);
    tensorSaida = interpreter->output(0);

    Serial.printf("Modelo OK! Arena usada: %d bytes\n", interpreter->arena_used_bytes());
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
    ESP32Sensors::Dryness::DRY dry = ESP32Sensors::Dryness::ler();

    if (!amb.valido || !dry.valido) {
        Serial.println("[ERRO] Leitura inválida!");
        return;
    }

    // 2. PREENCHER TENSOR DE ENTRADA
    tensorEntrada->data.f[0] = dry.valor;  // dryness (0-1023)
    tensorEntrada->data.f[1] = amb.temp;   // temperatura (°C)

    // 3. EXECUTAR INFERÊNCIA
    if (interpreter->Invoke() != kTfLiteOk) {
        Serial.println("[ERRO] Falha na inferência!");
        return;
    }

    // 4. LER RESULTADO
    float probabilidade = tensorSaida->data.f[0];
    bool deveIrrigar = (probabilidade >= 0.5);

    // 5. ATUAR
    if (deveIrrigar) {
        ESP32Sensors::LED::on();
        bombaLigada++;
    } else {
        ESP32Sensors::LED::off();
    }
    totalInferencias++;

    // 6. EXIBIR
    Serial.println("\n========== INFERÊNCIA TinyML ==========");
    Serial.printf("Dryness: %.1f | Temp: %.1f°C\n", dry.valor, amb.temp);
    Serial.printf("Prob. Irrigar: %.1f%% -> %s\n", 
                  probabilidade * 100.0, 
                  deveIrrigar ? "LIGAR BOMBA" : "DESLIGADA");
    Serial.printf("Stats: %d/%d (%.1f%% ligada)\n", 
                  bombaLigada, totalInferencias,
                  (bombaLigada * 100.0 / totalInferencias));
    Serial.println("========================================");
}
