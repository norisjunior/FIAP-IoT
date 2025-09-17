// Inclui os cabeçalhos dos módulos para que as funções sejam visíveis
#include "ESP32SensorsAmbiente.hpp"
#include "ESP32SensorsDistancia.hpp"
#include "ESP32SensorsLED.hpp"

namespace ESP32Sensors {
	// Função de inicialização que recebe as configurações de hardware
	void beginAll(uint8_t dhtPin, uint8_t dhtModel, uint8_t distTPin, uint8_t distEPin, float distLimiar, uint8_t ledPin) {
		Ambiente::inicializar(dhtPin, dhtModel);
		Distancia::inicializar(distTPin, distEPin, distLimiar);
		LED::inicializar(ledPin);
	}
}