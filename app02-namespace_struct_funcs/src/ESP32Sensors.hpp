// Inclui os cabeçalhos dos módulos para que as funções sejam visíveis
#include "ESP32SensorsAmbiente.hpp"
#include "ESP32SensorsLED.hpp"

namespace ESP32Sensors {
	void beginAll() {
		Ambiente::inicializar();
		LED::inicializar();
	}
}