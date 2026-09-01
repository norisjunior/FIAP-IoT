/* Aplicacao 07 - ASSINANTE MQTT

   O ESP32 recebe a distancia e acende o LED quando alguem esta perto.
   Fluxo da aula: BROKER -> SUBSCRIBE -> LED
*/

#include <WiFi.h>
#include <PubSubClient.h>

// Altere somente estes dados para a rede da sala.
// const char* WIFI_SSID = "Wokwi-GUEST";
// const char* WIFI_SENHA = "";
const char* WIFI_SSID = "NorisIoT";
const char* WIFI_SENHA = "Secure10T";
const char* BROKER_IP = "172.16.10.101";

const char* TOPICO = "fiap/iot/distancia";

const int PINO_LED = 21;
const int PINO_BUZ = 19;
const int LIMITE_VISUAL = 50;
const int LIMITE_SONORO = 20;

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

const char* CLIENT_ID = "aluno1";

void conectarWiFi() {
  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_SENHA);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println(" conectado!");
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
  // O endereco MAC deixa o nome de cada ESP32 unico na sala.
  while (!mqtt.connected()) {
    Serial.print("Conectando ao broker MQTT...");

    if (mqtt.connect(CLIENT_ID)) {
      Serial.println(" conectado!");
      mqtt.subscribe(TOPICO);
    } else {
      Serial.println(" falhou. Nova tentativa em 2 segundos.");
    }
  delay(2000);
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
  mqtt.setServer(BROKER_IP, 1883);
  mqtt.setKeepAlive(120);
  mqtt.setCallback(receberMensagem);
}

void loop() {
  if (!mqtt.connected()) {
    conectarMQTT();
  }

  mqtt.loop();  // recebe as mensagens do broker
  delay(100);
}
