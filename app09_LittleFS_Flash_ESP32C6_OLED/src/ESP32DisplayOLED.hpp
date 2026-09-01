#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "ESP32SensorsAmbiente.hpp"

namespace ESP32Display {
	namespace OLED {
		static Adafruit_SSD1306 display(128, 64, &Wire, -1);
		static bool disponivel = false;

		inline bool inicializar(uint8_t sdaPin, uint8_t sclPin, uint8_t endereco) {
			Wire.begin(sdaPin, sclPin);
			disponivel = display.begin(SSD1306_SWITCHCAPVCC, endereco);

			if (!disponivel) {
				Serial.println("[AVISO] Display OLED não encontrado");
			}

			return disponivel;
		}

		inline void exibirMedicoes(const ESP32Sensors::Ambiente::AMBIENTE &ambiente) {
			if (!disponivel) return;

			display.clearDisplay();
			display.setTextColor(SSD1306_WHITE);

			// Os 16 pixels superiores são amarelos neste modelo de OLED.
			display.setTextSize(1);
			display.setCursor(25, 4);
			display.print("MEDICAO ATUAL");
			display.drawFastHLine(0, 15, display.width(), SSD1306_WHITE);

			// T e U ficam inteiros na área azul do visor.
			display.setTextSize(2);
			display.setCursor(0, 18);
			display.printf("T: %.1f C", ambiente.temp);
			display.setCursor(0, 36);
			display.printf("U: %.1f %%", ambiente.umid);

			// IC = índice de calor.
			display.setTextSize(1);
			display.setCursor(0, 55);
			display.printf("IC: %.1f C", ambiente.ic);

			display.display();
		}
	}
}
