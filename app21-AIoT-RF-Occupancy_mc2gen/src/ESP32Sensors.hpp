// Cabeçalho agregador para inicializar todos os sensores usados nesta aula
#include "ESP32SensorsAmbiente.hpp"  // DHT22 (temp/umid/IC)
#include "ESP32SensorsLED.hpp"       // LED indicador
#include "ESP32SensorsLDR.hpp"       // LDR (luminosidade)
#include "ESP32SensorsCO2.hpp"       // CO2 simulado pelo potenciômetro
#include "ESP32SensorsHumidityRatio.hpp"       // HR simulado pelo potenciômetro
namespace ESP32Sensors {
  void beginAll(uint8_t dhtPin, uint8_t dhtModel, uint8_t ledPin, uint8_t ldrPin, uint8_t CO2Pin, uint8_t HRPin) {
    Ambiente::inicializar(dhtPin, dhtModel);
    LED::inicializar(ledPin);
    LDR::inicializar(ldrPin);
    CO2::inicializar(CO2Pin);
    HR::inicializar(HRPin);
  }
}
