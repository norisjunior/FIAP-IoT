namespace ESP32Sensors {
	namespace Audio {
		//Buzzer
		static uint8_t buzzerPin = 0;

		//PWM - Usar canal 15 para evitar conflito com servo (canais 0-7)
		const uint8_t CANAL = 15;
		const uint16_t FREQ = 500;
		const uint8_t RESOLUCAO = 8;

		void inicializar(uint8_t pin) {
            buzzerPin = pin;
			pinMode(buzzerPin, OUTPUT);
			ledcSetup(CANAL, FREQ, RESOLUCAO);
			ledcAttachPin(buzzerPin, CANAL);  
		}

		void ativarAlarme() {
			ledcWriteTone(CANAL, 250); // Ativa som de 1kHz
		}

		void desativarAlarme() {
			ledcWriteTone(CANAL, 0);
		}

		void tocar(uint16_t freq = FREQ, uint16_t dur = 500) {
			tone(buzzerPin, freq, dur);
		}
	}
}