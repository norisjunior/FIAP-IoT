namespace ESP32Sensors {
    namespace Dryness {
        static uint8_t drynessPin = 0;

        // Faixa de leitura do potenciômetro mapeada para 0-1023 (compatível com dataset)
        constexpr float DRY_MIN = 0;
        constexpr float DRY_MAX = 1023;

        struct DRY {
            float valor;
            bool valido;
        };

        void inicializar(uint8_t pin) {
            drynessPin = pin;
            analogReadResolution(12);
            pinMode(drynessPin, INPUT);
        }

        static inline float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
            return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
        }

        inline DRY ler() {
            DRY r;
            r.valor = 0;
            r.valido = false;

            int valorLido = analogRead(drynessPin); // 0..4095
            if (valorLido < 0) return r;

            // Mapeia para 0-1023 (range do dataset de treinamento)
            r.valor = mapf(static_cast<float>(valorLido), 0.0f, 4095.0f, DRY_MIN, DRY_MAX);
            r.valido = true;
            return r;
        }
    }
}
