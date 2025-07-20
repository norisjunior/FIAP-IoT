// lwm2m_coap_fixed.h
#ifndef LWM2M_COAP_FIXED_H
#define LWM2M_COAP_FIXED_H

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"

// Estados do cliente LWM2M
enum lwm2m_state_t {
    LWM2M_STATE_IDLE = 0,
    LWM2M_STATE_REGISTERING,
    LWM2M_STATE_REGISTERED,
    LWM2M_STATE_ERROR
};

// Funções públicas
bool lwm2m_coap_init();
void lwm2m_coap_step();
bool lwm2m_coap_is_registered();
String lwm2m_coap_get_status();

#endif // LWM2M_COAP_FIXED_H