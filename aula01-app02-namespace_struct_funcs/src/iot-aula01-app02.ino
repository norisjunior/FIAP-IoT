// File: src/iot-aula01-app02.ino

#include "ESP32Sensors.hpp"

//Botão
#define BOTAO 18

/* *** Struct SensorsAmbiente *** */
uint8_t MAX_LEITURAS = 5;
uint8_t idLeitura = 0;

// Uso didático, para produção não se recomenda usar "using", o ideal é usar o fully qualified name
using ESP32Sensors::Ambiente::AMBIENTE;
/* *** Struct fim *** */

/* *** Funções início *** */
bool botaoPressionado() {
  return digitalRead(BOTAO) == LOW;
}

//formato de produção (FQN):
//void avaliarAmbiente(ESP32Sensors::Ambiente::AMBIENTE aval[], uint8_t tamanho) {
void avaliarAmbiente(AMBIENTE aval[], uint8_t tamanho) {
  float tempMedia = 0;
  float umidMedia = 0;
  
  for (int i = 0; i < tamanho; i++) {
    tempMedia += aval[i].temp;
    umidMedia += aval[i].umid;
  }
  tempMedia /= tamanho;
  umidMedia /= tamanho;

  Serial.println("\n***** Média das " + String(tamanho) + " medições:");
  Serial.printf("Temp. média: %.2f | Umid. média: %.2f", tempMedia, umidMedia); Serial.println("");

  (tempMedia > 35.0 || umidMedia > 80.0) ? ESP32Sensors::LED::on() : ESP32Sensors::LED::off();  
}

/* *** Funções fim *** */



void setup() {
  Serial.begin(115200);
  pinMode(BOTAO, INPUT_PULLUP); // Configura o pino do botão como entrada com pull-up interno
  ESP32Sensors::beginAll();
  Serial.println("--------------------------------------");
  Serial.println("- Uso de matrizes, vetores e structs -");
  Serial.println("--------------------------------------");
  Serial.println("--> Pressione o botão para iniciar <--");
}



void loop() {
  //Declaração de "fabrica", já está no formato para produção
  ESP32Sensors::Ambiente::AMBIENTE fabrica[MAX_LEITURAS];
  bool acenderLed = false;
  float tempMedia = 0;
  float umidMedia = 0;

  while (!botaoPressionado()) {
    //Nada
  }
  
  Serial.println("\nBotão pressionado! Coletando 5 medições do ambiente...");

  ESP32Sensors::Ambiente::AMBIENTE leitura = ESP32Sensors::Ambiente::medirAmbiente();

  if (!leitura.valido) {
    Serial.println("Leitura inválida. Aguarde o tempo mínimo exigido pelo sensor DHT (2 segundos).");
    delay(100);
    return;
  }
  
  Serial.printf("Leitura número %i", idLeitura); Serial.print(" --> ");
  fabrica[idLeitura] = leitura;
  Serial.printf("Temp: %.2f ºC | Umid: %.2f %%", fabrica[idLeitura].temp, fabrica[idLeitura].umid); Serial.println("");
  
  idLeitura++;

  if (idLeitura == MAX_LEITURAS) {
    // Avalia as condições observando a média das medições
    avaliarAmbiente(fabrica, MAX_LEITURAS);
    idLeitura = 0;
  }

  delay(100);
  Serial.println("-> Pressione o botão para medir novamente... <-");
}
