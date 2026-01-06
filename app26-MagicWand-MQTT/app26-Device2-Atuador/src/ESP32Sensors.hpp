// Inclui os cabeçalhos dos módulos para que as funções sejam visíveis
#include "ESP32SensorsAudio.hpp"
#include "ESP32SensorsLED.hpp"
#include "ESP32SensorsServo.hpp"

namespace ESP32Sensors {
	void beginAll(uint8_t buzPin, uint8_t ledPin, uint8_t servoPin) {
		Audio::inicializar(buzPin);
		LED::inicializar(ledPin);
		MiniServo::inicializar(servoPin);
	}
}