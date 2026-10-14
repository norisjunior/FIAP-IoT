/* Aplicacao 07 - ASSINANTE MQTT

   O ESP32 recebe a distancia e acende o LED quando alguem esta perto.
   Fluxo da aula: BROKER -> SUBSCRIBE -> LED
*/

#include <WiFi.h>
#include <PubSubClient.h>

// Altere somente estes dados para a rede da sala.
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_SENHA = "";
// const char* WIFI_SSID = "NorisIoT";
// const char* WIFI_SENHA = "Secure10T";
// Wokwi: broker local; no kit, use "172.16.10.101".
const char* MQTT_BROKER = "host.wokwi.internal";
const uint16_t MQTT_PORTA = 1883;

const char* TOPICO = "fiap/iot/distancia";

const int PINO_LED = 21;
const int PINO_BUZ = 19;
const int LIMITE_VISUAL = 50;
const int LIMITE_SONORO = 20;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Troque o RA de exemplo; cada aluno/dispositivo precisa de um ID distinto.
const char* MQTT_CLIENT_ID = "a12345678-app08-01";

// Controle da conexão
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

void receberMensagem(char* topico, byte* conteudo, unsigned int tamanho) {
  String mensagem(conteudo, tamanho);
  float distancia = mensagem.toFloat();

  Serial.println("Distancia recebida: " + mensagem + " cm");

  if (distancia < LIMITE_VISUAL) {
    digitalWrite(PINO_LED, HIGH);
  } else {
    digitalWrite(PINO_LED, LOW);
  }

  if (distancia < LIMITE_SONORO) {
    tone(PINO_BUZ, 1000);
  } else {
    noTone(PINO_BUZ);
  }

}

void conectarMQTT() {
  if (millis() - ultimaTentativaMQTT < INTERVALO_RECONEXAO) {
    return;
  }
  ultimaTentativaMQTT = millis();

  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println("[MQTT] Conectado");
    mqttClient.subscribe(TOPICO);
  } else {
    Serial.printf("[MQTT] Falha: %d\r\n", mqttClient.state());
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PINO_LED, OUTPUT);
  pinMode(PINO_BUZ, OUTPUT);

  digitalWrite(PINO_LED, LOW);
  tone(PINO_BUZ, 1000);
  noTone(PINO_BUZ);

  conectarWiFi();
  mqttClient.setServer(MQTT_BROKER, MQTT_PORTA);
  mqttClient.setKeepAlive(120);
  mqttClient.setSocketTimeout(2); // Espera pela resposta MQTT, em segundos
  mqttClient.setCallback(receberMensagem);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  } else if (!mqttClient.connected()) {
    conectarMQTT();
  } else {
    mqttClient.loop();
  }
  delay(100);
}
