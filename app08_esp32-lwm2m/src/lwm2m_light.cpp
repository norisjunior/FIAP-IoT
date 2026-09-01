// lwm2m_light.cpp
#include "lwm2m_light.h"

// Array de instâncias de LED
static light_instance_t lights[3];

// Configuração PWM
static const int PWM_FREQ = 5000;
static const int PWM_RESOLUTION = 8;

// Inicializar sistema de LEDs
void light_init(uint8_t redPin, uint8_t bluePin, uint8_t yellowPin) {
    Serial.println("💡 Inicializando Light Control...");
    
    // Configurar LED vermelho (instância 0)
    lights[0].pin = redPin;
    lights[0].state = false;
    lights[0].dimmer = 100;
    lights[0].pwmChannel = 0;
    lights[0].color = "Red";
    
    // Configurar LED azul (instância 1)
    lights[1].pin = bluePin;
    lights[1].state = false;
    lights[1].dimmer = 100;
    lights[1].pwmChannel = 1;
    lights[1].color = "Blue";
    
    // Configurar LED amarelo (instância 2)
    lights[2].pin = yellowPin;
    lights[2].state = false;
    lights[2].dimmer = 100;
    lights[2].pwmChannel = 2;
    lights[2].color = "Yellow";
    
    // Configurar canais PWM e pinos
    for (int i = 0; i < 3; i++) {
        // Configurar canal PWM
        ledcSetup(lights[i].pwmChannel, PWM_FREQ, PWM_RESOLUTION);
        
        // Anexar o canal ao pino
        ledcAttachPin(lights[i].pin, lights[i].pwmChannel);
        
        // Iniciar com LED desligado
        ledcWrite(lights[i].pwmChannel, 0);
        
        Serial.printf("  [%d] LED %s no pino %d (PWM canal %d)\n", 
                      i, lights[i].color, lights[i].pin, lights[i].pwmChannel);
    }
    
    Serial.println("✅ Light Control inicializado!");
}

// Atualizar o estado físico do LED
static void update_led(uint8_t instance) {
    if (instance >= 3) return;
    
    if (lights[instance].state) {
        // LED ligado - aplicar dimmer
        uint8_t pwmValue = (lights[instance].dimmer * 255) / 100;
        ledcWrite(lights[instance].pwmChannel, pwmValue);
    } else {
        // LED desligado
        ledcWrite(lights[instance].pwmChannel, 0);
    }
}

// Ligar/desligar LED
void light_set_onoff(uint8_t instance, bool state) {
    if (instance >= 3) return;
    
    lights[instance].state = state;
    update_led(instance);
    
    // Log simplificado - removido para não poluir
    // Serial.printf("💡 LED %s (%d) %s\n", lights[instance].color, instance, state ? "ON" : "OFF");
}

// Obter estado do LED
bool light_get_onoff(uint8_t instance) {
    if (instance >= 3) return false;
    return lights[instance].state;
}

// Definir dimmer (0-100%)
void light_set_dimmer(uint8_t instance, uint8_t value) {
    if (instance >= 3) return;
    if (value > 100) value = 100;
    
    lights[instance].dimmer = value;
    update_led(instance);
    
    // Log simplificado - removido para não poluir
    // Serial.printf("🔆 LED %s (%d) dimmer: %d%%\n", lights[instance].color, instance, value);
}

// Obter valor do dimmer
uint8_t light_get_dimmer(uint8_t instance) {
    if (instance >= 3) return 0;
    return lights[instance].dimmer;
}

// Obter cor do LED
const char* light_get_colour(uint8_t instance) {
    if (instance >= 3) return "Unknown";
    return lights[instance].color;
}