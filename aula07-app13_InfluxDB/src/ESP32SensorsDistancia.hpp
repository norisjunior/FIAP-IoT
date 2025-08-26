#include <Arduino.h>

namespace ESP32Sensors {
	namespace Distancia {

		struct DISTANCIA {
			float cm;
			float limiar;
			bool alarmar;
		};

		float ESP32Sensors_LIMIAR = 100.0;    // 100 cm (1 metro)
		const uint8_t TRIG_PIN = 17;
		const uint8_t ECHO_PIN = 16;

		void inicializar() {
			pinMode(TRIG_PIN, OUTPUT);
			pinMode(ECHO_PIN, INPUT);
		}

		DISTANCIA medirDistancia() {
			pinMode(TRIG_PIN, OUTPUT);
			pinMode(ECHO_PIN, INPUT);

			DISTANCIA dist;
			dist.cm = 0;
			dist.limiar = ESP32Sensors_LIMIAR;
			dist.alarmar = false;


			// Etapa 1: Garante estabilidade prévia à leitura
			digitalWrite(TRIG_PIN, LOW);
			delayMicroseconds(2);

			// Etapa 2: Pulso de disparo de 10 microssegundos 
			digitalWrite(TRIG_PIN, HIGH);
			delayMicroseconds(10);
			digitalWrite(TRIG_PIN, LOW);
			
			// Etapa 3: Quanto tempo se passou até o obstáculo
			int duracao = pulseIn(ECHO_PIN, HIGH);
			
			// Etapa 4: Calcula em centímetros (som = 340 m/s = 0.034 cm/µs)
			float distancia = duracao * 0.034 / 2; // /2 -> ida e volta

			dist.cm = distancia;

			if (dist.cm >= dist.limiar) {
				dist.alarmar = true;
			}

			return dist;
		}
	}
}