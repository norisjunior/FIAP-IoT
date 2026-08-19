#include <Arduino.h>
#include <Wire.h>
#include <LSM6DS3.h>

// Biblioteca gerada pelo Edge Impulse (do ZIP que você extraiu)
#include <XIAO_nRF52840_1_inferencing.h>

// ---------- Configurações de IMU / amostragem ----------
#define CONVERT_G_TO_MS2    9.80665f
#define MAX_ACCEPTED_RANGE  4.0f

#define FREQUENCY_HZ        EI_CLASSIFIER_FREQUENCY
#define INTERVAL_MS         (1000 / (FREQUENCY_HZ + 1))

static unsigned long last_interval_ms = 0;

// Buffer com o tamanho exato esperado pelo modelo
static float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
static size_t feature_ix = 0;

// IMU do XIAO
LSM6DS3 myIMU(I2C_MODE, 0x6A);

// ---------- Pinos do LED RGB ----------
const int LED_R = 11;
const int LED_B = 12;
const int LED_G = 13;

// No XIAO BLE Sense os LEDs são ativos em LOW
void leds_off() {
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, HIGH);
}

void led_onda() {          // Vermelho
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, HIGH);
}

void led_N() {             // Verde
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, HIGH);
}

void led_lateral() {       // Azul
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, LOW);
}

void led_para_cima() {     // Amarelo (R + G)
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, HIGH);
}

// ---------- Função de impressão usada pelo EI ----------
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

// ---------- Setup ----------
void setup() {
    Serial.begin(115200);
    while (!Serial) {;}

    Serial.println("XIAO nRF52840 + Edge Impulse - Gestos IMU");

    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    leds_off();

    if (myIMU.begin() != 0) {
        Serial.println("Erro ao inicializar IMU!");
        while (1) {;}
    } else {
        Serial.println("IMU OK");
    }

    // Conferência: modelo deve ter 6 eixos brutos por amostra
    if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != 6) {
        ei_printf("ERR: EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME deve ser 6 (ax,ay,az,gx,gy,gz)\n");
        while (1) {;}
    }
}

// ---------- Loop principal ----------
void loop() {
    float ax, ay, az;
    float gx, gy, gz;

    if (millis() > last_interval_ms + INTERVAL_MS) {
        last_interval_ms = millis();

        // Acelerômetro em m/s² (igual coleta)
        ax = myIMU.readFloatAccelX() * CONVERT_G_TO_MS2;
        ay = myIMU.readFloatAccelY() * CONVERT_G_TO_MS2;
        az = myIMU.readFloatAccelZ() * CONVERT_G_TO_MS2;

        // Giroscópio em dps (igual coleta)
        gx = myIMU.readFloatGyroX();
        gy = myIMU.readFloatGyroY();
        gz = myIMU.readFloatGyroZ();

        // Preenche o vetor na ordem usada no treinamento
        if (feature_ix + 6 <= EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
            features[feature_ix++] = ax;
            features[feature_ix++] = ay;
            features[feature_ix++] = az;
            features[feature_ix++] = gx;
            features[feature_ix++] = gy;
            features[feature_ix++] = gz;
        }

        // Quando encher o buffer, roda o modelo
        if (feature_ix == EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {

            signal_t signal;
            numpy::signal_from_buffer(features,
                                      EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE,
                                      &signal);

            ei_impulse_result_t result;
            EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

            ei_printf("\nrun_classifier returned: %d\n", res);
            if (res != EI_IMPULSE_OK) {
                feature_ix = 0;
                return;
            }

            ei_printf("Timing -> DSP: %d ms, Class: %d ms, Anom: %d ms\n",
                      result.timing.dsp,
                      result.timing.classification,
                      result.timing.anomaly);

            // Acha o melhor rótulo
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

#if EI_CLASSIFIER_HAS_ANOMALY == 1
            ei_printf("  anomaly: %.3f\n", result.anomaly);
#endif

            const char *best_label = result.classification[best_ix].label;
            ei_printf("=> Gesto: %s (%.2f)\n", best_label, best_val);

            // Threshold para reduzir ruído
            if (best_val < 0.6f) {
                leds_off();
            } else {
                if (strcmp(best_label, "onda") == 0) {
                    led_onda();
                } else if (strcmp(best_label, "N") == 0) {
                    led_N();
                } else if (strcmp(best_label, "lateral") == 0) {
                    led_lateral();
                } else if (strcmp(best_label, "para_cima") == 0) {
                    led_para_cima();
                } else {
                    leds_off();
                }
            }

            // Prepara para a próxima janela
            feature_ix = 0;
        }
    }
}
