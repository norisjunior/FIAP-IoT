#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "ESP32Sensors.hpp"   // LED, Ambiente (DHT22)

// -------------------- Constantes --------------------
const gpio_num_t CS_PIN = GPIO_NUM_5;      // chip-select do cartão
const char *ARQ_DADOS                = "/dados.csv";  // único arquivo de log
const uint32_t INTERVALO_COLETA_MS   = 5000;          // 5 s entre leituras

// -------------------- Variáveis globais --------------------
unsigned long ultimoRegistro = 0;

// -------------------- Protótipos das Funções --------------------
void criaArquivoSeNecessario();
void gravaMedicoes();
void imprimeArquivo();

// -------------------- Setup --------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  // Inicia SD (usa barramento VSPI padrão: SCK 18, MISO (DO) 19, MOSI (DI) 23)
  if (!SD.begin(CS_PIN)) {
    Serial.println("[FATAL] Falha ao iniciar SD!");
    while (true) {}
  }
  uint64_t mb = SD.cardSize() / (1024 * 1024);
  Serial.printf("[SD] Cartão OK - %llu MB\n", mb); Serial.println("");

  ESP32Sensors::beginAll();
  criaArquivoSeNecessario();

  Serial.println("[OK] Sistema pronto - gravando no cartão SD.");
}

// -------------------- Loop --------------------
void loop() {
  if (millis() - ultimoRegistro >= INTERVALO_COLETA_MS) {
    ultimoRegistro = millis();
    gravaMedicoes();
  }
}







// -------------------- Funções auxiliares --------------------

void criaArquivoSeNecessario() {
  if (SD.exists(ARQ_DADOS)) return;
  File f = SD.open(ARQ_DADOS, FILE_WRITE);
  if (!f) {
    Serial.println("[ERRO] Não criou dados.csv");
    return;
  }
  f.println("temp_C,umid_%,indice_calor_C");
  f.close();
}


void gravaMedicoes() {
  //Coleta das medições
  ESP32Sensors::Ambiente::AMBIENTE ambiente = ESP32Sensors::Ambiente::medirAmbiente();
  if (!ambiente.valido) {
    Serial.println("[ERRO] Leituras DHT inválidas - pulando ciclo");
    return;
  }

  ESP32Sensors::LED::on();

  //Abre arquivo para gravar os dados coletados
  File f = SD.open(ARQ_DADOS, FILE_APPEND);
  if (!f) {
    Serial.println("[ERRO] Não abriu dados.csv para append");
    ESP32Sensors::LED::off();
    return;
  }

  //Escreve no arquivo
  f.printf("%.2f,%.2f,%.2f\n",
           ambiente.temp,
           ambiente.umid,
           ambiente.ic);
  f.close();

  ESP32Sensors::LED::off();

  Serial.println("[SALVO] Nova linha gravada. Conteúdo completo:");
  imprimeArquivo();
}

// Imprime o arquivo inteiro no Serial Monitor
void imprimeArquivo() {
  File f = SD.open(ARQ_DADOS, FILE_READ);
  if (!f) {
    Serial.println("[ERRO] Falha ao abrir dados.csv para leitura");
    return;
  }
  while (f.available()) {
    String linha = f.readStringUntil('\n');  // lê até LF
    Serial.println(linha);                  // envia CR+LF de uma vez
  }
  Serial.println("---------------- fim do arquivo ----------------\n");
  f.close();
}
