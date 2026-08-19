#include <Arduino.h>
#include <XIAO_nRF52840_1_inferencing.h>
#include <LSM6DS3.h>
#include <Wire.h>

#define CONVERT_G_TO_MS2 9.80665f

// LEDs do XIAO nRF52840
#define LED_RED    11
#define LED_BLUE   12
#define LED_GREEN  13

LSM6DS3 myIMU(I2C_MODE, 0x6A);

// Buffer para os dados - 6 eixos por amostra
float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
size_t feature_ix = 0;

unsigned long last_interval_ms = 0;
#define INTERVAL_MS (1000 / EI_CLASSIFIER_FREQUENCY)

void setup() {
    Serial.begin(115200);
    
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    
    // Todos LEDs apagados (lógica invertida)
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_BLUE, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    
    if (myIMU.begin() != 0) {
        Serial.println("Erro no IMU!");
        while(1);
    }
    Serial.println("IMU OK. Iniciando inferencia...");
}

void loop() {
    if (millis() > last_interval_ms + INTERVAL_MS) {
        last_interval_ms = millis();
        
        // Lê os 6 eixos na mesma ordem da coleta
        features[feature_ix++] = myIMU.readFloatAccelX() * CONVERT_G_TO_MS2;
        features[feature_ix++] = myIMU.readFloatAccelY() * CONVERT_G_TO_MS2;
        features[feature_ix++] = myIMU.readFloatAccelZ() * CONVERT_G_TO_MS2;
        features[feature_ix++] = myIMU.readFloatGyroX();
        features[feature_ix++] = myIMU.readFloatGyroY();
        features[feature_ix++] = myIMU.readFloatGyroZ();
        
        // Buffer cheio? Classifica!
        if (feature_ix >= EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
            
            signal_t signal;
            numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
            
            ei_impulse_result_t result;
            EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
            
            if (res == EI_IMPULSE_OK) {
                // Encontra o gesto com maior probabilidade
                float max_val = 0;
                int max_idx = 0;
                
                Serial.println("\n--- Resultado ---");
                for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
                    Serial.print(result.classification[ix].label);
                    Serial.print(": ");
                    Serial.println(result.classification[ix].value, 3);
                    
                    if (result.classification[ix].value > max_val) {
                        max_val = result.classification[ix].value;
                        max_idx = ix;
                    }
                }
                
                // Acende LED baseado no gesto (threshold 0.5)
                if (max_val > 0.5) {
                    acendeLED(result.classification[max_idx].label);
                }
            }
            
            feature_ix = 0;  // Reset do buffer
        }
    }
}

void acendeLED(const char* gesto) {
    // Apaga todos
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_BLUE, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    
    // Acende conforme o gesto
    if (strcmp(gesto, "onda") == 0) {
        digitalWrite(LED_RED, LOW);      // Vermelho
    }
    else if (strcmp(gesto, "N") == 0) {
        digitalWrite(LED_BLUE, LOW);     // Azul
    }
    else if (strcmp(gesto, "lateral") == 0) {
        digitalWrite(LED_GREEN, LOW);    // Verde
    }
    else if (strcmp(gesto, "para_cima") == 0) {
        digitalWrite(LED_RED, LOW);      // Amarelo (R+G)
        digitalWrite(LED_GREEN, LOW);
    }
}