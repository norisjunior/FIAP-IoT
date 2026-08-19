#include <Arduino.h>

#define BTN_MOTOR 26
#define LED_PIN   27

#define MOTOR_IN1 22
#define MOTOR_IN2 23

bool motorLigado = false;
int ultimoBotao = HIGH;

unsigned long ultimoDebounce = 0;
const unsigned long debounceMs = 300;

const int PWM_MOTOR = 100;

void ligarMotor() {
  analogWrite(MOTOR_IN1, 180);
  digitalWrite(MOTOR_IN2, LOW);
  delay(150);
  analogWrite(MOTOR_IN1, PWM_MOTOR);
  digitalWrite(MOTOR_IN2, LOW);
}

void desligarMotor() {
  analogWrite(MOTOR_IN1, 0);
  digitalWrite(MOTOR_IN2, LOW);
}

void setup() {
  Serial.begin(115200);

  pinMode(BTN_MOTOR, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);

  desligarMotor();
  digitalWrite(LED_PIN, LOW);

  Serial.println("Teste motor + botao iniciado");
}

void loop() {
  int leitura = digitalRead(BTN_MOTOR);

  if (ultimoBotao == HIGH && leitura == LOW) {
    if (millis() - ultimoDebounce > debounceMs) {
      motorLigado = !motorLigado;

      if (motorLigado) {
        ligarMotor();
        digitalWrite(LED_PIN, HIGH);
        Serial.println("Motor ligado");
      } else {
        desligarMotor();
        digitalWrite(LED_PIN, LOW);
        Serial.println("Motor desligado");
      }

      ultimoDebounce = millis();
    }
  }

  ultimoBotao = leitura;
}