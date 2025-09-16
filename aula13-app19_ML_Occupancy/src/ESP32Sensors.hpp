// Cabeçalho agregador para inicializar todos os sensores usados nesta aula
#include "ESP32SensorsAmbiente.hpp"  // DHT22 (temp/umid/IC)
#include "ESP32SensorsLED.hpp"       // LED indicador
#include "ESP32SensorsLDR.hpp"       // LDR (luminosidade)
#include "ESP32SensorsCO2.hpp"       // CO2 simulado pelo potenciômetro

namespace ESP32Sensors {
  inline void beginAll() {
    Ambiente::inicializar();
    LED::inicializar();
    LDR::inicializar();
    CO2::inicializar();
  }
}
