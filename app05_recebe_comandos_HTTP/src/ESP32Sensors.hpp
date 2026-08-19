// Inclui os cabeçalhos dos módulos para que as funções sejam visíveis
#include "ESP32SensorsAudio.hpp"
#include "ESP32SensorsLED.hpp"

namespace ESP32Sensors {
	void beginAll() {
		Audio::inicializar();
		LED::inicializar();
	}
}