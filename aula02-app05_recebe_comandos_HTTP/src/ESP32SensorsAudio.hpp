namespace ESP32Sensors {
	namespace Audio {
		//Buzzer
		const uint8_t BUZZER = 27;

		//PWM
		const uint8_t CANAL = 0;
		const uint16_t FREQ = 1000;
		const uint8_t RESOLUCAO = 8;

		void inicializar() {
			pinMode(BUZZER, OUTPUT);
			ledcSetup(CANAL, FREQ, RESOLUCAO);
			ledcAttachPin(BUZZER, CANAL);  
		}

		void ativarAlarme() {
			ledcWriteTone(CANAL, 1000); // Ativa som de 1kHz
		}

		void desativarAlarme() {
			ledcWriteTone(CANAL, 0);
		}

		void tocar(uint16_t freq = FREQ, uint16_t dur = 500) {
			tone(BUZZER, freq, dur);
		}
	}
}
