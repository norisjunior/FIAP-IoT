#include <DHT.h>

namespace ESP32Sensors {
    namespace Ambiente {

        struct AMBIENTE {
            float temp;
            float umid;
            float ic;
            bool valido;
        };

        static uint8_t dhtPin = 0;
        static uint8_t dhtModel = 0;
        static DHT dht(0, 0);
        static const int TEMPO_DECORRIDO_MIN = 2000;
        static int ultimoTempoColeta = 0;

        void inicializar(uint8_t pin, uint8_t modelo) {
            dhtPin = pin;
            dhtModel = modelo;
            dht = DHT(pin, modelo);
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
            dados.ic = NAN;
            dados.valido = false;

            if (!podeColetar()) {
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
            dados.ic = dht.computeHeatIndex(t, u, false);
            dados.valido = true;

            return dados;
        }
    }
}
