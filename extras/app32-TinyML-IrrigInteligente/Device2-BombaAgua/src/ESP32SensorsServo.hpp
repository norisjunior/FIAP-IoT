#include <ESP32Servo.h>

namespace ESP32Sensors {
    namespace MiniServo {
        static uint8_t servoPin = 0;
        Servo servo;

        // Posições do servo (simula válvula/bomba)
        const int POS_DESLIGADO = 0;    // Bomba desligada
        const int POS_LIGADO    = 90;   // Bomba ligada

        void inicializar(uint8_t pin) {
            servoPin = pin;
            servo.attach(servoPin);
            servo.write(POS_DESLIGADO);
        }

        void ligar() {
            servo.write(POS_LIGADO);
        }

        void desligar() {
            servo.write(POS_DESLIGADO);
        }

        void setPosicao(int angulo) {
            servo.write(angulo);
        }
    }
}