// Cabeçalho agregador para inicializar todos os sensores usados nesta aula
#include "ESP32SensorsAmbiente.hpp"  // DHT22 (temp/umid/IC)
#include "ESP32SensorsLED.hpp"       // LED indicador
#include "ESP32SensorsDryness.hpp"       // HR simulado pelo potenciômetro
namespace ESP32Sensors {
  void beginAll(uint8_t dhtPin, uint8_t dhtModel, uint8_t ledPin, uint8_t drynessPin) {
    Ambiente::inicializar(dhtPin, dhtModel);
    LED::inicializar(ledPin);
    Dryness::inicializar(drynessPin);
  }
}
