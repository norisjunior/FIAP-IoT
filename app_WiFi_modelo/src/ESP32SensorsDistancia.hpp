#pragma once

// HC-SR04
#include <EasyUltrasonic.h>
EasyUltrasonic sensorDist;

namespace ESP32Sensors {
    namespace Distancia {

        //static trigPin = 0;
        //static echoPin = 0;

        void inicializar(int tPin, int ePin) {
            sensorDist.attach(tPin, ePin);
        }

        float medirDist() {
            float distancia = sensorDist.getDistanceCM();
            return distancia;
        }
    }
}


