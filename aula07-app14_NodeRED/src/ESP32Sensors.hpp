// Inclui os cabeçalhos dos módulos para que as funções sejam visíveis
#include "ESP32SensorsAmbiente.hpp"
#include "ESP32SensorsDistancia.hpp"
#include "ESP32SensorsLED.hpp"
#include "ESP32SensorsPIR.hpp"

namespace ESP32Sensors {
	void beginAll() {
		Ambiente::inicializar();
		Distancia::inicializar();
		PIR::inicializar();
		LED::inicializar();
	}
}