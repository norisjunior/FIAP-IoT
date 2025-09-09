#include <Arduino.h>
#include <LittleFS.h>
#include "ESP32Sensors.hpp"   // LED, Ambiente (DHT22)

// -------------------- Configurações de Hardware --------------------
const uint8_t DHT_PIN   = 26;
const uint8_t DHT_MODEL = DHT22;
const uint8_t LED_PIN   = 27;
const uint8_t BTN_PIN   = 25;

// -------------------- Constantes --------------------
const char *ARQ_DADOS                = "/dados.csv";  // único arquivo de log
const uint32_t INTERVALO_COLETA_MS   = 5000;          // 5 s entre leituras

// -------------------- Variáveis globais --------------------
unsigned long ultimoRegistro = 0;

// -------------------- Protótipos das Funções --------------------
void criaArquivoSeNecessario();
void gravaMedicoes();
void imprimeArquivo();
void zeraMedicoes();

// -------------------- Setup --------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  if (!LittleFS.begin(true)) {   // true = formata se não existir
    Serial.println("[FATAL] Falha ao montar SPIFFS");
    while (true) {}
  }

  // Inicializa os sensores com as configurações de hardware definidas acima
  ESP32Sensors::beginAll(DHT_PIN, DHT_MODEL, LED_PIN);
  pinMode(BTN_PIN, INPUT_PULLUP);
  
  criaArquivoSeNecessario();

  Serial.println("[OK] Sistema pronto - gravando sem rotação.");
}

// -------------------- Loop --------------------
void loop() {
  if (millis() - ultimoRegistro >= INTERVALO_COLETA_MS) {
    ultimoRegistro = millis();
    gravaMedicoes();
  }

  if (digitalRead(BTN_PIN) == LOW) { //Pressionou o botão
    zeraMedicoes();
    delay(500);
  }
}

// -------------------- Funções auxiliares --------------------

void criaArquivoSeNecessario() {
  if (LittleFS.exists(ARQ_DADOS)) return;
  File f = LittleFS.open(ARQ_DADOS, FILE_WRITE);
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
  File f = LittleFS.open(ARQ_DADOS, FILE_APPEND);
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
  File f = LittleFS.open(ARQ_DADOS, FILE_READ);
  if (!f) {
    Serial.println("[ERRO] Falha ao abrir dados.csv para leitura");
    return;
  }
  while (f.available()) Serial.write(f.read());
  Serial.println("---------------- fim do arquivo ----------------\n");
  f.close();
}

//Zera medições com o pressionamento do botão
void zeraMedicoes() {
  Serial.println("[BOTÃO] Zerando medições...");

  // Revmove arquivo existente
  if (LittleFS.exists(ARQ_DADOS)) {
    LittleFS.remove(ARQ_DADOS);
  }

  //Cria arquivo novamente (apenas com o cabeçalho)
  // Cria novo arquivo diretamente com o cabeçalho
  File f = LittleFS.open(ARQ_DADOS, FILE_WRITE);
  if (!f) {
    Serial.println("[ERRO] Não foi possível criar arquivo!");
    return;
  }
  f.println("temp_C,umid_%,indice_calor_C");
  f.close();

  Serial.println("[OK] Medições zeradas!");
  imprimeArquivo();
}