// Inclui os cabeçalhos dos módulos para que as funções sejam visíveis
#include "ESP32SensorsAccel.hpp"
#include "ESP32SensorsAmbiente.hpp"
#include "ESP32SensorsLED.hpp"

namespace ESP32Sensors {
	void beginAll(uint8_t dhtPin, uint8_t dhtModel, uint8_t clPin, uint8_t daPin, uint8_t ledPin) {
		Ambiente::inicializar(dhtPin, dhtModel);
		Accel::inicializar(clPin, daPin);
		LED::inicializar(ledPin);
	}
}