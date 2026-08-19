#pragma once

class SemaforoLeds {
private:
    uint8_t pinRed;
    uint8_t pinYellow;
    uint8_t pinGreen;

    /* **** Desliga todos os LEDs **** */
    void desligarTodos() const {
        digitalWrite(pinRed, LOW);
        digitalWrite(pinYellow, LOW);
        digitalWrite(pinGreen, LOW);
    }

public:
    /* **** Construtor: define pinos **** */
    SemaforoLeds(uint8_t red, uint8_t yellow, uint8_t green)
        : pinRed(red), pinYellow(yellow), pinGreen(green) {}

    /* **** Inicializa GPIOs e começa em verde **** */
    void inicializar() const {
        pinMode(pinRed, OUTPUT);
        pinMode(pinYellow, OUTPUT);
        pinMode(pinGreen, OUTPUT);
        faseVerde();
    }

    /* **** Estados do semáforo **** */
    void faseVerde() const {
        desligarTodos();
        digitalWrite(pinGreen, HIGH);
    }

    void faseAmarela() const {
        desligarTodos();
        digitalWrite(pinYellow, HIGH);
    }

    void faseVermelha() const {
        desligarTodos();
        digitalWrite(pinRed, HIGH);
    }
};

