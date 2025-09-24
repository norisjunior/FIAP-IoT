namespace ESP32Sensors {
	namespace Movimento {
		static uint8_t pirPin = 0;

		void inicializar(uint8_t pin) {
			pirPin = pin;
			pinMode(pirPin, INPUT);
		}

		bool detectarMovimento() {
			bool movimento = digitalRead(pirPin);
			delay(100);
			return movimento;
		}
	}
}
