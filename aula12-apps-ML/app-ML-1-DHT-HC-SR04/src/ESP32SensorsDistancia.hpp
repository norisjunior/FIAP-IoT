#include <Arduino.h>
namespace ESP32Sensors {
	namespace Distancia {
		// Variável privada do módulo
		static uint8_t trigPin = 0;
		static uint8_t echoPin = 0;
		static float distLimiar = 100.0;    // 100 cm (1 metro)
		struct DISTANCIA {
			float cm;
			float limiar;
			bool alarmar;
		};

		
		//const uint8_t TRIG_PIN = 17;
		//const uint8_t ECHO_PIN = 16;

		void inicializar(uint8_t tPin, uint8_t ePin, float limiar) {
			trigPin = tPin;
			echoPin = ePin;
			distLimiar = limiar;
			pinMode(trigPin, OUTPUT);
			pinMode(echoPin, INPUT);
		}

		DISTANCIA medirDistancia() {
			pinMode(trigPin, OUTPUT);
			pinMode(echoPin, INPUT);

			DISTANCIA dist;
			dist.cm = 0;
			dist.limiar = distLimiar;
			dist.alarmar = false;


			// Etapa 1: Garante estabilidade prévia à leitura
			digitalWrite(trigPin, LOW);
			delayMicroseconds(2);

			// Etapa 2: Pulso de disparo de 10 microssegundos 
			digitalWrite(trigPin, HIGH);
			delayMicroseconds(10);
			digitalWrite(trigPin, LOW);
			
			// Etapa 3: Quanto tempo se passou até o obstáculo
			int duracao = pulseIn(echoPin, HIGH);
			
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