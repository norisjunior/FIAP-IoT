/* Aplicacao 06 - PUBLICADOR MQTT

   O ESP32 mede a distancia e publica o valor no broker.
   Fluxo da aula: SENSOR -> PUBLISH -> BROKER
*/
#include "ESP32SensorsDistancia.hpp"
#include <WiFi.h>
#include <PubSubClient.h>

// Wokwi: rede aberta. No kit físico, informe o SSID e a senha da rede.
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_SENHA = "";

// Broker no seu computador pelo Wokwi; no laboratório, use "172.16.10.101".
const char* MQTT_BROKER = "host.wokwi.internal";
const uint16_t MQTT_PORTA = 1883;
// Cada aluno troca o RA; cada dispositivo usa um sufixo diferente.
const char* MQTT_CLIENT_ID = "a12345678-app06-01";
const char* TOPICO = "fiap/iot/distancia";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

const int TRIG_PIN = 22;
const int ECHO_PIN = 23;
const uint32_t TEMPO_LIMITE_ECHO = 401U * 57; // Limite do exemplo anterior

uint32_t tempoAgora = 0;
uint32_t INTERVALO_COLETA = 500;

// Controle da conexão, como no app14
const uint32_t INTERVALO_RECONEXAO = 5000;
uint32_t ultimaTentativaWiFi = 0, ultimaTentativaMQTT = 0;

void conectarWiFi() {
  // Uma tentativa a cada INTERVALO_RECONEXAO, como no app14.
  if (millis() - ultimaTentativaWiFi < INTERVALO_RECONEXAO) {
    return;
  }
  ultimaTentativaWiFi = millis();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_SENHA);
  WiFi.setSleep(false);

  // Aguarda conectar para mostrar o IP obtido nesta conexão.
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.println("[WiFi] Conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void conectarMQTT() {
  if (millis() - ultimaTentativaMQTT < INTERVALO_RECONEXAO) {
    return;
  }
  ultimaTentativaMQTT = millis();

  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println("[MQTT] Conectado");
  } else {
    Serial.printf("[MQTT] Falha: %d\r\n", mqttClient.state());
  }
}

void setup() {
  Serial.begin(115200);
  ESP32Sensors::Distancia::inicializar(TRIG_PIN, ECHO_PIN, TEMPO_LIMITE_ECHO);

  conectarWiFi();
  mqttClient.setServer(MQTT_BROKER, MQTT_PORTA);
  mqttClient.setKeepAlive(120);
  mqttClient.setSocketTimeout(2); // Limita a espera pela resposta MQTT, em segundos
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  } else if (!mqttClient.connected()) {
    conectarMQTT();
  } else {
    mqttClient.loop();
  }

  if (millis() - tempoAgora >= INTERVALO_COLETA) {
    tempoAgora = millis();
    ESP32Sensors::Distancia::DISTANCIA leitura = ESP32Sensors::Distancia::medirDistancia();

    // Mantém a conversão inteira da NewPing usada neste exemplo (57 us por cm).
    float distancia = leitura.duracaoUs / 57;
    if (!leitura.valido || distancia == 0 || distancia > 400) distancia = 400;
    String mensagem = String(distancia, 0);

    mqttClient.publish(TOPICO, mensagem.c_str());
    Serial.println("Distancia publicada: " + mensagem + " cm");
  }
}
