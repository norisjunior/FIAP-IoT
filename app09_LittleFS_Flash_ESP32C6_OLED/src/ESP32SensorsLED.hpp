#pragma once

#include <Arduino.h>
#include "esp32-hal-rgb-led.h"

namespace ESP32Sensors {
	namespace LED {
		static uint8_t ledPin = 0;

		inline void inicializar(uint8_t pin) {
			ledPin = pin;
			pinMode(ledPin, OUTPUT);
			digitalWrite(ledPin, LOW);
		}

		inline void on() {
			if (ledPin > 0) {
				digitalWrite(ledPin, HIGH);
			}
		}

		inline void off() {
			if (ledPin > 0) {
				digitalWrite(ledPin, LOW);
			}
		}

		inline void piscar(uint8_t vezes = 2, uint16_t intervaloMs = 120) {
			for (uint8_t i = 0; i < vezes; i++) {
				on();
				delay(intervaloMs);
				off();
				delay(intervaloMs);
			}
		}
	}

	namespace LED_RGB {
		// Cor institucional FIAP: #EF1957.
		static const uint8_t FIAP_R = 0xEF;
		static const uint8_t FIAP_G = 0x19;
		static const uint8_t FIAP_B = 0x57;

		inline void inicializar(uint8_t pin) {
			rgbLedWrite(pin, FIAP_R, FIAP_G, FIAP_B);
		}
	}
}
