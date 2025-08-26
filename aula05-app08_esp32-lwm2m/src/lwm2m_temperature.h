// lwm2m_temperature.h
#ifndef LWM2M_TEMPERATURE_H
#define LWM2M_TEMPERATURE_H

#include <Arduino.h>
#include <DHT.h>

// Object 3303: Temperature Sensor
// Resources:
// - 5700: Sensor Value (R, Float, Mandatory)
// - 5701: Sensor Units (R, String, Optional) 
// - 5601: Min Measured Value (R, Float, Optional)
// - 5602: Max Measured Value (R, Float, Optional)
// - 5605: Reset Min and Max (E, Optional)

class LWM2MTemperature {
private:
    DHT* dht;
    uint8_t instance;
    uint8_t pin_;  // Armazenar o pino
    float currentValue;
    float minValue;
    float maxValue;
    unsigned long lastUpdate;
    bool initialized;
    
public:
    LWM2MTemperature(uint8_t pin, uint8_t inst);
    ~LWM2MTemperature();
    
    void begin();
    void update();
    
    // LWM2M Operations
    float getValue();           // Resource 5700
    const char* getUnits();     // Resource 5701
    float getMinValue();        // Resource 5601
    float getMaxValue();        // Resource 5602
    void resetMinMax();         // Resource 5605
    
    // Getters
    uint8_t getInstance() { return instance; }
    bool isValid() { return initialized && !isnan(currentValue); }
};

#endif // LWM2M_TEMPERATURE_H