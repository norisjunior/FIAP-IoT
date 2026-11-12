// app-ESP32-C6-MUSELAB
// Teste da placa: arco-iris no LED RGB interno.
//
// O LED RGB da nanoESP32-C6 (Muse Lab) e' um WS2812B ligado no GPIO 8
// (conferido no esquematico da placa). Nao precisa de biblioteca: a funcao
// rgbLedWrite() ja vem no core Arduino do ESP32 (versao 3.x).
// No app-ESP32-C3-SuperMini (core 2.x) a mesma funcao se chama neopixelWrite().

#define PINO_LED  8    // LED RGB interno
#define BRILHO    40   // 0 a 255 (o LED e' muito forte; 40 basta)
#define PASSO_MS  10   // tempo entre uma cor e a proxima

// Roda de cores: pos vai de 0 a 255 e passa por
// vermelho -> verde -> azul -> vermelho.
void roda(int pos, int &r, int &g, int &b) {
  if (pos < 85) {           // vermelho -> verde
    r = 255 - pos * 3;  g = pos * 3;        b = 0;
  } else if (pos < 170) {   // verde -> azul
    pos -= 85;
    r = 0;              g = 255 - pos * 3;  b = pos * 3;
  } else {                  // azul -> vermelho
    pos -= 170;
    r = pos * 3;        g = 0;              b = 255 - pos * 3;
  }
}

int volta = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32-C6 Muse Lab: arco-iris no GPIO 8");
}

void loop() {
  for (int pos = 0; pos < 256; pos++) {
    int r, g, b;
    roda(pos, r, g, b);
    rgbLedWrite(PINO_LED, r * BRILHO / 255, g * BRILHO / 255, b * BRILHO / 255);
    delay(PASSO_MS);
  }

  volta++;
  Serial.print("Volta ");
  Serial.println(volta);
}
