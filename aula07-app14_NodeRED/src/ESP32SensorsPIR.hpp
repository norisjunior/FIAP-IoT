#define PIR_PIN 13

namespace ESP32Sensors {
	namespace PIR {
		const uint8_t LED_PIN = 27;

		void inicializar() {
			pinMode(PIR_PIN, INPUT);
		}

		int detectarMovimento() {
			return digitalRead(PIR_PIN);
		}
	}
}
