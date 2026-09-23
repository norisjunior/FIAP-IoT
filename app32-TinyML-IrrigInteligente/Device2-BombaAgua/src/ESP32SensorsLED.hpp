namespace ESP32Sensors {
	namespace LED {
		// Variável privada do módulo
		static uint8_t ledPin = 0;

		// Função de inicialização que recebe o pino como parâmetro
		void inicializar(uint8_t pin) {
			ledPin = pin;
			pinMode(ledPin, OUTPUT);
			digitalWrite(ledPin, LOW);
		}

		void on() {
			if (ledPin > 0) {
				digitalWrite(ledPin, HIGH);
			}
		}

		void off() {
			if (ledPin > 0) {
				digitalWrite(ledPin, LOW);
			}
		}
	}
}