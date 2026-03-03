// Cabeçalho agregador para inicializar todos os sensores usados nesta aula
#include "ESP32SensorsAQI.hpp"  // DHT22 (temp/umid/IC)
namespace ESP32Sensors {
  void beginAll() {
    AQI::inicializar();
  }
}
