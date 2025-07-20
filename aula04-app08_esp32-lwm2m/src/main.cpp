// main.cpp
#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "lwm2m_light.h"
#include "lwm2m_coap_fixed.h"
#include "lwm2m_objects_manager.h"  // IMPORTANTE: incluir o gerenciador de objetos

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n=== LWM2M ESP32 Project ===");
    Serial.println("Versão: Cliente IoT com sensores e atuadores");
    Serial.println("Objetos: 3303 (Temp), 3304 (Umid), 3311 (LEDs)");
    Serial.println("=========================================\n");
    
    // Inicializar Light Control
    light_init(LED_RED_PIN, LED_BLUE_PIN, LED_YELLOW_PIN);
    
    // Inicializar objetos IPSO (sensores DHT)
    objectsManager.begin();
    
    // Teste rápido dos LEDs
    Serial.println("🔦 Testando LEDs...");
    for (int i = 0; i < 3; i++) {
        light_set_onoff(i, true);
        delay(200);
        light_set_onoff(i, false);
    }
    
    // Conectar WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("📡 Conectando ao WiFi");
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        light_set_onoff(2, !light_get_onoff(2)); // LED amarelo piscando
        attempts++;
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\n❌ Falha ao conectar WiFi!");
        light_set_onoff(0, true); // LED vermelho
        while(1) delay(1000);
    }
    
    Serial.println("\n✅ WiFi conectado!");
    Serial.printf("📍 IP Local: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("📶 Sinal: %d dBm\n", WiFi.RSSI());
    light_set_onoff(2, false);
    
    // Aguardar estabilização da rede
    Serial.println("⏳ Aguardando estabilização...");
    delay(3000);
    
    // Inicializar LWM2M
    Serial.println("\n🚀 Inicializando LWM2M...");
    if (lwm2m_coap_init()) {
        Serial.println("✅ Cliente LWM2M pronto!");
        light_set_dimmer(1, 50);
        light_set_onoff(1, true); // LED azul = sistema pronto
    } else {
        Serial.println("❌ Falha na inicialização LWM2M");
        light_set_onoff(0, true); // LED vermelho = erro
        while(1) delay(1000);
    }
    
    Serial.println("\n📋 === INFORMAÇÕES DO SISTEMA ===");
    Serial.printf("Endpoint: %s\n", LWM2M_ENDPOINT_NAME);
    Serial.printf("Lifetime: %d segundos\n", LWM2M_LIFETIME);
    Serial.println("Servidor: leshan.eclipseprojects.io:5683");
    Serial.println("Objetos disponíveis:");
    Serial.println("  - 3303: Temperature (2 instâncias)");
    Serial.println("  - 3304: Humidity (2 instâncias)");
    Serial.println("  - 3311: Light Control (3 instâncias)");
    Serial.println("================================\n");
    
    Serial.println("💡 Dica: Abra https://leshan.eclipseprojects.io");
    Serial.println("        e procure pelo endpoint após o registro!");
    Serial.println("        Você poderá controlar os LEDs e ver os sensores!\n");
}

void loop() {
    static unsigned long lastStatus = 0;
    static unsigned long lastHeartbeat = 0;
    static unsigned long lastSensorUpdate = 0;
    static bool wasRegistered = false;
    static int celebrationCount = 0;
    
    unsigned long currentTime = millis();
    
    // Step do LWM2M (processa comandos do Leshan)
    lwm2m_coap_step();
    
    // Atualizar objetos IPSO (sensores)
    objectsManager.update();
    
    // Status periódico
    if (currentTime - lastStatus >= 10000) {
        lastStatus = currentTime;
        Serial.printf("\n📊 Status: %s", lwm2m_coap_get_status().c_str());
        
        if (lwm2m_coap_is_registered()) {
            Serial.printf(" ✅ [Visível no Leshan]");
        }
        Serial.println();
        
        // Info adicional
        Serial.printf("⏱️  Uptime: %lu segundos\n", currentTime / 1000);
        Serial.printf("📶 WiFi: %d dBm\n", WiFi.RSSI());
    }
    
    // Mostrar valores dos sensores periodicamente
    if (currentTime - lastSensorUpdate >= 30000 && lwm2m_coap_is_registered()) {
        lastSensorUpdate = currentTime;
        objectsManager.printStatus();
    }
    
    // Primeira vez que registra
    if (lwm2m_coap_is_registered() && !wasRegistered) {
        Serial.println("\n🎊 === PRIMEIRA CONEXÃO ESTABELECIDA! ===");
        Serial.println("🎯 DISPOSITIVO REGISTRADO NO LESHAN!");
        Serial.printf("🔍 Endpoint: %s\n", LWM2M_ENDPOINT_NAME);
        Serial.println("📱 Acesse: https://leshan.eclipseprojects.io");
        Serial.println("🎮 Controle disponível:");
        Serial.println("   - LEDs: On/Off e Dimmer");
        Serial.println("   - Sensores: Temperatura e Umidade");
        Serial.println("=======================================\n");
        
        wasRegistered = true;
        celebrationCount = 0;
    }
    
    // Animação de celebração
    if (wasRegistered && celebrationCount < 10) {
        static unsigned long lastCelebration = 0;
        if (currentTime - lastCelebration >= 300) {
            lastCelebration = currentTime;
            
            // Efeito arco-íris nos LEDs
            int pattern = celebrationCount % 3;
            light_set_onoff(0, pattern == 0);
            light_set_onoff(1, pattern == 1);
            light_set_onoff(2, pattern == 2);
            
            celebrationCount++;
            
            if (celebrationCount >= 10) {
                // Estado final: apenas LED azul
                light_set_onoff(0, false);
                light_set_onoff(1, true);
                light_set_onoff(2, false);
                light_set_dimmer(1, 100);
            }
        }
    }
    
    // Indicador visual do estado
    if (lwm2m_coap_is_registered()) {
        // LED azul respirando
        static unsigned long lastBreath = 0;
        static int brightness = 50;
        static int direction = 5;
        
        if (currentTime - lastBreath >= 50) {
            lastBreath = currentTime;
            brightness += direction;
            
            if (brightness >= 100 || brightness <= 20) {
                direction = -direction;
            }
            
            light_set_dimmer(1, brightness);
        }
    } else {
        // LED amarelo piscando enquanto tenta registrar
        static unsigned long lastBlink = 0;
        if (currentTime - lastBlink >= 1000) {
            lastBlink = currentTime;
            light_set_onoff(2, !light_get_onoff(2));
        }
    }
    
    // Heartbeat do sistema (LED vermelho pisca rapidamente)
    if (currentTime - lastHeartbeat >= 5000) {
        lastHeartbeat = currentTime;
        light_set_onoff(0, true);
        delay(30);
        light_set_onoff(0, false);
    }
}