namespace ESP32Sensors {
    namespace LED {
        static uint8_t ledPin = 0;

        void inicializar(uint8_t pin) {
            ledPin = pin;
            pinMode(ledPin, OUTPUT);
            digitalWrite(ledPin, LOW);
        }

        void on() {
            digitalWrite(ledPin, HIGH);
        }

        void off() {
            digitalWrite(ledPin, LOW);
        }
    }
}
