// lwm2m_coap_fixed.cpp
#include "lwm2m_coap_fixed.h"
#include "lwm2m_objects_manager.h"
#include <WiFiUdp.h>

// Constantes CoAP
#define COAP_VERSION 1
#define COAP_TYPE_CON 0
#define COAP_TYPE_NON 1
#define COAP_TYPE_ACK 2
#define COAP_TYPE_RST 3

#define COAP_CODE_GET 1
#define COAP_CODE_POST 2
#define COAP_CODE_PUT 3
#define COAP_CODE_DELETE 4

#define COAP_CODE_CREATED 65  // 2.01
#define COAP_CODE_DELETED 66  // 2.02
#define COAP_CODE_VALID 67    // 2.03
#define COAP_CODE_CHANGED 68  // 2.04
#define COAP_CODE_CONTENT 69  // 2.05

#define COAP_OPTION_URI_PATH 11
#define COAP_OPTION_CONTENT_FORMAT 12
#define COAP_OPTION_URI_QUERY 15

// Variáveis globais
static WiFiUDP udp;
static lwm2m_state_t g_state = LWM2M_STATE_IDLE;
static IPAddress serverIP;
static uint32_t lastRegisterTime = 0;
static uint32_t lastUpdateTime = 0;
static uint16_t messageId = 1;
static String registrationLocation = "";
static uint8_t token[8];
static uint8_t tokenLen = 4;

// Gerar token aleatório
void generateToken() {
    for (int i = 0; i < tokenLen; i++) {
        token[i] = random(256);
    }
}

// Enviar registro LWM2M - Versão simplificada
bool sendLWM2MRegister() {
    uint8_t buffer[512];
    size_t idx = 0;
    
    Serial.println("\n📡 === ENVIANDO REGISTRO LWM2M (POST) ===");
    
    // Gerar token
    generateToken();
    
    // Cabeçalho CoAP
    buffer[idx++] = (COAP_VERSION << 6) | (COAP_TYPE_CON << 4) | tokenLen;
    buffer[idx++] = COAP_CODE_POST;
    messageId++;
    buffer[idx++] = (messageId >> 8) & 0xFF;
    buffer[idx++] = messageId & 0xFF;
    
    // Token
    for (int i = 0; i < tokenLen; i++) {
        buffer[idx++] = token[i];
    }
    
    // Opções em ordem crescente de número
    
    // Option 11: Uri-Path = "rd"
    buffer[idx++] = 0xB2;  // (11 << 4) | 2
    buffer[idx++] = 'r';
    buffer[idx++] = 'd';
    
    // Option 15: Uri-Query = "ep=ESP32-Aluno-001"
    String ep = "ep=" + String(LWM2M_ENDPOINT_NAME);
    buffer[idx++] = 0x4D;  // ((15-11) << 4) | 13 (length >= 13)
    buffer[idx++] = ep.length() - 13;  // Extended length
    memcpy(buffer + idx, ep.c_str(), ep.length());
    idx += ep.length();
    
    // Option 15: Uri-Query = "lt=300"
    String lt = "lt=" + String(LWM2M_LIFETIME);
    buffer[idx++] = 0x06;  // (0 << 4) | 6
    memcpy(buffer + idx, lt.c_str(), lt.length());
    idx += lt.length();
    
    // Option 15: Uri-Query = "lwm2m=1.0"
    String ver = "lwm2m=1.0";
    buffer[idx++] = 0x09;  // (0 << 4) | 9
    memcpy(buffer + idx, ver.c_str(), ver.length());
    idx += ver.length();
    
    // Option 15: Uri-Query = "b=U"
    buffer[idx++] = 0x03;  // (0 << 4) | 3
    buffer[idx++] = 'b';
    buffer[idx++] = '=';
    buffer[idx++] = 'U';
    
    // Payload marker
    buffer[idx++] = 0xFF;
    
    // Payload - usar flag para incluir ou não os sensores
    String payload = "</0/0>,</1/0>,</3/0>,</3311/0>,</3311/1>,</3311/2>";
    
    #if ENABLE_TEMPERATURE_HUMIDITY
    // Adicionar objetos de temperatura e umidade
    payload += ",</3303/0>,</3303/1>,</3304/0>,</3304/1>";
    #endif
    
    memcpy(buffer + idx, payload.c_str(), payload.length());
    idx += payload.length();
    
    // Debug
    Serial.printf("📤 Enviando %d bytes para %s:5683\n", idx, serverIP.toString().c_str());
    Serial.printf("🆔 Message ID: %d\n", messageId);
    Serial.printf("🔑 Token: ");
    for (int i = 0; i < tokenLen; i++) {
        Serial.printf("%02X", token[i]);
    }
    Serial.println();
    Serial.printf("📋 Endpoint: %s\n", LWM2M_ENDPOINT_NAME);
    Serial.printf("📄 Payload: %s\n", payload.c_str());
    
    // Hex dump
    Serial.println("🔍 Hex dump:");
    for (size_t i = 0; i < idx; i++) {
        if (i % 16 == 0) Serial.printf("\n  %04X: ", i);
        Serial.printf("%02X ", buffer[i]);
    }
    Serial.println("\n");
    
    // Enviar
    udp.beginPacket(serverIP, 5683);
    udp.write(buffer, idx);
    udp.endPacket();
    
    return true;
}

// Processar resposta CoAP do buffer
void processCoAPResponseBuffer(uint8_t* buffer, int len) {
    if (len < 4) return;
    
    // Parse cabeçalho
    uint8_t version = (buffer[0] >> 6) & 0x03;
    uint8_t type = (buffer[0] >> 4) & 0x03;
    uint8_t tkl = buffer[0] & 0x0F;
    uint8_t code = buffer[1];
    uint16_t msgId = (buffer[2] << 8) | buffer[3];
    
    // Verificar se é uma resposta para nosso registro
    bool isOurResponse = false;
    
    // Comparar token se for ACK
    if (tkl == tokenLen) {
        bool tokenMatch = true;
        for (int i = 0; i < tkl; i++) {
            if (buffer[4 + i] != token[i]) {
                tokenMatch = false;
                break;
            }
        }
        if (tokenMatch) isOurResponse = true;
    }
    
    // Se não for nossa resposta, ignorar
    if (!isOurResponse) return;
    
    Serial.println("\n📥 === RESPOSTA DO SERVIDOR ===");
    Serial.printf("📊 Código: %d.%02d", (code >> 5), (code & 0x1F));
    
    switch (code) {
        case COAP_CODE_CREATED: {
            Serial.println(" (Created - SUCESSO!)");
            Serial.println("🎉 REGISTRO APROVADO PELO LESHAN!");
            Serial.println("📱 Acesse: https://leshan.eclipseprojects.io");
            Serial.printf("🔍 Procure por: %s\n", LWM2M_ENDPOINT_NAME);
            
            // Extrair Location-Path da resposta
            size_t idx = 4 + tkl;
            uint8_t lastOption = 0;
            registrationLocation = "";
            
            while (idx < len && buffer[idx] != 0xFF) {
                uint8_t delta = (buffer[idx] >> 4) & 0x0F;
                uint8_t optLen = buffer[idx] & 0x0F;
                
                // Verificar extended length
                if (optLen == 13) {
                    idx++;
                    optLen = buffer[idx] + 13;
                } else if (optLen == 14) {
                    idx++;
                    optLen = (buffer[idx] << 8) | buffer[idx + 1] + 269;
                    idx++;
                }
                
                idx++;
                
                uint8_t optNum = lastOption + delta;
                lastOption = optNum;
                
                if (optNum == 8) {  // Location-Path
                    char location[optLen + 1];
                    memcpy(location, buffer + idx, optLen);
                    location[optLen] = 0;
                    
                    if (registrationLocation.length() > 0) {
                        registrationLocation += "/";
                    }
                    registrationLocation += location;
                    
                    Serial.printf("📍 Location: %s\n", location);
                }
                
                idx += optLen;
            }
            
            g_state = LWM2M_STATE_REGISTERED;
            break;
        }
            
        case COAP_CODE_CHANGED: {
            Serial.println(" (Changed - Update aceito)");
            break;
        }
            
        default: {
            Serial.printf(" (Código: 0x%02X)\n", code);
            if (code >= 0x80) {
                Serial.println("❌ Erro no servidor");
                g_state = LWM2M_STATE_ERROR;
            }
            break;
        }
    }
    
    Serial.println("==============================");
}

// Processar resposta CoAP original (mantida por compatibilidade)
void processCoAPResponse() {
    int packetSize = udp.parsePacket();
    if (packetSize == 0) return;
    
    uint8_t buffer[512];
    int len = udp.read(buffer, sizeof(buffer));
    
    processCoAPResponseBuffer(buffer, len);
}

// Enviar resposta CoAP
void sendCoAPResponse(IPAddress ip, int port, uint8_t type, uint8_t code, 
                      uint16_t msgId, uint8_t* token, uint8_t tokenLen,
                      const String& payload = "") {
    uint8_t buffer[512];
    size_t idx = 0;
    
    // Cabeçalho CoAP
    buffer[idx++] = (COAP_VERSION << 6) | (type << 4) | tokenLen;
    buffer[idx++] = code;
    buffer[idx++] = (msgId >> 8) & 0xFF;
    buffer[idx++] = msgId & 0xFF;
    
    // Token
    if (tokenLen > 0) {
        memcpy(buffer + idx, token, tokenLen);
        idx += tokenLen;
    }
    
    // Content-Format para respostas com payload
    if (payload.length() > 0) {
        // Option 12: Content-Format = text/plain (0)
        buffer[idx++] = 0xC1; // (12 << 4) | 1
        buffer[idx++] = 0;    // text/plain
    }
    
    // Payload marker e payload
    if (payload.length() > 0) {
        buffer[idx++] = 0xFF;
        memcpy(buffer + idx, payload.c_str(), payload.length());
        idx += payload.length();
    }
    
    // Enviar resposta
    udp.beginPacket(ip, port);
    udp.write(buffer, idx);
    udp.endPacket();
    
    Serial.printf("📤 Resposta enviada: %d.%02d (%d bytes)\n", 
                  (code >> 5), (code & 0x1F), idx);
}

// Processar requisição CoAP do buffer
void processCoAPRequestBuffer(uint8_t* buffer, int len, IPAddress remoteIP, int remotePort) {
    if (len < 4) return;
    
    // Parse cabeçalho
    uint8_t version = (buffer[0] >> 6) & 0x03;
    uint8_t type = (buffer[0] >> 4) & 0x03;
    uint8_t tkl = buffer[0] & 0x0F;
    uint8_t code = buffer[1];
    uint16_t msgId = (buffer[2] << 8) | buffer[3];
    
    // Extrair token
    uint8_t reqToken[8];
    if (tkl > 0 && tkl <= 8) {
        memcpy(reqToken, buffer + 4, tkl);
    }
    
    // Parse das opções e URI
    size_t idx = 4 + tkl;
    String uriPath = "";
    uint8_t lastOption = 0;
    
    while (idx < len && buffer[idx] != 0xFF) {
        uint8_t delta = (buffer[idx] >> 4) & 0x0F;
        uint8_t optLen = buffer[idx] & 0x0F;
        
        // Extended length
        if (optLen == 13) {
            idx++;
            optLen = buffer[idx] + 13;
        } else if (optLen == 14) {
            idx++;
            optLen = (buffer[idx] << 8) | buffer[idx + 1] + 269;
            idx++;
        }
        
        idx++;
        
        uint8_t optNum = lastOption + delta;
        lastOption = optNum;
        
        // Uri-Path (option 11)
        if (optNum == 11) {
            if (uriPath.length() > 0) uriPath += "/";
            char segment[optLen + 1];
            memcpy(segment, buffer + idx, optLen);
            segment[optLen] = 0;
            uriPath += segment;
        }
        
        idx += optLen;
    }
    
    // Pular payload marker se existir
    if (idx < len && buffer[idx] == 0xFF) {
        idx++;
    }
    
    // Extrair payload se existir
    String payload = "";
    if (idx < len) {
        payload = String((char*)buffer + idx, len - idx);
    }
    
    Serial.println("\n📨 === REQUISIÇÃO DO LESHAN ===");
    Serial.printf("📍 De: %s:%d\n", remoteIP.toString().c_str(), remotePort);
    Serial.printf("🔍 Método: %d.%02d ", (code >> 5), (code & 0x1F));
    
    switch (code) {
        case COAP_CODE_GET: Serial.println("(GET)"); break;
        case COAP_CODE_PUT: Serial.println("(PUT)"); break;
        case COAP_CODE_POST: Serial.println("(POST)"); break;
        case COAP_CODE_DELETE: Serial.println("(DELETE)"); break;
        default: Serial.println("(?)"); break;
    }
    
    Serial.printf("📂 URI: /%s\n", uriPath.c_str());
    if (payload.length() > 0) {
        Serial.printf("📄 Payload (%d bytes): ", payload.length());
        // Mostrar payload em hex e ASCII
        for (int i = 0; i < payload.length(); i++) {
            Serial.printf("%02X ", (uint8_t)payload[i]);
        }
        Serial.print(" = '");
        Serial.print(payload);
        Serial.println("'");
    }
    
    // Processar a requisição baseado no URI
    String responsePayload = "";
    uint8_t responseCode = 0x84; // 4.04 Not Found por padrão
    
    // Parse do URI path para extrair object/instance/resource
    // O Leshan envia direto como "3311/0/5850"
    int firstSlash = uriPath.indexOf('/');
    int secondSlash = -1;
    
    if (firstSlash > 0) {
        secondSlash = uriPath.indexOf('/', firstSlash + 1);
    }
    
    if (firstSlash > 0 && secondSlash > firstSlash) {
        String objectStr = uriPath.substring(0, firstSlash);
        String instanceStr = uriPath.substring(firstSlash + 1, secondSlash);
        String resourceStr = uriPath.substring(secondSlash + 1);
        
        uint16_t objectId = objectStr.toInt();
        uint16_t instanceId = instanceStr.toInt();
        uint16_t resourceId = resourceStr.toInt();
        
        Serial.printf("🎯 Object: %d, Instance: %d, Resource: %d\n", 
                     objectId, instanceId, resourceId);
        
        // Processar baseado no método
        if (code == COAP_CODE_GET) {
            // Operação READ
            if (objectsManager.processRead(objectId, instanceId, resourceId, responsePayload)) {
                responseCode = COAP_CODE_CONTENT; // 2.05 Content
                Serial.printf("✅ Read OK: %s\n", responsePayload.c_str());
            } else {
                responseCode = 0x84; // 4.04 Not Found
                Serial.println("❌ Recurso não encontrado");
            }
        } else if (code == COAP_CODE_PUT) {
            // Operação WRITE
            if (objectsManager.processWrite(objectId, instanceId, resourceId, payload)) {
                responseCode = COAP_CODE_CHANGED; // 2.04 Changed
                Serial.println("✅ Write OK");
            } else {
                responseCode = 0x85; // 4.05 Method Not Allowed
                Serial.println("❌ Write não permitido");
            }
        } else if (code == COAP_CODE_POST) {
            // Operação EXECUTE
            if (objectsManager.processExecute(objectId, instanceId, resourceId)) {
                responseCode = COAP_CODE_CHANGED; // 2.04 Changed
                Serial.println("✅ Execute OK");
            } else {
                responseCode = 0x85; // 4.05 Method Not Allowed
                Serial.println("❌ Execute não permitido");
            }
        }
    } else {
        Serial.printf("❌ URI inválido: %s\n", uriPath.c_str());
        responseCode = 0x80; // 4.00 Bad Request
    }
    
    // Enviar resposta
    uint8_t responseType = (type == COAP_TYPE_CON) ? COAP_TYPE_ACK : COAP_TYPE_NON;
    sendCoAPResponse(remoteIP, remotePort, responseType, responseCode, 
                     msgId, reqToken, tkl, responsePayload);
    
    Serial.println("==============================");
}

// Processar requisição CoAP original (mantida por compatibilidade)
void processCoAPRequest() {
    int packetSize = udp.parsePacket();
    if (packetSize == 0) return;
    
    uint8_t buffer[512];
    IPAddress remoteIP = udp.remoteIP();
    int remotePort = udp.remotePort();
    int len = udp.read(buffer, sizeof(buffer));
    
    processCoAPRequestBuffer(buffer, len, remoteIP, remotePort);
}

// Enviar update
bool sendLWM2MUpdate() {
    if (registrationLocation.isEmpty()) return false;
    
    uint8_t buffer[256];
    size_t idx = 0;
    
    Serial.println("\n💓 Enviando update de registro...");
    
    // Cabeçalho CoAP PUT
    buffer[idx++] = (COAP_VERSION << 6) | (COAP_TYPE_CON << 4) | tokenLen;
    buffer[idx++] = COAP_CODE_PUT;
    messageId++;
    buffer[idx++] = (messageId >> 8) & 0xFF;
    buffer[idx++] = messageId & 0xFF;
    
    // Token
    for (int i = 0; i < tokenLen; i++) {
        buffer[idx++] = token[i];
    }
    
    // Parsear o location path
    String location = registrationLocation;
    
    // Option 11: Uri-Path (cada segmento)
    int start = 0;
    int end = location.indexOf('/', start);
    uint8_t lastOpt = 0;
    
    while (start < location.length()) {
        if (end == -1) end = location.length();
        
        String segment = location.substring(start, end);
        if (segment.length() > 0) {
            uint8_t delta = (lastOpt == 0) ? 11 : 0;
            buffer[idx++] = (delta << 4) | (segment.length() & 0x0F);
            memcpy(buffer + idx, segment.c_str(), segment.length());
            idx += segment.length();
            lastOpt = 11;
        }
        
        start = end + 1;
        end = location.indexOf('/', start);
    }
    
    // Enviar
    udp.beginPacket(serverIP, 5683);
    udp.write(buffer, idx);
    udp.endPacket();
    
    Serial.printf("📤 Update enviado para %s\n", registrationLocation.c_str());
    
    return true;
}

// Funções públicas
bool lwm2m_coap_init() {
    Serial.println("🔧 Inicializando cliente LWM2M/CoAP...");
    
    // Resolver servidor
    if (WiFi.hostByName("leshan.eclipseprojects.io", serverIP)) {
        Serial.printf("✅ Servidor resolvido: %s\n", serverIP.toString().c_str());
    } else {
        Serial.println("❌ Falha ao resolver servidor DNS");
        return false;
    }
    
    // Inicializar UDP
    if (!udp.begin(5683)) {
        Serial.println("❌ Falha ao inicializar UDP");
        return false;
    }
    
    Serial.println("✅ Cliente CoAP inicializado");
    g_state = LWM2M_STATE_IDLE;
    
    return true;
}

void lwm2m_coap_step() {
    uint32_t currentTime = millis();
    
    // Processar pacotes recebidos
    int packetSize = udp.parsePacket();
    if (packetSize > 0) {
        uint8_t buffer[512];
        IPAddress remoteIP = udp.remoteIP();
        int remotePort = udp.remotePort();
        int len = udp.read(buffer, sizeof(buffer));
        
        if (len >= 4) {
            uint8_t type = (buffer[0] >> 4) & 0x03;
            uint8_t code = buffer[1];
            
            // Se for requisição (GET, PUT, POST, DELETE)
            if (code >= 1 && code <= 4) {
                // Reprocessar o buffer como requisição
                processCoAPRequestBuffer(buffer, len, remoteIP, remotePort);
            } 
            // Se for resposta ou ACK
            else {
                // Reprocessar o buffer como resposta
                processCoAPResponseBuffer(buffer, len);
            }
        }
    }
    
    switch (g_state) {
        case LWM2M_STATE_IDLE:
            if (currentTime - lastRegisterTime > 5000) {
                if (sendLWM2MRegister()) {
                    g_state = LWM2M_STATE_REGISTERING;
                    lastRegisterTime = currentTime;
                } else {
                    g_state = LWM2M_STATE_ERROR;
                }
            }
            break;
            
        case LWM2M_STATE_REGISTERING:
            // Timeout
            if (currentTime - lastRegisterTime > 30000) {
                Serial.println("⏰ Timeout no registro - tentando novamente...");
                g_state = LWM2M_STATE_IDLE;
            }
            break;
            
        case LWM2M_STATE_REGISTERED:
            // Update periódico (metade do lifetime)
            if (currentTime - lastUpdateTime > (LWM2M_LIFETIME * 500)) {
                if (sendLWM2MUpdate()) {
                    lastUpdateTime = currentTime;
                }
            }
            break;
            
        case LWM2M_STATE_ERROR:
            if (currentTime - lastRegisterTime > 30000) {
                Serial.println("🔄 Tentando novamente após erro...");
                g_state = LWM2M_STATE_IDLE;
            }
            break;
    }
}

bool lwm2m_coap_is_registered() {
    return (g_state == LWM2M_STATE_REGISTERED);
}

String lwm2m_coap_get_status() {
    switch (g_state) {
        case LWM2M_STATE_IDLE: return "Aguardando";
        case LWM2M_STATE_REGISTERING: return "Registrando";
        case LWM2M_STATE_REGISTERED: return "Registrado";
        case LWM2M_STATE_ERROR: return "Erro";
        default: return "Desconhecido";
    }
}