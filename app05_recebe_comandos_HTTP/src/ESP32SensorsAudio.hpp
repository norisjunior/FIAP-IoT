namespace ESP32Sensors {
	namespace Audio {
		static uint8_t buzzerPin = 0;

		//PWM
		static uint8_t CANAL = 0;
		static uint16_t FREQ = 1000;
		static uint8_t RESOLUCAO = 8;		

		void inicializar(uint8_t pin) {
			buzzerPin = pin;
			ledcSetup(CANAL, FREQ, RESOLUCAO);
			ledcAttachPin(buzzerPin, CANAL); 
		}

		void ativarAlarme() {
			//ledcWriteTone(buzzerPin, 1000);  // toca indefinidamente em 1kHz
			tone(buzzerPin, FREQ);
		}

		void desativarAlarme() {
			//ledcWriteTone(buzzerPin, 0);     // silencia
			noTone(buzzerPin);
		}

		void tocar(uint16_t freq = FREQ, uint16_t dur = 500) {
			tone(buzzerPin, freq, dur);
		}
	}
}
