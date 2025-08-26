// lwm2m_objects_manager.h
#ifndef LWM2M_OBJECTS_MANAGER_H
#define LWM2M_OBJECTS_MANAGER_H

#include <Arduino.h>
#include "lwm2m_temperature.h"
#include "lwm2m_humidity.h"
#include "lwm2m_light.h"

// Gerenciador de objetos IPSO
class LWM2MObjectsManager {
private:
    // Sensores de temperatura (3303)
    LWM2MTemperature* temperature[2];
    
    // Sensores de umidade (3304)
    LWM2MHumidity* humidity[2];
    
    // Já temos o light control global
    
public:
    LWM2MObjectsManager();
    ~LWM2MObjectsManager();
    
    void begin();
    void update();
    
    // Processar requisições CoAP
    bool processRead(uint16_t objectId, uint16_t instanceId, 
                     uint16_t resourceId, String& value);
    bool processWrite(uint16_t objectId, uint16_t instanceId, 
                      uint16_t resourceId, const String& value);
    bool processExecute(uint16_t objectId, uint16_t instanceId, 
                        uint16_t resourceId);
    
    // Debug
    void printStatus();
};

// Instância global
extern LWM2MObjectsManager objectsManager;

#endif // LWM2M_OBJECTS_MANAGER_H