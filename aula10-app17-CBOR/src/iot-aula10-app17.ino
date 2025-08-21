/* ========================================
 * SISTEMA IoT - EDGE COMPUTING
 * ESP32 + WiFi + MQTT + CBOR + SD Card
 * Coleta dados, armazena em CBOR e envia em lote
 * ======================================== */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <cbor.h>
#include <mbedtls/base64.h>  // Para codificar CBOR em Base64 para MQTT
#include "ESP32Sensors.hpp"

/* ===================== Config Wi‑Fi ===================== */
const char* WIFI_SSID     = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
WiFiClient wifiClient;

/* ===================== Config MQTT ===================== */
#define MQTT_SERVER     "broker.emqx.io"   // Broker público para testes
#define MQTT_PORT       1883
#define MQTT_PUB_TOPIC  "FIAPIoT/aula10/noris/dados/cbor"    // Tópico para dados CBOR
#define MQTT_SUB_TOPIC  "FIAPIoT/aula10/noris/motor/cmd"     // Tópico para comandos
#define MQTT_PUB_STATUS "FIAPIoT/aula10/noris/status"        // Tópico para status
// ATENÇÃO: ClientID DEVE SER ÚNICO NO BROKER!
#define MQTT_CLIENT_ID  "IoTDeviceNoris001_CBOR"
PubSubClient mqttClient(wifiClient);

/* ===================== Configurações de Tempo ===================== */
const uint32_t INTERVALO_COLETA_MS = 5000;      // 5 segundos entre coletas
const uint32_t INTERVALO_ENVIO_MS = 60000;      // 60 segundos para enviar lote
const int MAX_MEDICOES_LOTE = 12;               // Máximo de medições por lote (60s / 5s = 12)

/* ===================== Variáveis Globais ===================== */
unsigned long ultimaColeta = 0;
unsigned long ultimoEnvio = 0;
bool motorLigado = true;
bool wifiConectado = false;
bool mqttConectado = false;

/* ===================== Protótipos ===================== */
void conectarWiFi();
void conectarMQTT();
void callbackMQTT(char* topic, byte* payload, unsigned int length);
void coletarEArmazenar();
bool enviarLoteCBOR();
void publicarStatus();
String cborParaBase64(uint8_t* dados, size_t tamanho);
void decodificarComandoCBOR(byte* payload, unsigned int length);

/* ===================== SETUP ===================== */
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n========================================");
    Serial.println("   SISTEMA IoT - EDGE COMPUTING        ");
    Serial.println("   WiFi + MQTT + CBOR + SD Card        ");
    Serial.println("========================================");
    Serial.println("Demonstração: Armazenamento local CBOR");
    Serial.println("             e envio em lote via MQTT  ");
    Serial.println("========================================\n");
    
    // Inicializa todos os sensores
    Serial.println("[INIT] Inicializando sensores...");
    ESP32Sensors::beginAll();
    
    // Inicializa SD Card
    if (!ESP32Sensors::SDCard::inicializar()) {
        Serial.println("[ERRO] SD Card falhou - continuando sem persistência");
    }
    
    // Inicializa estado do motor
    ESP32Sensors::LED::on();
    motorLigado = true;
    Serial.println("[MOTOR] Motor iniciado (LED ligado)");
    
    // Conecta WiFi
    conectarWiFi();
    
    // Configura e conecta MQTT
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(callbackMQTT);
    mqttClient.setBufferSize(4096);  // Aumenta buffer para dados CBOR
    conectarMQTT();
    
    Serial.println("\n[OK] Sistema pronto!");
    Serial.println("[INFO] Coletando dados a cada 5 segundos");
    Serial.println("[INFO] Enviando lote CBOR a cada 60 segundos\n");
}

/* ===================== LOOP ===================== */
void loop() {
    // Mantém conexão MQTT
    if (!mqttClient.connected()) {
        conectarMQTT();
    }
    mqttClient.loop();
    
    // Coleta dados a cada 5 segundos
    if (millis() - ultimaColeta >= INTERVALO_COLETA_MS) {
        ultimaColeta = millis();
        coletarEArmazenar();
    }
    
    // Envia lote a cada 60 segundos
    if (millis() - ultimoEnvio >= INTERVALO_ENVIO_MS) {
        ultimoEnvio = millis();
        
        int medicoes = ESP32Sensors::SDCard::obterContadorMedicoes();
        if (medicoes > 0) {
            Serial.printf("\n[LOTE] Tempo de envio! %d medições no buffer\n", medicoes); Serial.println("");
            
            if (enviarLoteCBOR()) {
                ESP32Sensors::SDCard::limparBufferCBOR();
            } else {
                Serial.println("[ERRO] Falha ao enviar lote - mantendo buffer");
            }
        } else {
            Serial.println("\n[LOTE] Nenhuma medição para enviar");
        }
    }
    
    delay(100);
}

/* ===================== FUNÇÕES AUXILIARES ===================== */

// ---------- Conectar WiFi ----------
void conectarWiFi() {
    Serial.printf("[WiFi] Conectando a %s", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int tentativas = 0;
    while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
        delay(500);
        Serial.print(".");
        tentativas++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[WiFi] Conectado!");
        Serial.print("[WiFi] IP: ");
        Serial.println(WiFi.localIP());
        wifiConectado = true;
        ESP32Sensors::LED::on();
    } else {
        Serial.println("\n[WiFi] Falha na conexão!");
        wifiConectado = false;
    }
}

// ---------- Conectar MQTT ----------
void conectarMQTT() {
    if (!wifiConectado) {
        return;
    }
    
    while (!mqttClient.connected()) {
        Serial.printf("[MQTT] Conectando a %s...", MQTT_SERVER);
        
        if (mqttClient.connect(MQTT_CLIENT_ID)) {
            Serial.println(" Conectado!");
            mqttConectado = true;
            
            // Inscreve no tópico de comandos
            mqttClient.subscribe(MQTT_SUB_TOPIC);
            Serial.printf("[MQTT] Inscrito em: %s\n", MQTT_SUB_TOPIC); Serial.println("");
            
            // Publica status inicial
            publicarStatus();
            
        } else {
            Serial.printf(" Falha (rc=%d). Tentando em 5s...\n", mqttClient.state());
            mqttConectado = false;
            delay(5000);
        }
    }
}

// ---------- Callback MQTT ----------
void callbackMQTT(char* topic, byte* payload, unsigned int length) {
    Serial.println("\n========================================");
    Serial.println("COMANDO RECEBIDO:");
    Serial.printf("Tópico: %s\n", topic);
    Serial.printf("Tamanho: %d bytes\n", length);
    
    // Tenta interpretar como texto simples primeiro
    String comando;
    for (unsigned int i = 0; i < length; i++) {
        comando += (char)payload[i];
    }
    comando.toUpperCase();
    
    // Comandos simples de texto
    if (comando == "ON") {
        ESP32Sensors::LED::on();
        motorLigado = true;
        Serial.println("COMANDO: Motor LIGADO");
        publicarStatus();
    }
    else if (comando == "OFF") {
        ESP32Sensors::LED::off();
        motorLigado = false;
        Serial.println("COMANDO: Motor DESLIGADO");
        publicarStatus();
    }
    else if (comando == "STATUS") {
        publicarStatus();
    }
    else if (comando == "DUMP") {
        // Comando para debug - imprime arquivo CSV
        ESP32Sensors::SDCard::imprimirArquivoCSV();
    }
    else {
        // Tenta interpretar como CBOR
        decodificarComandoCBOR(payload, length);
    }
    
    Serial.println("========================================\n");
}

// ---------- Coletar e Armazenar ----------
void coletarEArmazenar() {
    Serial.println("\n--- Coleta de Dados ---");
    
    // Lê sensores
    ESP32Sensors::Ambiente::AMBIENTE ambiente = ESP32Sensors::Ambiente::medirAmbiente();
    sensors_event_t accel = ESP32Sensors::Accel::medirAccel();

    if (!ambiente.valido) {
        Serial.println("Erro na leitura do DHT22");
    }

    // Exibe dados coletados
    Serial.printf("[SENSOR] Temp: %.2f°C | Umid: %.2f%% | IC: %.2f°C\n",
                  ambiente.temp, ambiente.umid, ambiente.ic); Serial.println("");
    Serial.printf("[SENSOR] Accel: X=%.2f Y=%.2f Z=%.2f m/s²\n",
                  accel.acceleration.x, accel.acceleration.y, accel.acceleration.z); Serial.println("");
    Serial.printf("[MOTOR] Status: %s\n", motorLigado ? "LIGADO" : "DESLIGADO"); Serial.println("");
    
    // Cria estrutura de medição
    ESP32Sensors::SDCard::Medicao medicao;
    medicao.device = MQTT_CLIENT_ID;
    medicao.temp = ambiente.temp;
    medicao.umid = ambiente.umid;
    medicao.ic = ambiente.ic;
    medicao.accel_x = accel.acceleration.x;
    medicao.accel_y = accel.acceleration.y;
    medicao.accel_z = accel.acceleration.z;
    medicao.motor_status = motorLigado;
    medicao.timestamp = millis();
    
    // Salva no SD Card em CBOR
    if (ESP32Sensors::SDCard::salvarMedicaoCBOR(medicao)) {
        int total = ESP32Sensors::SDCard::obterContadorMedicoes();
        Serial.printf("[SD] Total no buffer: %d medições\n", total); Serial.println("");
        
        // LED pisca para indicar gravação
        ESP32Sensors::LED::off();
        delay(50);
        if (motorLigado) ESP32Sensors::LED::on();
    } else {
        Serial.println("[ERRO] Falha ao salvar medição");
    }
}

// ---------- Enviar Lote CBOR ----------
// ---------- Enviar Lote CBOR ----------
bool enviarLoteCBOR() {
    // Obtém tamanho do buffer JSONL original
    size_t tamanhoJSONL = ESP32Sensors::SDCard::obterTamanhoBuffer();
    Serial.printf("\n[COMPARAÇÃO] Tamanho do buffer JSONL: %d bytes\n", tamanhoJSONL); Serial.println("");
    
    // Cria lote CBOR
    size_t tamanhoCBOR = 0;
    uint8_t* loteCBOR = ESP32Sensors::SDCard::criarLoteCBOR(tamanhoCBOR);
    
    if (!loteCBOR || tamanhoCBOR == 0) {
        Serial.println("[ERRO] Falha ao criar lote CBOR");
        return false;
    }
    
    // Calcula e exibe redução de tamanho
    Serial.printf("[COMPARAÇÃO] Tamanho do lote CBOR: %d bytes\n", tamanhoCBOR); Serial.println("");
    
    // Verifica se precisa enviar em chunks ou pode enviar direto
    bool sucesso = false;
    
    // Envia direto como Base64 (para compatibilidade com MQTT/JSON)
    String base64Data = cborParaBase64(loteCBOR, tamanhoCBOR);
        
    // Cria JSON wrapper
    JsonDocument doc;
    doc["type"] = "cbor_batch";
    doc["encoding"] = "base64";
    doc["size"] = tamanhoCBOR;
    // doc["size_original_jsonl"] = tamanhoJSONL;  // Adiciona tamanho original para comparação
    // doc["compression_ratio"] = (tamanhoJSONL > 0) ? ((float)tamanhoCBOR / (float)tamanhoJSONL) : 1.0;
    doc["data"] = base64Data;
        
    String jsonBuffer;
    serializeJson(doc, jsonBuffer);
        
    sucesso = mqttClient.publish(MQTT_PUB_TOPIC, jsonBuffer.c_str());
    
    if (sucesso) {
        Serial.printf("[MQTT] Lote enviado: %d bytes CBOR (redução de %.1f%%)\n", 
                        tamanhoCBOR, 
                        ((float)(tamanhoJSONL - tamanhoCBOR) / (float)tamanhoJSONL) * 100.0); Serial.println("");
    } else {
        Serial.println("[MQTT] Falha ao publicar lote");
    }
    
    // Limpa memória
    delete[] loteCBOR;
    
    return sucesso;
}

// ---------- Publicar Status ----------
void publicarStatus() {
    JsonDocument doc;
    
    doc["device"] = MQTT_CLIENT_ID;
    doc["uptime"] = millis();
    doc["motor"] = motorLigado;
    doc["buffer_size"] = ESP32Sensors::SDCard::obterTamanhoBuffer();
    doc["buffer_count"] = ESP32Sensors::SDCard::obterContadorMedicoes();
    doc["sd_ok"] = ESP32Sensors::SDCard::estaOK();
    
    String buffer;
    serializeJson(doc, buffer);
    
    if (mqttClient.publish(MQTT_PUB_STATUS, buffer.c_str())) {
        Serial.println("[MQTT] Status publicado");
    }
}

// ---------- CBOR para Base64 ----------
// String cborParaBase64(uint8_t* dados, size_t tamanho) {
//     // Codifica dados binários em Base64 para transmissão via JSON/MQTT
//     char* base64 = new char[base64_enc_len(tamanho) + 1];
//     base64_encode(base64, (char*)dados, tamanho);
//     String resultado = String(base64);
//     delete[] base64;
//     return resultado;
// }
String cborParaBase64(uint8_t* dados, size_t tamanho) {
    if (!dados || tamanho == 0) return String();

    size_t necessario = 0;
    // 1) Descobre o tamanho necessário (vai retornar BUFFER_TOO_SMALL com 'necessario' preenchido)
    int ret = mbedtls_base64_encode(
        nullptr,          // sem buffer de saída
        0,                // tamanho 0 de saída
        &necessario,      // aqui vem o tamanho necessário
        dados,            // entrada (CBOR)
        tamanho
    );

    if (ret != MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL && ret != 0) {
        Serial.printf("[Base64] Erro ao estimar tamanho (cod=%d)\n", ret);
        return String();
    }

    // +1 para terminador NUL
    unsigned char* out = new unsigned char[necessario + 1];
    if (!out) {
        Serial.println("[Base64] Falha ao alocar memória");
        return String();
    }

    size_t escrito = 0;
    // 2) Codifica de fato
    ret = mbedtls_base64_encode(
        out,               // buffer de saída
        necessario,        // tamanho do buffer de saída
        &escrito,          // tamanho realmente escrito
        dados,             // entrada (CBOR)
        tamanho            // tamanho da entrada
    );

    if (ret != 0) {
        Serial.printf("[Base64] Erro ao codificar (cod=%d)\n", ret);
        delete[] out;
        return String();
    }

    out[escrito] = '\0';
    String resultado = String((char*)out);
    delete[] out;
    return resultado;
}


// ---------- Decodificar Comando CBOR ----------
void decodificarComandoCBOR(byte* payload, unsigned int length) {
    // Tenta decodificar comando CBOR
    CborParser parser;
    CborValue value;
    
    if (cbor_parser_init(payload, length, 0, &parser, &value) != CborNoError) {
        Serial.println("[CBOR] Payload não é CBOR válido");
        return;
    }
    
    if (cbor_value_is_map(&value)) {
        CborValue mapValue;
        cbor_value_enter_container(&value, &mapValue);
        
        while (!cbor_value_at_end(&mapValue)) {
            // Lê chave
            if (cbor_value_is_text_string(&mapValue)) {
                char key[32];
                size_t keyLen = sizeof(key);
                cbor_value_copy_text_string(&mapValue, key, &keyLen, &mapValue);
                
                // Processa valor baseado na chave
                // if (strcmp(key, "servo") == 0 && cbor_value_is_integer(&mapValue)) {
                //     int angle;
                //     cbor_value_get_int(&mapValue, &angle);
                //     ESP32Sensors::Servo::mover(angle);
                //     Serial.printf("[CBOR] Servo movido para %d°\n", angle);
                // }
                // Adicione mais comandos CBOR conforme necessário
                
                cbor_value_advance(&mapValue);
            } else {
                cbor_value_advance(&mapValue);
            }
        }
    }
}