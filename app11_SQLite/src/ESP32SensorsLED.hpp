namespace ESP32Sensors {
	namespace LED {
		const uint8_t LED_PIN = 27;

		void inicializar() {
			pinMode(LED_PIN, OUTPUT);
			digitalWrite(LED_PIN, LOW);
		}

		void on() {
			digitalWrite(LED_PIN, HIGH);
		}

		void off() {
			digitalWrite(LED_PIN, LOW);
		}
	}
}
