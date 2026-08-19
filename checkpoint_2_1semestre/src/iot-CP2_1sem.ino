#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "ESP32Sensors.hpp"   // LED, Ambiente (DHT22)

// -------------------- Configurações de Hardware --------------------
const uint8_t DHT_PIN   = 26;
const uint8_t DHT_MODEL = DHT22;
const uint8_t LED_PIN   = 27;
const uint8_t BTN_ZERAR_PIN = 12;    // Botão para zerar medições
const uint8_t BTN_MOSTRAR_PIN = 16;  // Botão para mostrar dados
const uint8_t BTN_ENVIAR_PIN = 17;   // Botão para enviar via MQTT

// -------------------- Configurações WiFi --------------------
const char* ssid     = "Wokwi-GUEST";
const char* password = "";
WiFiClient wifiClient;

// -------------------- Configurações MQTT --------------------
#define MQTT_HOST       "broker.emqx.io"
#define MQTT_PORT       1883
#define MQTT_PUB_TOPIC  "fazenda/sensores/ambiente"
#define MQTT_DEVICEID   "ESP32_Ambiente_Monitor_001"
PubSubClient mqttClient(wifiClient);

// -------------------- Constantes --------------------
const gpio_num_t CS_PIN = GPIO_NUM_5;      // chip-select do cartão
const char *ARQ_DADOS = "/dados.csv";      // arquivo de log
const uint32_t INTERVALO_COLETA_MS = 10000; // 10s entre leituras
const uint32_t INTERVALO_MQTT_MS = 120000;  // 2 minutos para envio automático

// -------------------- Variáveis globais --------------------
unsigned long ultimoRegistro = 0;
unsigned long ultimoEnvioMQTT = 0;
bool wifiConectado = false;
bool mqttConectado = false;

// -------------------- Protótipos das Funções --------------------
void conectarWiFi();
void conectarMQTT();
void criaArquivoSeNecessario();
void gravaMedicoes();
void imprimeArquivo();
void zerarMedicoes();
void enviarDadosViaMQTT();
void verificarBotoes();
int contarLinhasArquivo();

// -------------------- Setup --------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  // Inicializa botões
  pinMode(BTN_ZERAR_PIN, INPUT_PULLUP);
  pinMode(BTN_MOSTRAR_PIN, INPUT_PULLUP);
  pinMode(BTN_ENVIAR_PIN, INPUT_PULLUP);

  // Inicia SD
  if (!SD.begin(CS_PIN)) {
    Serial.println("[FATAL] Falha ao iniciar SD!");
    while (true) {}
  }
  uint64_t mb = SD.cardSize() / (1024 * 1024);
  Serial.printf("[SD] Cartão OK - %llu MB\n", mb);
  Serial.println("");

  // Inicializa sensores
  ESP32Sensors::beginAll(DHT_PIN, DHT_MODEL, LED_PIN);
  criaArquivoSeNecessario();

  // Inicializa WiFi
  conectarWiFi();

  //Configura MQTT e conecta no Broker
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  conectarMQTT();

  Serial.println("[OK] Sistema pronto - SD Card + MQTT integrados.");
  Serial.println("Botões: Zerar, Mostrar, Enviar");
}

// -------------------- Loop --------------------
void loop() {
  // Verifica conexões
  if (wifiConectado && !mqttClient.connected()) {
    conectarMQTT();
  }
  if (mqttConectado) {
    mqttClient.loop();
  }

  // Coleta dados periodicamente
  if (millis() - ultimoRegistro >= INTERVALO_COLETA_MS) {
    ultimoRegistro = millis();
    gravaMedicoes();
  }

  // Envio automático via MQTT a cada 2 minutos
  if (mqttConectado && (millis() - ultimoEnvioMQTT >= INTERVALO_MQTT_MS)) {
    ultimoEnvioMQTT = millis();
    enviarDadosViaMQTT();
  }

  // Verifica botões
  verificarBotoes();

  delay(100); // Pequeno delay para estabilidade
}

// -------------------- Funções WiFi e MQTT --------------------
void conectarWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");
  
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[WiFi] Conectado!");
    Serial.printf("[WiFi] IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.println("");
    wifiConectado = true;
  } else {
    Serial.println("\n[WiFi] Falha na conexão - continuando sem MQTT");
    wifiConectado = false;
  }
}

void conectarMQTT() {
  if (!wifiConectado) return;
  
  Serial.printf("[MQTT] Conectando ao broker %s", MQTT_HOST);
  
  int tentativas = 0;
  while (!mqttClient.connected() && tentativas < 3) {
    if (mqttClient.connect(MQTT_DEVICEID)) {
      Serial.println(" --> Conectado ao MQTT Broker!");
      mqttConectado = true;
    } else {
      Serial.printf(" Falha, rc=%d", mqttClient.state());
      tentativas++;
      if (tentativas < 3) {
        Serial.println(" Tentando novamente em 3s...");
        delay(3000);
      }
    }
  }
  
  if (!mqttConectado) {
    Serial.println(" --> Falha final na conexão MQTT");
  }
}

// -------------------- Funções do SD Card --------------------
void criaArquivoSeNecessario() {
  if (SD.exists(ARQ_DADOS)) return;
  File f = SD.open(ARQ_DADOS, FILE_WRITE);
  if (!f) {
    Serial.println("[ERRO] Não criou dados.csv");
    return;
  }
  f.println("timestamp,temp_C,umid_%,indice_calor_C");
  f.close();
  Serial.println("[SD] Arquivo CSV criado com cabeçalho");
}

void gravaMedicoes() {
  // Coleta das medições
  ESP32Sensors::Ambiente::AMBIENTE ambiente = ESP32Sensors::Ambiente::medirAmbiente();
  if (!ambiente.valido) {
    Serial.println("[ERRO] Leituras DHT inválidas - pulando ciclo");
    return;
  }

  ESP32Sensors::LED::on();

  // Abre arquivo para gravar os dados coletados
  File f = SD.open(ARQ_DADOS, FILE_APPEND);
  if (!f) {
    Serial.println("[ERRO] Não abriu dados.csv para inclusão");
    ESP32Sensors::LED::off();
    return;
  }

  // Gera timestamp simples (millis)
  unsigned long timestamp = millis();
  
  // Escreve no arquivo
  f.printf("%lu,%.2f,%.2f,%.2f\n",
           timestamp,
           ambiente.temp,
           ambiente.umid,
           ambiente.ic);
  f.close();


  Serial.printf("[SALVO] T:%.1f°C U:%.1f%% IC:%.1f°C | Total de registros: %d",
               ambiente.temp, ambiente.umid, ambiente.ic, contarLinhasArquivo()-1);
  Serial.println("");
  ESP32Sensors::LED::off();
}

void imprimeArquivo() {
  File f = SD.open(ARQ_DADOS, FILE_READ);
  if (!f) {
    Serial.println("[ERRO] Falha ao abrir dados.csv para leitura");
    return;
  }
  
  Serial.println("\n========== DADOS ARMAZENADOS ==========");
  int linhas = 0;
  float somaTemp = 0, somaUmid = 0;
  float minTemp = 999, maxTemp = -999;
  
  while (f.available()) {
    String linha = f.readStringUntil('\n');
    Serial.println(linha);
    
    // Calcula estatísticas (pula cabeçalho)
    if (linhas > 0 && linha.length() > 10) {
      int pos1 = linha.indexOf(',');
      int pos2 = linha.indexOf(',', pos1 + 1);
      int pos3 = linha.indexOf(',', pos2 + 1);
      
      if (pos1 > 0 && pos2 > pos1 && pos3 > pos2) {
        float temp = linha.substring(pos1 + 1, pos2).toFloat();
        float umid = linha.substring(pos2 + 1, pos3).toFloat();
        
        somaTemp += temp;
        somaUmid += umid;
        if (temp < minTemp) minTemp = temp;
        if (temp > maxTemp) maxTemp = temp;
      }
    }
    linhas++;
  }
  f.close();
  
  // Mostra estatísticas
  if (linhas > 1) {
    Serial.println("========== ESTATÍSTICAS ==========");
    Serial.printf("Total de registros: %d\n", linhas - 1); Serial.println("");
    Serial.printf("Temperatura - Média: %.1f°C | Min: %.1f°C | Max: %.1f°C", 
                  somaTemp/(linhas-1), minTemp, maxTemp); Serial.println("");
    Serial.printf("Umidade média: %.1f%%\n", somaUmid/(linhas-1)); Serial.println("");
  }
  Serial.println("=====================================\n");
}

void zerarMedicoes() {
  Serial.println("[BOTÃO] Zerando medições...");

  // Remove arquivo existente
  if (SD.exists(ARQ_DADOS)) {
    SD.remove(ARQ_DADOS);
  }

  // Cria arquivo novamente com cabeçalho
  criaArquivoSeNecessario();
  Serial.println("[OK] Medições zeradas!");
}

// -------------------- Função MQTT --------------------
void enviarDadosViaMQTT() {
  if (!mqttConectado) {
    Serial.println("[MQTT] Não conectado - pulando envio");
    return;
  }

  File f = SD.open(ARQ_DADOS, FILE_READ);
  if (!f) {
    Serial.println("[MQTT] Erro ao abrir arquivo para envio");
    return;
  }

  Serial.println("[MQTT] Iniciando envio de dados...");
  ESP32Sensors::LED::on();
  
  int dadosEnviados = 0;
  String cabecalho = f.readStringUntil('\n'); // Pula cabeçalho
  
  while (f.available()) {
    String linha = f.readStringUntil('\n');
    linha.trim();
    
    if (linha.length() > 10) { // Linha válida
      // Parse da linha CSV: timestamp,temp,umid,ic
      int pos1 = linha.indexOf(',');
      int pos2 = linha.indexOf(',', pos1 + 1);
      int pos3 = linha.indexOf(',', pos2 + 1);
      
      if (pos1 > 0 && pos2 > pos1 && pos3 > pos2) {
        String timestamp = linha.substring(0, pos1);
        float temp = linha.substring(pos1 + 1, pos2).toFloat();
        float umid = linha.substring(pos2 + 1, pos3).toFloat();
        float ic = linha.substring(pos3 + 1).toFloat();
        
        // Cria JSON
        JsonDocument doc;
        doc["device_id"] = MQTT_DEVICEID;
        doc["timestamp"] = timestamp.toInt();
        doc["temperatura"] = temp;
        doc["umidade"] = umid;
        doc["indice_calor"] = ic;
        
        String buffer;
        serializeJson(doc, buffer);
        
        // Envia via MQTT
        bool enviado = mqttClient.publish(MQTT_PUB_TOPIC, buffer.c_str());
        
        if (enviado) {
          dadosEnviados++;
          Serial.printf("[MQTT] Enviado %d: T=%.1f°C U=%.1f%%", dadosEnviados, temp, umid); Serial.println("");
        } else {
          Serial.printf("[MQTT] Erro ao enviar registro %d\n", dadosEnviados + 1); Serial.println("");
        }
        
        delay(100); // Pequena pausa entre envios
      }
    }
  }
  f.close();
  
  ESP32Sensors::LED::off();
  
  if (dadosEnviados > 0) {
    Serial.printf("[MQTT] SUCESSO! Enviados %d registros com sucesso!\n", dadosEnviados); Serial.println("");
    Serial.println("[MQTT] Zerando armazenamento local...");
    zerarMedicoes();
  } else {
    Serial.println("[MQTT] Nenhum dado para enviar");
  }
}

// -------------------- Controle de Botões --------------------
void verificarBotoes() {
  bool btn_zerar_atual = digitalRead(BTN_ZERAR_PIN);
  bool btn_mostrar_atual = digitalRead(BTN_MOSTRAR_PIN);
  bool btn_enviar_atual = digitalRead(BTN_ENVIAR_PIN);
  
  // Botão Zerar (borda de descida)
  if (btn_zerar_atual == LOW) {
    zerarMedicoes();
    delay(300); // Debounce
  }
  
  // Botão Mostrar (borda de descida)
  if (btn_mostrar_atual == LOW) {
    imprimeArquivo();
    delay(300); // Debounce
  }
  
  // Botão Enviar (borda de descida)
  if (btn_enviar_atual == LOW) {
    if (mqttConectado) {
      enviarDadosViaMQTT();
    } else {
      Serial.println("[BOTÃO] MQTT não conectado - tentando reconectar...");
      conectarMQTT();
    }
    delay(300); // Debounce
  }
}

// -------------------- Função auxiliar --------------------
int contarLinhasArquivo() {
  File f = SD.open(ARQ_DADOS, FILE_READ);
  if (!f) return 0;
  
  int linhas = 0;
  while (f.available()) {
    f.readStringUntil('\n');
    linhas++;
  }
  f.close();
  return linhas;
}