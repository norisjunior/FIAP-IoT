#include <Arduino.h>

#define MOTOR_INA 25
#define MOTOR_INB 26
#define BTN_A     32  // gira para frente
#define BTN_B     33  // gira para trás
#define LED_PIN   27  // acende enquanto qualquer botão estiver pressionado

void setup() {
  pinMode(MOTOR_INA, OUTPUT);
  pinMode(MOTOR_INB, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);

  digitalWrite(MOTOR_INA, LOW);
  digitalWrite(MOTOR_INB, LOW);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  bool btnA = digitalRead(BTN_A) == LOW;
  bool btnB = digitalRead(BTN_B) == LOW;

  if (btnA) {
    digitalWrite(MOTOR_INA, HIGH);
    digitalWrite(MOTOR_INB, LOW);
  } else if (btnB) {
    digitalWrite(MOTOR_INA, LOW);
    digitalWrite(MOTOR_INB, HIGH);
  } else {
    digitalWrite(MOTOR_INA, LOW);
    digitalWrite(MOTOR_INB, LOW);
  }

  bool acendeLED = (btnA || btnB ? HIGH : LOW);
  digitalWrite(LED_PIN, acendeLED);
}

