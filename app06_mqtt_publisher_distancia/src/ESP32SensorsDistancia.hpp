#pragma once
#include <Arduino.h>

namespace ESP32Sensors {
	namespace Distancia {
		// Variáveis privadas do módulo
		static uint8_t trigPin = 0;
		static uint8_t echoPin = 0;

		void inicializar(uint8_t tPin, uint8_t ePin) {
			trigPin = tPin;
			echoPin = ePin;
			pinMode(trigPin, OUTPUT);
			pinMode(echoPin, INPUT);
		}

		float medirDistancia() {
			// Etapa 1: Garante estabilidade prévia à leitura
			digitalWrite(trigPin, LOW);
			delayMicroseconds(2);

			// Etapa 2: Pulso de disparo de 10 microssegundos
			digitalWrite(trigPin, HIGH);
			delayMicroseconds(10);
			digitalWrite(trigPin, LOW);

			// Etapa 3: Quanto tempo se passou até o obstáculo.
			//          O timeout de 30 ms devolve 0 quando não há eco,
			//          em vez de travar o loop esperando.
			unsigned long duracao = pulseIn(echoPin, HIGH, 30000);

			// Etapa 4: Calcula em centímetros (som = 340 m/s = 0.034 cm/µs)
			return duracao * 0.034 / 2;   // /2 -> ida e volta
		}
	}
}
