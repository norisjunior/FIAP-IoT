// lwm2m_temperature.cpp
#include "lwm2m_temperature.h"
#include "config.h"

LWM2MTemperature::LWM2MTemperature(uint8_t pin, uint8_t inst) {
    dht = new DHT(pin, DHT_TYPE);
    instance = inst;
    currentValue = 0.0;
    minValue = 999.0;
    maxValue = -999.0;
    lastUpdate = 0;
    initialized = false;
    pin_ = pin;  // Armazenar o pino
}

LWM2MTemperature::~LWM2MTemperature() {
    delete dht;
}

void LWM2MTemperature::begin() {
    dht->begin();
    delay(2000); // DHT22 precisa de tempo para estabilizar
    initialized = true;
    update(); // Primeira leitura
    
    Serial.printf("🌡️ Sensor de temperatura %d inicializado no pino %d\n", 
                  instance, pin_);
}

void LWM2MTemperature::update() {
    // Atualizar no máximo a cada 2 segundos
    if (millis() - lastUpdate < 2000) return;
    
    float temp = dht->readTemperature();
    
    if (!isnan(temp)) {
        currentValue = temp;
        
        // Atualizar min/max
        if (temp < minValue) minValue = temp;
        if (temp > maxValue) maxValue = temp;
        
        lastUpdate = millis();
        
        #ifdef DEBUG_SENSORS
        // Removido para não poluir - descomentar se precisar debug
        // Serial.printf("🌡️ [%d] Temperatura: %.1f°C (min: %.1f, max: %.1f)\n", 
        //               instance, currentValue, minValue, maxValue);
        #endif
    }
}

float LWM2MTemperature::getValue() {
    update();
    return currentValue;
}

const char* LWM2MTemperature::getUnits() {
    return "Celsius";
}

float LWM2MTemperature::getMinValue() {
    return minValue;
}

float LWM2MTemperature::getMaxValue() {
    return maxValue;
}

void LWM2MTemperature::resetMinMax() {
    minValue = currentValue;
    maxValue = currentValue;
    Serial.printf("🔄 [%d] Min/Max resetados para %.1f°C\n", instance, currentValue);
}