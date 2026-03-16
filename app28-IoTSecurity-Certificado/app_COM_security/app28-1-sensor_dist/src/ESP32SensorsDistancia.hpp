#pragma once

class DistanceSensor {
private:
    uint8_t trigPin;
    uint8_t echoPin;

public:
    explicit DistanceSensor(uint8_t trig, uint8_t echo)
        : trigPin(trig), echoPin(echo) {}

	// Ou
	// DistanceSensor(uint8_t trig, uint8_t echo) {
	// 	trigPin = trig;
	// 	echoPin = echo;
	// }

    void inicializar() const {
        pinMode(trigPin, OUTPUT);
        pinMode(echoPin, INPUT);
    }

    float medirDist() const {
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

		return distancia;
    }
};
