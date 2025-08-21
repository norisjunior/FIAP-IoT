#ifndef ESP32SENSORS_SDCARD_HPP
#define ESP32SENSORS_SDCARD_HPP

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <cbor.h>
#include <ArduinoJson.h>   // <- ArduinoJson v7 (JsonDocument)

namespace ESP32Sensors {
namespace SDCard {

// ==================== Configurações ====================
const gpio_num_t CS_PIN = GPIO_NUM_5;              // Chip Select do SD Card
const char* ARQ_BUFFER_CBOR = "/buffer.jsonl";     // 1 JSON por linha (JSONL)
const char* ARQ_LOG_CSV     = "/log.csv";          // opcional (debug)
const int   MAX_BUFFER_SIZE = 4096;                // tamanho base do buffer CBOR

// ==================== Estrutura da Medição ====================
struct Medicao {
    float temp;
    float umid;
    float ic;
    float accel_x;
    float accel_y;
    float accel_z;
    bool  motor_status;
    uint32_t timestamp;
    String device;
};

// ==================== Estado ====================
static bool sd_ok = false;

// ==================== Utilitários ====================
static bool ensureFileExists(const char* path) {
    if (!SD.exists(path)) {
        File f = SD.open(path, FILE_WRITE);
        if (!f) return false;
        f.close();
    }
    return true;
}

// ==================== Inicialização ====================
bool inicializar() {
    if (sd_ok) return true;
    if (!SD.begin(CS_PIN)) {
        Serial.println("[SD] Falha ao inicializar SD");
        sd_ok = false;
        return false;
    }
    sd_ok = true;

    ensureFileExists(ARQ_BUFFER_CBOR);
    if (!SD.exists(ARQ_LOG_CSV)) {
        File f = SD.open(ARQ_LOG_CSV, FILE_WRITE);
        if (f) {
            f.println("timestamp,device,temp,umid,ic,accel_x,accel_y,accel_z,motor");
            f.close();
        }
    }
    Serial.println("[SD] SD pronto");
    return true;
}

// ==================== Salvar medição (JSON por linha, com JsonDocument) ====================
bool salvarMedicaoCBOR(const Medicao& m) {
    if (!sd_ok && !inicializar()) return false;

    JsonDocument doc;  
    doc["device"]       = m.device.length() ? m.device : "ESP32_AULA_IOT_BR_001";
    doc["temp"]         = m.temp;
    doc["umid"]         = m.umid;
    doc["ic"]           = m.ic;
    doc["accel_x"]      = m.accel_x;
    doc["accel_y"]      = m.accel_y;
    doc["accel_z"]      = m.accel_z;
    doc["motor_status"] = m.motor_status ? "true" : "false"; 
    //doc["ts"]           = m.timestamp;

    File bf = SD.open(ARQ_BUFFER_CBOR, FILE_APPEND);
    if (!bf) {
        Serial.println("[SD] Falha ao abrir buffer JSONL");
        return false;
    }
    // Escreve uma linha JSON (JSONL)
    serializeJson(doc, bf);
    bf.print('\n');
    bf.close();

    // (Opcional) CSV para depuração
    File cf = SD.open(ARQ_LOG_CSV, FILE_APPEND);
    if (cf) {
        cf.printf("%lu,%s,%.2f,%.2f,%.5f,%.5f,%.5f,%.5f,%s\n",
                  (unsigned long)m.timestamp, m.device.c_str(),
                  m.temp, m.umid, m.ic, m.accel_x, m.accel_y, m.accel_z,
                  m.motor_status ? "true" : "false");
        cf.close();
    }
    return true;
}

// ==================== Criar Lote CBOR (a partir do JSONL) ====================
uint8_t* criarLoteCBOR(size_t& outSize) {
    outSize = 0;
    if (!sd_ok && !inicializar()) return nullptr;

    File f = SD.open(ARQ_BUFFER_CBOR, FILE_READ);
    if (!f) {
        Serial.println("[SD] Não foi possível abrir buffer JSONL para leitura");
        return nullptr;
    }

    size_t cap = MAX_BUFFER_SIZE;
    uint8_t* buffer = nullptr;

    while (true) {  // tenta com cap; se faltar memória, dobra e recomeça
        if (buffer) { delete[] buffer; buffer = nullptr; }
        buffer = new uint8_t[cap];
        if (!buffer) { f.close(); return nullptr; }

        CborEncoder root, arr, map;
        cbor_encoder_init(&root, buffer, cap, 0);

        CborError err = CborNoError;

        err = cbor_encoder_create_array(&root, &arr, CborIndefiniteLength);
        if (err != CborNoError) { delete[] buffer; f.close(); return nullptr; }

        f.seek(0);  // recomeça da primeira linha a cada tentativa
        String linha;
        size_t linhas = 0;
        bool outOfMem = false;

        while (f.available()) {
            linha = f.readStringUntil('\n');
            linha.trim();
            if (linha.length() == 0) continue;

            JsonDocument doc;
            DeserializationError jerr = deserializeJson(doc, linha);
            if (jerr) {
                Serial.println("[SD] Linha JSON inválida – ignorando");
                continue;
            }

            err = cbor_encoder_create_map(&arr, &map, CborIndefiniteLength);
            if (err == CborErrorOutOfMemory) { outOfMem = true; break; }
            if (err != CborNoError) { delete[] buffer; f.close(); return nullptr; }

            // device
            const char* device = doc["device"].is<const char*>() ? doc["device"].as<const char*>() : "";
            err = cbor_encode_text_stringz(&map, "device");
            if (err == CborErrorOutOfMemory) { outOfMem = true; break; }
            if (err != CborNoError) { delete[] buffer; f.close(); return nullptr; }

            err = cbor_encode_text_stringz(&map, device);
            if (err == CborErrorOutOfMemory) { outOfMem = true; break; }
            if (err != CborNoError) { delete[] buffer; f.close(); return nullptr; }

            // números (aceita float/double/int)
            auto putNumber = [&](const char* k, const char* jkey) -> bool {
                if (!doc[jkey].isNull()) {
                    err = cbor_encode_text_stringz(&map, k);
                    if (err == CborErrorOutOfMemory) { outOfMem = true; return false; }
                    if (err != CborNoError) return false;

                    // usa float para compactar
                    float v = doc[jkey].as<float>();
                    err = cbor_encode_float(&map, v);
                    if (err == CborErrorOutOfMemory) { outOfMem = true; return false; }
                    if (err != CborNoError) return false;
                }
                return true;
            };

            if (!putNumber("temp", "temp")) break;
            if (!putNumber("umid", "umid")) break;
            if (!putNumber("ic", "ic")) break;
            if (!putNumber("accel_x", "accel_x")) break;
            if (!putNumber("accel_y", "accel_y")) break;
            if (!putNumber("accel_z", "accel_z")) break;

            // motor_status: string "true"/"false" no arquivo → boolean em CBOR
            err = cbor_encode_text_stringz(&map, "motor_status");
            if (err == CborErrorOutOfMemory) { outOfMem = true; break; }
            if (err != CborNoError) { delete[] buffer; f.close(); return nullptr; }

            bool mstatus = false;
            if (doc["motor_status"].is<bool>()) {
                mstatus = doc["motor_status"].as<bool>();
            } else if (doc["motor_status"].is<const char*>()) {
                const char* s = doc["motor_status"].as<const char*>();
                mstatus = (strcmp(s, "true") == 0 || strcmp(s, "1") == 0);
            }
            err = cbor_encode_boolean(&map, mstatus);
            if (err == CborErrorOutOfMemory) { outOfMem = true; break; }
            if (err != CborNoError) { delete[] buffer; f.close(); return nullptr; }

            // ts (opcional)
            if (!doc["ts"].isNull()) {
                err = cbor_encode_text_stringz(&map, "ts");
                if (err == CborErrorOutOfMemory) { outOfMem = true; break; }
                if (err != CborNoError) { delete[] buffer; f.close(); return nullptr; }

                err = cbor_encode_uint(&map, doc["ts"].as<uint32_t>());
                if (err == CborErrorOutOfMemory) { outOfMem = true; break; }
                if (err != CborNoError) { delete[] buffer; f.close(); return nullptr; }
            }

            // fecha o mapa da medição
            err = cbor_encoder_close_container(&arr, &map);
            if (err == CborErrorOutOfMemory) { outOfMem = true; break; }
            if (err != CborNoError) { delete[] buffer; f.close(); return nullptr; }

            ++linhas;
        }

        if (outOfMem) {
            // dobra buffer e tenta tudo de novo
            cap *= 2;
            continue;
        }

        // fecha o array do lote
        err = cbor_encoder_close_container(&root, &arr);
        if (err == CborErrorOutOfMemory) { cap *= 2; continue; }
        if (err != CborNoError) { delete[] buffer; f.close(); return nullptr; }

        // sucesso
        size_t used = cbor_encoder_get_buffer_size(&root, buffer);
        uint8_t* out = new uint8_t[used];
        if (!out) { delete[] buffer; f.close(); return nullptr; }
        memcpy(out, buffer, used);
        delete[] buffer;

        outSize = used;
        Serial.printf("[SD] Lote CBOR criado de %zu bytes\n", outSize); Serial.println("");
        f.close();
        return out;
    }
}

// ==================== Imprimir CSV (debug) ====================
void imprimirArquivoCSV() {
    if (!sd_ok && !inicializar()) return;
    if (!SD.exists(ARQ_LOG_CSV)) {
        Serial.println("[SD] Sem CSV");
        return;
    }
    File f = SD.open(ARQ_LOG_CSV, FILE_READ);
    if (!f) return;
    Serial.println("----- log.csv -----");
    while (f.available()) Serial.write(f.read());
    Serial.println("\n-------------------");
    f.close();
}

// ==================== Limpar Buffer (JSONL) ====================
void limparBufferCBOR() {
    if (!sd_ok && !inicializar()) return;
    if (SD.exists(ARQ_BUFFER_CBOR)) SD.remove(ARQ_BUFFER_CBOR);
    ensureFileExists(ARQ_BUFFER_CBOR);
    Serial.println("[SD] Buffer (JSONL) limpo");
}

// ==================== Contadores/Tamanho ====================
int obterContadorMedicoes() {
    if (!sd_ok && !inicializar()) return 0;
    if (!SD.exists(ARQ_BUFFER_CBOR)) return 0;
    File f = SD.open(ARQ_BUFFER_CBOR, FILE_READ);
    if (!f) return 0;
    int linhas = 0;
    while (f.available()) if (f.read() == '\n') ++linhas;
    f.close();
    return linhas;
}

size_t obterTamanhoBuffer() {
    if (!sd_ok && !inicializar()) return 0;
    if (!SD.exists(ARQ_BUFFER_CBOR)) return 0;
    File f = SD.open(ARQ_BUFFER_CBOR, FILE_READ);
    if (!f) return 0;
    size_t s = f.size();
    f.close();
    return s;
}

// ==================== Verificação básica ====================
bool estaOK() { return sd_ok; }

} // namespace SDCard
} // namespace ESP32Sensors

#endif // ESP32SENSORS_SDCARD_HPP




// #ifndef ESP32SENSORS_SDCARD_HPP
// #define ESP32SENSORS_SDCARD_HPP

// #include <Arduino.h>
// #include <SD.h>
// #include <SPI.h>
// #include <cbor.h>

// namespace ESP32Sensors {
//     namespace SDCard {
//         // ==================== Configurações ====================
//         const gpio_num_t CS_PIN = GPIO_NUM_5;          // Chip Select do SD Card
//         const char* ARQ_BUFFER_CBOR = "/buffer.cbor";  // Arquivo buffer em CBOR
//         const char* ARQ_LOG_CSV = "/log.csv";          // Arquivo log em CSV (backup/debug)
//         const int MAX_BUFFER_SIZE = 4096;              // Tamanho máximo do buffer (4KB)
        
//         // ==================== Variáveis Internas ====================
//         bool sdCardOK = false;
//         int contadorMedicoes = 0;
        
//         // ==================== Estrutura de Medição ====================
//         struct Medicao {
//             float temp;
//             float umid;
//             float ic;
//             float accel_x;
//             float accel_y;
//             float accel_z;
//             bool motor_status;
//             unsigned long timestamp;
//         };

//         // ==================== Criar Arquivo CSV ====================
//         void criarArquivoCSVSeNecessario() {
//             if (SD.exists(ARQ_LOG_CSV)) {
//                 Serial.println("[SD] Arquivo CSV já existe");
//                 return;
//             }
            
//             File f = SD.open(ARQ_LOG_CSV, FILE_WRITE);
//             if (!f) {
//                 Serial.println("[SD] ERRO: Não criou arquivo CSV");
//                 return;
//             }
            
//             // Cabeçalho CSV
//             f.println("timestamp,temp_C,umid_%,ic_C,accel_x,accel_y,accel_z,motor");
//             f.close();
            
//             Serial.println("[SD] Arquivo CSV criado com sucesso");
//         }
        
//         // ==================== Limpar Buffer CBOR ====================
//         void limparBufferCBOR() {
//             if (SD.exists(ARQ_BUFFER_CBOR)) {
//                 SD.remove(ARQ_BUFFER_CBOR);
//                 Serial.println("[SD] Buffer CBOR anterior removido");
//             }
//             contadorMedicoes = 0;
//         }
                
//         // ==================== Inicialização ====================
//         bool inicializar() {
//             Serial.println("[SD] Inicializando cartão SD...");
            
//             // Inicia SD (barramento VSPI padrão: SCK 18, MISO 19, MOSI 23)
//             if (!SD.begin(CS_PIN)) {
//                 Serial.println("[SD] ERRO: Falha ao iniciar SD Card!"); 
//                 sdCardOK = false;
//                 return false;
//             }
            
//             uint64_t cardSize = SD.cardSize() / (1024 * 1024);
//             Serial.printf("[SD] Cartão OK - %llu MB\n", cardSize); Serial.println("");
            
//             sdCardOK = true;
            
//             // Cria arquivo CSV se não existir (para debug)
//             criarArquivoCSVSeNecessario();
            
//             // Limpa buffer CBOR antigo se existir
//             limparBufferCBOR();
            
//             return true;
//         }

//         // ==================== Salvar Medição em CSV (Debug) ====================
//         void salvarMedicaoCSV(const Medicao& medicao) {
//             File csvFile = SD.open(ARQ_LOG_CSV, FILE_APPEND);
//             if (!csvFile) {
//                 return;  // Falha silenciosa, CSV é só debug
//             }
            
//             csvFile.printf("%lu,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d\n",
//                           medicao.timestamp,
//                           medicao.temp,
//                           medicao.umid,
//                           medicao.ic,
//                           medicao.accel_x,
//                           medicao.accel_y,
//                           medicao.accel_z,
//                           medicao.motor_status ? 1 : 0);
//             csvFile.close();
//         }
        
//         // ==================== Salvar Medição em CBOR ====================
//         bool salvarMedicaoCBOR(const Medicao& medicao) {
//             if (!sdCardOK) {
//                 Serial.println("[SD] ERRO: SD Card não disponível");
//                 return false;
//             }
            
//             // Prepara buffer para codificação CBOR
//             uint8_t buffer[256];
//             CborEncoder encoder;
//             CborEncoder mapEncoder;
            
//             // Inicializa encoder
//             cbor_encoder_init(&encoder, buffer, sizeof(buffer), 0);
            
//             // Cria mapa com 8 campos
//             cbor_encoder_create_map(&encoder, &mapEncoder, 8);
            
//             // Adiciona campos
//             cbor_encode_text_stringz(&mapEncoder, "ts");
//             cbor_encode_uint(&mapEncoder, medicao.timestamp);
            
//             cbor_encode_text_stringz(&mapEncoder, "temp");
//             cbor_encode_float(&mapEncoder, medicao.temp);
            
//             cbor_encode_text_stringz(&mapEncoder, "umid");
//             cbor_encode_float(&mapEncoder, medicao.umid);
            
//             cbor_encode_text_stringz(&mapEncoder, "ic");
//             cbor_encode_float(&mapEncoder, medicao.ic);
            
//             cbor_encode_text_stringz(&mapEncoder, "ax");
//             cbor_encode_float(&mapEncoder, medicao.accel_x);
            
//             cbor_encode_text_stringz(&mapEncoder, "ay");
//             cbor_encode_float(&mapEncoder, medicao.accel_y);
            
//             cbor_encode_text_stringz(&mapEncoder, "az");
//             cbor_encode_float(&mapEncoder, medicao.accel_z);
            
//             cbor_encode_text_stringz(&mapEncoder, "motor");
//             cbor_encode_boolean(&mapEncoder, medicao.motor_status);
            
//             // Fecha o mapa
//             cbor_encoder_close_container(&encoder, &mapEncoder);
            
//             // Calcula tamanho real
//             size_t bufferSize = cbor_encoder_get_buffer_size(&encoder, buffer);
            
//             // Abre arquivo CBOR para append
//             File cborFile = SD.open(ARQ_BUFFER_CBOR, FILE_APPEND);
//             if (!cborFile) {
//                 Serial.println("[SD] ERRO: Não abriu buffer CBOR"); Serial.println("");
//                 return false;
//             }
            
//             // Escreve dados CBOR
//             cborFile.write(buffer, bufferSize);
//             cborFile.close();
            
//             contadorMedicoes++;
//             Serial.printf("[SD] Medição %d salva em CBOR (%d bytes)\n", contadorMedicoes, bufferSize); Serial.println("");
            
//             // Também salva em CSV para debug (opcional)
//             salvarMedicaoCSV(medicao);
            
//             return true;
//         }
        
//         // ==================== Ler Buffer CBOR Completo ====================
//         uint8_t* lerBufferCBOR(size_t& tamanho) {
//             if (!SD.exists(ARQ_BUFFER_CBOR)) {
//                 Serial.println("[SD] AVISO: Buffer CBOR vazio");
//                 tamanho = 0;
//                 return nullptr;
//             }
            
//             File cborFile = SD.open(ARQ_BUFFER_CBOR, FILE_READ);
//             if (!cborFile) {
//                 Serial.println("[SD] ERRO: Não conseguiu ler buffer CBOR");
//                 tamanho = 0;
//                 return nullptr;
//             }
            
//             tamanho = cborFile.size();
//             uint8_t* buffer = new uint8_t[tamanho];
            
//             if (cborFile.read(buffer, tamanho) != tamanho) {
//                 Serial.println("[SD] ERRO: Falha na leitura do buffer");
//                 delete[] buffer;
//                 tamanho = 0;
//                 cborFile.close();
//                 return nullptr;
//             }
            
//             cborFile.close();
//             Serial.printf("[SD] Buffer CBOR lido: %d bytes\n", tamanho); Serial.println("");
//             return buffer;
//         }
        
//         // ==================== Criar Lote CBOR para Envio ====================
//         uint8_t* criarLoteCBOR(size_t& tamanhoLote) {
//             // Lê buffer existente
//             size_t tamanhoBuffer;
//             uint8_t* bufferMedicoes = lerBufferCBOR(tamanhoBuffer);
            
//             if (!bufferMedicoes || tamanhoBuffer == 0) {
//                 Serial.println("[SD] Nenhuma medição para enviar");
//                 tamanhoLote = 0;
//                 return nullptr;
//             }
            
//             // Cria container CBOR para o lote
//             uint8_t* loteBuffer = new uint8_t[tamanhoBuffer + 256];  // Extra para metadata
//             CborEncoder encoder;
//             CborEncoder mapEncoder;
            
//             cbor_encoder_init(&encoder, loteBuffer, tamanhoBuffer + 256, 0);
            
//             // Cria mapa principal com metadata
//             cbor_encoder_create_map(&encoder, &mapEncoder, 4);
            
//             // Campo 1: Device ID
//             cbor_encode_text_stringz(&mapEncoder, "device");
//             cbor_encode_text_stringz(&mapEncoder, "ESP32_IoT_001");
            
//             // Campo 2: Timestamp do lote
//             cbor_encode_text_stringz(&mapEncoder, "batch_ts");
//             cbor_encode_uint(&mapEncoder, millis());
            
//             // Campo 3: Quantidade de medições
//             cbor_encode_text_stringz(&mapEncoder, "count");
//             cbor_encode_int(&mapEncoder, contadorMedicoes);
            
//             // Campo 4: Dados (buffer de medições)
//             cbor_encode_text_stringz(&mapEncoder, "data");
//             cbor_encode_byte_string(&mapEncoder, bufferMedicoes, tamanhoBuffer);
            
//             // Fecha o mapa
//             cbor_encoder_close_container(&encoder, &mapEncoder);
            
//             // Calcula tamanho real do lote
//             tamanhoLote = cbor_encoder_get_buffer_size(&encoder, loteBuffer);
            
//             // Limpa buffer temporário
//             delete[] bufferMedicoes;
            
//             Serial.printf("[SD] Lote CBOR criado: %d medições, %d bytes\n", 
//                          contadorMedicoes, tamanhoLote); Serial.println("");
            
//             return loteBuffer;
//         }
        
//         // ==================== Obter Contador de Medições ====================
//         int obterContadorMedicoes() {
//             return contadorMedicoes;
//         }
        
//         // ==================== Verificar Status do SD Card ====================
//         bool estaOK() {
//             return sdCardOK;
//         }
        
//         // ==================== Imprimir Arquivo CSV (Debug) ====================
//         void imprimirArquivoCSV() {
//             File f = SD.open(ARQ_LOG_CSV, FILE_READ);
//             if (!f) {
//                 Serial.println("[SD] ERRO: Não abriu arquivo CSV");
//                 return;
//             }
            
//             Serial.println("\n--- Conteúdo do arquivo CSV ---");
//             while (f.available()) {
//                 String linha = f.readStringUntil('\n');
//                 Serial.println(linha);
//             }
//             Serial.println("--- Fim do arquivo ---\n");
//             f.close();
//         }
        
//         // ==================== Obter Tamanho do Buffer ====================
//         size_t obterTamanhoBuffer() {
//             if (!SD.exists(ARQ_BUFFER_CBOR)) {
//                 return 0;
//             }
            
//             File f = SD.open(ARQ_BUFFER_CBOR, FILE_READ);
//             if (!f) {
//                 return 0;
//             }
            
//             size_t tamanho = f.size();
//             f.close();
//             return tamanho;
//         }
//     }
// }

// #endif // ESP32SENSORS_SDCARD_HPP