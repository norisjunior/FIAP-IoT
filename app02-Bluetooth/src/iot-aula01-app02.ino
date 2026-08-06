// Passador de slides via Bluetooth: o ESP32 vira um "teclado sem fio"

#include <BleKeyboard.h>

// Pinos
#define BOTAO_AVANCAR     18
#define BOTAO_RETROCEDER  19
#define LED_PIN           23

// Nome que vai aparecer na lista de dispositivos Bluetooth do computador
#define DEVICE_NAME "PassadorSlides"

BleKeyboard teclado(DEVICE_NAME);

void setup() {
  Serial.begin(115200);
  pinMode(BOTAO_AVANCAR, INPUT_PULLUP);    // botão solto = HIGH, pressionado = LOW
  pinMode(BOTAO_RETROCEDER, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  teclado.begin();                         // liga o Bluetooth

  Serial.println("--------------------------------------");
  Serial.println("-      Passador de slides (BLE)      -");
  Serial.println("--------------------------------------");
  Serial.printf("--> Conecte '%s' no Bluetooth do computador <--\r\n", DEVICE_NAME);
}



void loop() {
  // LED aceso = conectado no computador
  if (!teclado.isConnected()) {
    digitalWrite(LED_PIN, LOW);
    return;
  }
  digitalWrite(LED_PIN, HIGH);

  if (digitalRead(BOTAO_AVANCAR) == LOW) {
    teclado.write(KEY_RIGHT_ARROW);        // mesma tecla "seta direita" do teclado
    Serial.println("AVANÇAR");
    delay(300);                            // evita mandar vários slides de uma vez
  }

  if (digitalRead(BOTAO_RETROCEDER) == LOW) {
    teclado.write(KEY_LEFT_ARROW);         // mesma tecla "seta esquerda" do teclado
    Serial.println("RETROCEDER");
    delay(300);
  }
}
