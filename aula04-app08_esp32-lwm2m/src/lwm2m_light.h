// lwm2m_light.h
#ifndef LWM2M_LIGHT_H
#define LWM2M_LIGHT_H

#include <Arduino.h>

// Object 3311: Light Control
// Resources:
// - 5850: On/Off (R/W, Boolean, Mandatory)
// - 5851: Dimmer (R/W, Integer 0-100, Optional)
// - 5706: Colour (R/W, String, Optional)

// Estrutura para cada LED
typedef struct {
    uint8_t pin;
    bool state;
    uint8_t dimmer;
    uint8_t pwmChannel;
    const char* color;
} light_instance_t;

// Funções públicas
void light_init(uint8_t redPin, uint8_t bluePin, uint8_t yellowPin);
void light_set_onoff(uint8_t instance, bool state);
bool light_get_onoff(uint8_t instance);
void light_set_dimmer(uint8_t instance, uint8_t value);
uint8_t light_get_dimmer(uint8_t instance);
const char* light_get_colour(uint8_t instance);

#endif // LWM2M_LIGHT_H