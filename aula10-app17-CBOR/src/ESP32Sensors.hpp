// Inclui os cabeçalhos dos módulos para que as funções sejam visíveis
#include "ESP32SensorsAccel.hpp"
#include "ESP32SensorsAmbiente.hpp"
#include "ESP32SensorsLED.hpp"
#include "ESP32SensorsSDCard.hpp"

namespace ESP32Sensors {
	void beginAll() {
		Ambiente::inicializar();
		Accel::inicializar();
		LED::inicializar();
		SDCard::inicializar();
	}
}