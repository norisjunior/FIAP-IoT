// Inclui os cabeçalhos dos módulos para que as funções sejam visíveis
#include "ESP32SensorsAccelGyro.hpp"

namespace ESP32Sensors {
	void beginAll(uint8_t sdaPin, uint8_t sclPin) {
		AccelGyro::inicializar(sdaPin, sclPin);
	}
}