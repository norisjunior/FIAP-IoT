// Inclui os cabeçalhos dos módulos para que as funções sejam visíveis
#include "ESP32SensorsLED.hpp"

namespace ESP32Sensors {
	void beginAll(uint8_t ledPin) {
		LED::inicializar(ledPin);
	}
}