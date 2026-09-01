#include <Arduino.h>

#define MOTOR_INA 18
#define MOTOR_INB 19
#define BTN_A     4  // gira para frente
#define BTN_B     21  // gira para trás
#define LED_PIN   2  // acende enquanto qualquer botão estiver pressionado

const int PWM_MOTOR = 100; // velocidade reduzida (0-255) em vez de full-on

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
    analogWrite(MOTOR_INA, PWM_MOTOR);
    digitalWrite(MOTOR_INB, LOW);
  } else if (btnB) {
    digitalWrite(MOTOR_INA, LOW);
    analogWrite(MOTOR_INB, PWM_MOTOR);
  } else {
    digitalWrite(MOTOR_INA, LOW);
    digitalWrite(MOTOR_INB, LOW);
  }

  bool acendeLED = (btnA || btnB ? HIGH : LOW);
  digitalWrite(LED_PIN, acendeLED);
}

