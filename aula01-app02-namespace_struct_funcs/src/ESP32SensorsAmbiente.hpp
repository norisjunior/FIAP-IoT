#include <DHT.h>

namespace ESP32Sensors {
	namespace Ambiente {

		struct AMBIENTE {
			float temp;
			float umid;
			bool valido;
		};

		const uint8_t DHT_PIN   = 21;
		#define DHT_MODEL DHT22
		DHT dht(DHT_PIN, DHT_MODEL);

		const int TEMPO_DECORRIDO_MIN = 2000;  // 2 segundos
		static int ultimoTempoColeta = 0;

		void inicializar() {
			dht.begin();
		}

		bool podeColetar() {
			int agora = millis();
			if (agora - ultimoTempoColeta >= TEMPO_DECORRIDO_MIN) {
				ultimoTempoColeta = agora;
				return true;
			}
			return false;
		}

		AMBIENTE medirAmbiente() {
			AMBIENTE dados;
			dados.temp = NAN;
			dados.umid = NAN;
			dados.valido = false;

			if (!podeColetar()) {
				Serial.println("ERRO: Tempo mínimo emtre leituras não atingido.");
				return dados;
			}

			float t = dht.readTemperature();
			float u = dht.readHumidity();

			if (isnan(t) || isnan(u)) {
				Serial.println("ERRO: leituras incorretas no sensor DHT.");
				return dados;
			}

			dados.temp = t;
			dados.umid = u;
			dados.valido = true;

			return dados;
		}
	}
}
