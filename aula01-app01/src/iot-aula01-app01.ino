#include <DHT.h>

//Definições - GPIO
#define BOTAO 18
#define LED 26

// Estrutura DHT
#define DHTPIN 21
#define DHTMODE DHT22
DHT dht(DHTPIN, DHTMODE);

/* *** Funções início *** */
bool botaoPressionado() {
  return digitalRead(BOTAO) == LOW;
}

bool medirTempUmid(float* p_temp, float* p_umid, float* p_ic) {
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();

  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println("Falha ao ler do sensor DHT!");
    *p_temp = -99.0;
    *p_umid = -99.0;
    *p_ic = -99.0;
    return false;
  }

  float heatIndex = dht.computeHeatIndex(temperatura, umidade, false);

  Serial.print("Temperatura: "); Serial.print(temperatura);
  Serial.print(" °C, Umidade: "); Serial.print(umidade);
  Serial.print(" %, Índice de calor: "); Serial.print(heatIndex); Serial.println(" °C");

  *p_temp = temperatura;
  *p_umid = umidade;
  *p_ic = heatIndex;

  return true;
}
/* *** Funções fim *** */


void setup() {
  pinMode(BOTAO, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  dht.begin();
  Serial.begin(115200);
  Serial.println("Pressione o botão para medir.");
}

void loop() {
  float temp, umid, ic;
  bool acenderLed = false;

  while (!botaoPressionado()) {
    //Nada
  }
  
  if (medirTempUmid(&temp, &umid, &ic)) {
    (ic > 30) ? acenderLed = true : acenderLed = false;
    digitalWrite(LED, acenderLed ? HIGH : LOW);
    //ou
    //digitalWrite(LED, acenderLed);
    (ic > 50) ? Serial.println("Alerta: Índice de calor alto!") : Serial.println("Índice de calor normal.");
  } else {
    Serial.println("Erro ao medir temperatura e umidade.");
  }

  delay(2000); // Aguarda 2 segundos antes da próxima leitura
  Serial.println("Pressione o botão para medir novamente.");
}

