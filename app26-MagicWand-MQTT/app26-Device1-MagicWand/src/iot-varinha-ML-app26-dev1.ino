#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "ESP32Sensors.hpp"
#include <MagicWand_ESP32_inferencing.h>

/* ---- Configurações ---- */
//WiFi
const char* WIFI_SSID     = "Wokwi-GUEST";   // Rede pública do simulador
const char* WIFI_PASSWORD = "";
WiFiClient wifiClient;

//Sensores
const static uint8_t SCL_PIN   = 23;
const static uint8_t SDA_PIN   = 22;
const static uint8_t LED_R_PIN = 21;
const static uint8_t LED_B_PIN = 18;
const static uint8_t LED_G_PIN = 19;

//EdgeAI
#define FREQUENCY_HZ        50
#define INTERVAL_MS         (1000 / (FREQUENCY_HZ))

static unsigned long last_interval_ms = 0;

// Buffer para inferência (6 eixos * número de amostras)
static float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
static size_t feature_ix = 0;

//Variável que armazena o resultado da inferência (tempo, probabilidades de cada classe)
ei_impulse_result_t result;

//Variáveis que armazenam as probabilidades e classe da predição do gesto
float best_val = 0.0f;
size_t best_ix = 0;

//MQTT
#define MQTT_HOST       "host.wokwi.internal"
#define MQTT_PORT       1883
#define MQTT_PUB_TOPIC  "FIAPIoT/smartagro/cmd/local"
#define MQTT_DEVICEID   "FIAPIoTapp26Dev1"
PubSubClient mqttClient(wifiClient);
bool wifiConectado = false;
bool mqttConectado = false;
unsigned long ultimaTentativaReconexao = 0;
const unsigned long INTERVALO_RECONEXAO = 5000; // 5 segundos entre tentativas

/* ---- Setup ---- */
void setup() {
    Serial.begin(115200);
    while (!Serial) {;}
    
    Serial.println("\n=== App26: ESP32 + MPU6050 + Edge Impulse + MQTT ===");

    //Conecta no WiFi
    conectarWiFi();
    
    //Configura MQTT e conecta no Broker
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setKeepAlive(60);     // Manter conexão viva por 60s
    mqttClient.setSocketTimeout(60); // Timeout de 60s
    mqttClient.setBufferSize(512);   // Buffer adequado considerando o payload gerado
    conectarMQTT();

    // Configurar LEDs
    pinMode(LED_R_PIN, OUTPUT);
    pinMode(LED_B_PIN, OUTPUT);
    pinMode(LED_G_PIN, OUTPUT);
    
    // Teste dos LEDs
    for (int i = 0; i < 2; i++) {
        digitalWrite(LED_R_PIN, HIGH);
        digitalWrite(LED_B_PIN, HIGH);
        digitalWrite(LED_G_PIN, HIGH);
        delay(500);
        leds_off();
        delay(500);
    }
    
    // Inicializar sensores
    ESP32Sensors::beginAll(SDA_PIN, SCL_PIN);
    
    // Verificar configuração do modelo
    if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != 6) {
        Serial.println("ERRO: Modelo deve ter 6 eixos (ax,ay,az,gx,gy,gz)");
        while (1) {;}
    }
    
    Serial.println("\nPronto para detectar gestos!");
    Serial.print("Tamanho do buffer: ");
    Serial.println(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
}

/* ---- Loop principal ---- */
void loop() {
    mqttClient.loop();

    if (millis() > last_interval_ms + INTERVAL_MS) {
        last_interval_ms = millis();

        // Ler sensor
        ESP32Sensors::AccelGyro::DADOS dados = ESP32Sensors::AccelGyro::medirAccelGyro();

        // Preencher buffer (ax, ay, az, gx, gy, gz)
        if (feature_ix + 6 <= EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
            features[feature_ix++] = dados.accel.acceleration.x;
            features[feature_ix++] = dados.accel.acceleration.y;
            features[feature_ix++] = dados.accel.acceleration.z;
            features[feature_ix++] = dados.gyro.gyro.x;
            features[feature_ix++] = dados.gyro.gyro.y;
            features[feature_ix++] = dados.gyro.gyro.z;
        }

        // Quando o buffer estiver cheio, fazer inferência
        if (feature_ix == EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {

            // Realiza a Transformada de Fourier considerando o vetor features[]
            // Preenche a variável "result"
            EdgeAI_fft();

            // Faz a previsão, retornando a melhor classe e probabilidade
            EdgeAI_predict();

            // Identifica o gesto com base na predição
            const char *gesto = result.classification[best_ix].label;
            ei_printf("=> Gesto detectado: %s (%.2f%%)\n", gesto, best_val * 100);

            // Acionar LED correspondente E enviar comando MQTT (threshold de 60%)
            if (best_val < 0.6f) {
                leds_off();
            } else {
                if (strcmp(gesto, "cima-baixo") == 0) {
                    led_cima_baixo();
                    enviarComando("3342", false);
                } else if (strcmp(gesto, "circulo") == 0) {
                    led_circulo();
                    enviarComando("3342", true);
                } else if (strcmp(gesto, "lateral-parabaixo") == 0) {
                    led_lateral_parabaixo();
                    enviarComando("3338", true);
                } else if (strcmp(gesto, "descanso-parabaixo") == 0) {
                    led_descanso_parabaixo();
                    enviarComando("3338", false);
                } else {
                    leds_off();
                }
            }

            // Resetar buffer para próxima janela
            feature_ix = 0;
        }
    }
}


/* ---- Funções ---- */
void conectarWiFi() {
  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  uint8_t tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 30) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("OK! IP: ");
    Serial.println(WiFi.localIP());
    wifiConectado = true;
  } else {
    Serial.println("Falha ao conectar Wi-Fi.");
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

void enviarComando(const char* atuador, bool comando) {
    if (!mqttConectado) {
        conectarMQTT();
        if (!mqttConectado) return;
    }

    JsonDocument doc;
    doc["deviceId"] = MQTT_DEVICEID;
    doc["atuador"] = atuador;
    doc["comando"] = comando;

    char payload[200];
    serializeJson(doc, payload);

    if (mqttClient.publish(MQTT_PUB_TOPIC, payload)) {
        Serial.printf("[MQTT] Comando enviado -> Atuador: %s, Comando: %s\n",
                      atuador, comando ? "true" : "false");
    } else {
        Serial.println("[MQTT] Falha ao enviar comando");
        mqttConectado = false;
    }
}

void leds_off() {
    digitalWrite(LED_R_PIN, LOW);
    digitalWrite(LED_G_PIN, LOW);
    digitalWrite(LED_B_PIN, LOW);
}

void led_cima_baixo() {
    digitalWrite(LED_R_PIN, HIGH);
    digitalWrite(LED_G_PIN, LOW);
    digitalWrite(LED_B_PIN, LOW);
}

void led_circulo() {
    digitalWrite(LED_R_PIN, LOW);
    digitalWrite(LED_G_PIN, HIGH);
    digitalWrite(LED_B_PIN, LOW);
}

void led_lateral_parabaixo() {
    digitalWrite(LED_R_PIN, LOW);
    digitalWrite(LED_G_PIN, LOW);
    digitalWrite(LED_B_PIN, HIGH);
}

void led_descanso_parabaixo() {
    digitalWrite(LED_R_PIN, HIGH);
    digitalWrite(LED_G_PIN, HIGH);
    digitalWrite(LED_B_PIN, HIGH);
}

/* ---- Função de impressão do Edge Impulse ---- */
void ei_printf(const char *format, ...) {
    static char print_buf[256];
    va_list args;
    va_start(args, format);
    int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
    va_end(args);
    if (r > 0) {
        Serial.write(print_buf);
    }
}

void EdgeAI_fft() {
    // Criar sinal para o classificador
    signal_t signal;
    numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);

    // Rodar inferência
    EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
    
    if (res != EI_IMPULSE_OK) {
        ei_printf("Erro na inferência: %d\n", res);
        feature_ix = 0;
        return;
    }

}

void EdgeAI_predict() {
    // Mostrar tempos
    ei_printf("\nTiming -> DSP: %d ms, Class: %d ms\n",
                result.timing.dsp,
                result.timing.classification);

    // Encontrar maior probabilidade
    
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        float v = result.classification[ix].value;
        const char *lab = result.classification[ix].label;
        ei_printf("  %s: %.3f\n", lab, v);
        
        if (v > best_val) {
            best_val = v;
            best_ix = ix;
        }
    }
}
