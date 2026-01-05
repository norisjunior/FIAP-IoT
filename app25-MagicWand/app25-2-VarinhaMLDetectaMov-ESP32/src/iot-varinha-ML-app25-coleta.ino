#include <Arduino.h>
#include "ESP32Sensors.hpp"
#include <MagicWand_ESP32_inferencing.h>

/* ---- Configurações ---- */
#define FREQUENCY_HZ        50
#define INTERVAL_MS         (1000 / (FREQUENCY_HZ))

/*
  Como as amostras são coletadas:
    50 Hz = 50 amostras por segundo
    Intervalo entre amostras = 1000ms / 50 = 20ms
    Isso define quanto tempo esperar entre cada leitura do sensor
*/

const static uint8_t SCL_PIN   = 23;
const static uint8_t SDA_PIN   = 22;
const static uint8_t LED_R_PIN = 21;
const static uint8_t LED_B_PIN = 18;
const static uint8_t LED_G_PIN = 19;

static unsigned long last_interval_ms = 0;

// Buffer para inferência (6 eixos * número de amostras)
static float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
static size_t feature_ix = 0;

/*
  Janela de 1200ms (tamanho do buffer -> vetor features[])

  - A janela de 1200ms é preenchida **automaticamente** conforme você coleta amostras
  - 1200ms / 20ms por amostra = **60 amostras**
  - 60 amostras × 6 eixos = **360 valores no buffer**

  Como funciona:
  ```
  Tempo 0ms:    coleta amostra 1  → buffer[0..5]
  Tempo 20ms:   coleta amostra 2  → buffer[6..11]
  Tempo 40ms:   coleta amostra 3  → buffer[12..17]
  ...
  Tempo 1180ms: coleta amostra 60 → buffer[354..359]
  Tempo 1200ms: buffer cheio → FAZ INFERÊNCIA
*/

/* ---- Funções de LEDs ---- */
void leds_off() {
    digitalWrite(LED_R_PIN, LOW);
    digitalWrite(LED_G_PIN, LOW);
    digitalWrite(LED_B_PIN, LOW);
}

void led_cima_baixo() {
    digitalWrite(LED_R_PIN, HIGH);
    digitalWrite(LED_G_PIN, LOW);
    digitalWrite(LED_B_PIN, LOW);
}

void led_circulo() {
    digitalWrite(LED_R_PIN, LOW);
    digitalWrite(LED_G_PIN, HIGH);
    digitalWrite(LED_B_PIN, LOW);
}

void led_lateral_parabaixo() {
    digitalWrite(LED_R_PIN, LOW);
    digitalWrite(LED_G_PIN, LOW);
    digitalWrite(LED_B_PIN, HIGH);
}

void led_descanso_parabaixo() {
    digitalWrite(LED_R_PIN, HIGH);
    digitalWrite(LED_G_PIN, HIGH);
    digitalWrite(LED_B_PIN, HIGH);
}

/* ---- Função de impressão do Edge Impulse ---- */
void ei_printf(const char *format, ...) {
    static char print_buf[256];
    va_list args;
    va_start(args, format);
    int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
    va_end(args);
    if (r > 0) {
        Serial.write(print_buf);
    }
}

/* ---- Setup ---- */
void setup() {
    Serial.begin(115200);
    while (!Serial) {;}
    
    Serial.println("\n=== ESP32 + MPU6050 + Edge Impulse ===");
    
    // Configurar LEDs
    pinMode(LED_R_PIN, OUTPUT);
    pinMode(LED_B_PIN, OUTPUT);
    pinMode(LED_G_PIN, OUTPUT);
    
    // Teste dos LEDs
    for (int i = 0; i < 2; i++) {
        digitalWrite(LED_R_PIN, HIGH);
        digitalWrite(LED_B_PIN, HIGH);
        digitalWrite(LED_G_PIN, HIGH);
        delay(500);
        leds_off();
        delay(500);
    }
    
    // Inicializar sensores
    ESP32Sensors::beginAll(SDA_PIN, SCL_PIN);
    
    // Verificar configuração do modelo
    if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != 6) {
        Serial.println("ERRO: Modelo deve ter 6 eixos (ax,ay,az,gx,gy,gz)");
        while (1) {;}
    }
    
    Serial.println("\nPronto para detectar gestos!");
    Serial.print("Tamanho do buffer: ");
    Serial.println(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
}

/* ---- Loop principal ---- */
void loop() {
    if (millis() > last_interval_ms + INTERVAL_MS) {
        last_interval_ms = millis();

        // Ler sensor
        ESP32Sensors::AccelGyro::DADOS dados = ESP32Sensors::AccelGyro::medirAccelGyro();

        // Preencher buffer (ax, ay, az, gx, gy, gz)
        if (feature_ix + 6 <= EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
            features[feature_ix++] = dados.accel.acceleration.x;
            features[feature_ix++] = dados.accel.acceleration.y;
            features[feature_ix++] = dados.accel.acceleration.z;
            features[feature_ix++] = dados.gyro.gyro.x;
            features[feature_ix++] = dados.gyro.gyro.y;
            features[feature_ix++] = dados.gyro.gyro.z;
        }

        // Quando o buffer estiver cheio, fazer inferência
        if (feature_ix == EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {

            // Criar sinal para o classificador
            signal_t signal;
            numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);

            // Rodar inferência
            ei_impulse_result_t result;
            EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
            
            if (res != EI_IMPULSE_OK) {
                ei_printf("Erro na inferência: %d\n", res);
                feature_ix = 0;
                return;
            }

            // Mostrar tempos
            ei_printf("\nTiming -> DSP: %d ms, Class: %d ms\n",
                      result.timing.dsp,
                      result.timing.classification);

            // Encontrar maior probabilidade
            float best_val = 0.0f;
            size_t best_ix = 0;
            
            for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
                float v = result.classification[ix].value;
                const char *lab = result.classification[ix].label;
                ei_printf("  %s: %.3f\n", lab, v);
                
                if (v > best_val) {
                    best_val = v;
                    best_ix = ix;
                }
            }

            // Pegar o melhor gesto
            const char *gesto = result.classification[best_ix].label;
            ei_printf("=> Gesto detectado: %s (%.2f%%)\n", gesto, best_val * 100);

            // Acionar LED correspondente (threshold de 60%)
            if (best_val < 0.6f) {
                leds_off();
            } else {
                if (strcmp(gesto, "cima-baixo") == 0) {
                    led_cima_baixo();
                } else if (strcmp(gesto, "circulo") == 0) {
                    led_circulo();
                } else if (strcmp(gesto, "lateral-parabaixo") == 0) {
                    led_lateral_parabaixo();
                } else if (strcmp(gesto, "descanso-parabaixo") == 0) {
                    led_descanso_parabaixo();
                } else {
                    leds_off();
                }
            }

            // Resetar buffer para próxima janela
            feature_ix = 0;
        }
    }
}