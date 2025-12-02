// Cabeçalho agregador para inicializar todos os sensores usados nesta aplicação
#include "ESP32SensorsAmbiente.hpp"  // DHT22 (temp/umid)
#include "ESP32SensorsLED.hpp"       // LED indicador (simula bomba)
#include "ESP32SensorsDryness.hpp"   // Potenciômetro (simula sensor de umidade do solo)

namespace ESP32Sensors {
    void beginAll(uint8_t dhtPin, uint8_t dhtModel, uint8_t ledPin, uint8_t drynessPin) {
        Ambiente::inicializar(dhtPin, dhtModel);
        LED::inicializar(ledPin);
        Dryness::inicializar(drynessPin);
    }
}
