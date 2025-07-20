// lwm2m_objects_manager.cpp
#include "lwm2m_objects_manager.h"
#include "config.h"

// Instância global
LWM2MObjectsManager objectsManager;

LWM2MObjectsManager::LWM2MObjectsManager() {
    // Criar sensores de temperatura
    temperature[0] = new LWM2MTemperature(DHT_PIN_0, 0);
    temperature[1] = new LWM2MTemperature(DHT_PIN_1, 1);
    
    // Criar sensores de umidade (mesmos pinos dos DHT)
    humidity[0] = new LWM2MHumidity(DHT_PIN_0, 0);
    humidity[1] = new LWM2MHumidity(DHT_PIN_1, 1);
}

LWM2MObjectsManager::~LWM2MObjectsManager() {
    delete temperature[0];
    delete temperature[1];
    delete humidity[0];
    delete humidity[1];
}

void LWM2MObjectsManager::begin() {
    Serial.println("\n🔧 Inicializando objetos IPSO...");
    
    // Inicializar sensores de temperatura
    temperature[0]->begin();
    temperature[1]->begin();
    
    // Inicializar sensores de umidade
    humidity[0]->begin();
    humidity[1]->begin();
    
    Serial.println("✅ Objetos IPSO prontos!");
    
    // Mostrar status inicial
    printStatus();
}

void LWM2MObjectsManager::update() {
    // Atualizar sensores
    temperature[0]->update();
    temperature[1]->update();
    humidity[0]->update();
    humidity[1]->update();
}

bool LWM2MObjectsManager::processRead(uint16_t objectId, uint16_t instanceId, 
                                      uint16_t resourceId, String& value) {
    switch (objectId) {
        case 3303: // Temperature
            if (instanceId < 2) {
                switch (resourceId) {
                    case 5700: // Sensor Value
                        value = String(temperature[instanceId]->getValue(), 1);
                        return true;
                    case 5701: // Units
                        value = temperature[instanceId]->getUnits();
                        return true;
                    case 5601: // Min Value
                        value = String(temperature[instanceId]->getMinValue(), 1);
                        return true;
                    case 5602: // Max Value
                        value = String(temperature[instanceId]->getMaxValue(), 1);
                        return true;
                }
            }
            break;
            
        case 3304: // Humidity
            if (instanceId < 2) {
                switch (resourceId) {
                    case 5700: // Sensor Value
                        value = String(humidity[instanceId]->getValue(), 1);
                        return true;
                    case 5701: // Units
                        value = humidity[instanceId]->getUnits();
                        return true;
                    case 5601: // Min Value
                        value = String(humidity[instanceId]->getMinValue(), 1);
                        return true;
                    case 5602: // Max Value
                        value = String(humidity[instanceId]->getMaxValue(), 1);
                        return true;
                }
            }
            break;
            
        case 3311: // Light Control
            if (instanceId < 3) {
                switch (resourceId) {
                    case 5850: // On/Off
                        value = light_get_onoff(instanceId) ? "1" : "0";
                        return true;
                    case 5851: // Dimmer
                        value = String(light_get_dimmer(instanceId));
                        return true;
                    case 5706: // Colour
                        value = light_get_colour(instanceId);
                        return true;
                }
            }
            break;
    }
    
    return false;
}

bool LWM2MObjectsManager::processWrite(uint16_t objectId, uint16_t instanceId, 
                                       uint16_t resourceId, const String& value) {
    // Debug do valor recebido
    Serial.printf("📝 processWrite - Object: %d, Instance: %d, Resource: %d\n", 
                  objectId, instanceId, resourceId);
    Serial.printf("📦 Valor recebido (length=%d): ", value.length());
    
    // Mostrar em hex
    for (int i = 0; i < value.length(); i++) {
        Serial.printf("%02X ", (uint8_t)value[i]);
    }
    Serial.println();
    
    // Tentar interpretar o valor
    String processedValue = "";
    
    // Se for formato TLV (começa com byte de tipo)
    if (value.length() >= 3 && (uint8_t)value[0] == 0xE1) {
        // TLV format: Type (1 byte) + Length (1-2 bytes) + Value
        int valueStart = 2;  // Assumindo length de 1 byte
        if ((uint8_t)value[1] & 0x80) {
            // Length em 2 bytes
            valueStart = 3;
        }
        
        // Pegar o valor real
        if (value.length() > valueStart) {
            uint8_t actualValue = (uint8_t)value[valueStart];
            processedValue = String(actualValue);
            Serial.printf("🔍 Valor TLV decodificado: %d\n", actualValue);
        }
    } 
    // Se for texto simples
    else if (value.length() == 1 && (value[0] == '0' || value[0] == '1')) {
        processedValue = value;
        Serial.printf("🔍 Valor texto simples: %s\n", processedValue.c_str());
    }
    // Tentar outros formatos
    else {
        // Pode ser apenas o byte direto
        processedValue = String((uint8_t)value[0]);
        Serial.printf("🔍 Valor direto: %s\n", processedValue.c_str());
    }
    
    switch (objectId) {
        case 3311: // Light Control
            if (instanceId < 3) {
                switch (resourceId) {
                    case 5850: // On/Off
                        {
                            bool newState = (processedValue == "1" || processedValue == "01");
                            light_set_onoff(instanceId, newState);
                            Serial.printf("💡 LED %d: %s\n", instanceId, 
                                        newState ? "ON" : "OFF");
                            return true;
                        }
                    case 5851: // Dimmer
                        {
                            int dimValue = processedValue.toInt();
                            if (dimValue >= 0 && dimValue <= 100) {
                                light_set_dimmer(instanceId, dimValue);
                                Serial.printf("🔆 LED %d dimmer: %d%%\n", 
                                            instanceId, dimValue);
                                return true;
                            }
                        }
                        break;
                }
            }
            break;
    }
    
    return false;
}

bool LWM2MObjectsManager::processExecute(uint16_t objectId, uint16_t instanceId, 
                                         uint16_t resourceId) {
    switch (objectId) {
        case 3303: // Temperature
            if (instanceId < 2 && resourceId == 5605) {
                temperature[instanceId]->resetMinMax();
                return true;
            }
            break;
            
        case 3304: // Humidity
            if (instanceId < 2 && resourceId == 5605) {
                humidity[instanceId]->resetMinMax();
                return true;
            }
            break;
    }
    
    return false;
}

void LWM2MObjectsManager::printStatus() {
    Serial.println("\n📊 === STATUS DOS OBJETOS IPSO ===");
    
    // Status dos LEDs
    Serial.println("💡 Light Control (3311):");
    const char* colors[] = {"Vermelho", "Azul", "Amarelo"};
    for (int i = 0; i < 3; i++) {
        Serial.printf("  [%d] %s: %s, Dimmer: %d%%\n", 
                      i, colors[i],
                      light_get_onoff(i) ? "ON" : "OFF",
                      light_get_dimmer(i));
    }
    
    // Status da temperatura
    Serial.println("🌡️ Temperature (3303):");
    for (int i = 0; i < 2; i++) {
        if (temperature[i]->isValid()) {
            Serial.printf("  [%d] %.1f°C (min: %.1f, max: %.1f)\n",
                          i, temperature[i]->getValue(),
                          temperature[i]->getMinValue(),
                          temperature[i]->getMaxValue());
        } else {
            Serial.printf("  [%d] Aguardando leitura...\n", i);
        }
    }
    
    // Status da umidade
    Serial.println("💧 Humidity (3304):");
    for (int i = 0; i < 2; i++) {
        if (humidity[i]->isValid()) {
            Serial.printf("  [%d] %.1f%% (min: %.1f, max: %.1f)\n",
                          i, humidity[i]->getValue(),
                          humidity[i]->getMinValue(),
                          humidity[i]->getMaxValue());
        } else {
            Serial.printf("  [%d] Aguardando leitura...\n", i);
        }
    }
    
    Serial.println("==================================\n");
}