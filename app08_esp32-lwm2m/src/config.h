// config.h
#ifndef CONFIG_H
#define CONFIG_H

// ===== CONFIGURAÇÕES WiFi =====
#define WIFI_SSID "Wokwi-GUEST"      // Nome da rede WiFi
#define WIFI_PASSWORD ""              // Senha (vazia para Wokwi)

// ===== CONFIGURAÇÕES LWM2M =====
#define LWM2M_SERVER_URI "coap://leshan.eclipseprojects.io:5683"
#define LWM2M_ENDPOINT_NAME "Noris-IoT-Alunos-001"  // MUDE ESTE NOME!
#define LWM2M_LIFETIME 300                     // Tempo de vida em segundos

// ===== HABILITAR/DESABILITAR OBJETOS =====
#define ENABLE_TEMPERATURE_HUMIDITY true  // Mude para true depois de testar

// ===== PINOS DOS LEDs =====
#define LED_RED_PIN    25   // LED Vermelho (3311/0)
#define LED_BLUE_PIN   26   // LED Azul (3311/1)  
#define LED_YELLOW_PIN 27   // LED Amarelo (3311/2)

// ===== PINOS DOS SENSORES DHT =====
#define DHT_PIN_0      19   // Sensor DHT 0 (3303/0 e 3304/0)
#define DHT_PIN_1      18   // Sensor DHT 1 (3303/1 e 3304/1)
#define DHT_TYPE       DHT22

// ===== CONFIGURAÇÕES DE DEBUG =====
#define DEBUG_LWM2M    true
#define DEBUG_SENSORS  true

#endif // CONFIG_H