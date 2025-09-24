// Inclui os cabeçalhos dos módulos para que as funções sejam visíveis
#include "ESP32SensorsAmbiente.hpp"
#include "ESP32SensorsDistancia.hpp"
#include "ESP32SensorsLED.hpp"
#include "ESP32SensorsPIR.hpp"

namespace ESP32Sensors {
	void beginAll(uint8_t dhtPin, uint8_t dhtModel, uint8_t distTPin, uint8_t distEPin, float distLimiar, uint8_t ledPin, uint8_t pirPin) {
		Ambiente::inicializar(dhtPin, dhtModel);
		Distancia::inicializar(distTPin, distEPin, distLimiar);
		Movimento::inicializar(pirPin);
		LED::inicializar(ledPin);
	}
}