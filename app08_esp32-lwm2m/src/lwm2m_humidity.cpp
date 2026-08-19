// lwm2m_humidity.cpp
#include "lwm2m_humidity.h"
#include "config.h"

LWM2MHumidity::LWM2MHumidity(uint8_t pin, uint8_t inst) {
    dht = new DHT(pin, DHT_TYPE);
    instance = inst;
    currentValue = 0.0;
    minValue = 100.0;
    maxValue = 0.0;
    lastUpdate = 0;
    initialized = false;
    pin_ = pin;  // Armazenar o pino
}

LWM2MHumidity::~LWM2MHumidity() {
    delete dht;
}

void LWM2MHumidity::begin() {
    dht->begin();
    delay(2000); // DHT22 precisa de tempo para estabilizar
    initialized = true;
    update(); // Primeira leitura
    
    Serial.printf("💧 Sensor de umidade %d inicializado no pino %d\n", 
                  instance, pin_);  Serial.println("");
}

void LWM2MHumidity::update() {
    // Atualizar no máximo a cada 2 segundos
    if (millis() - lastUpdate < 2000) return;
    
    float humidity = dht->readHumidity();
    
    if (!isnan(humidity)) {
        currentValue = humidity;
        
        // Atualizar min/max
        if (humidity < minValue) minValue = humidity;
        if (humidity > maxValue) maxValue = humidity;
        
        lastUpdate = millis();
        
        #ifdef DEBUG_SENSORS
        // Removido para não poluir - descomentar se precisar debug
        // Serial.printf("💧 [%d] Umidade: %.1f%% (min: %.1f, max: %.1f)\n", 
        //               instance, currentValue, minValue, maxValue);
        #endif
    }
}

float LWM2MHumidity::getValue() {
    update();
    return currentValue;
}

const char* LWM2MHumidity::getUnits() {
    return "%RH";
}

float LWM2MHumidity::getMinValue() {
    return minValue;
}

float LWM2MHumidity::getMaxValue() {
    return maxValue;
}

void LWM2MHumidity::resetMinMax() {
    minValue = currentValue;
    maxValue = currentValue;
    Serial.printf("🔄 [%d] Min/Max resetados para %.1f%%\n", instance, currentValue);  Serial.println("");
}