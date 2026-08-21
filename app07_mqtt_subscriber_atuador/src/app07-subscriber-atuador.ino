/* Aplicacao 07 - ASSINANTE MQTT

   O ESP32 recebe a distancia e acende o LED quando alguem esta perto.
   Fluxo da aula: BROKER -> SUBSCRIBE -> LED
*/

#include <WiFi.h>
#include <PubSubClient.h>

// Altere somente estes dados para a rede da sala.
const char* WIFI_SSID = "NorisIoT";
const char* WIFI_SENHA = "Secure10T";
const char* BROKER_IP = "172.16.10.101";

const char* TOPICO = "fiap/iot/distancia";

const int PINO_LED = 21;
const int LIMITE = 100;  // LED acende a 100 cm ou menos

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

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
  digitalWrite(PINO_LED, distancia <= LIMITE);
}

void conectarMQTT() {
  // O endereco MAC deixa o nome de cada ESP32 unico na sala.
  String nomeDoESP32 = "aluno-" + WiFi.macAddress();

  while (!mqtt.connected()) {
    Serial.print("Conectando ao broker MQTT...");

    if (mqtt.connect(nomeDoESP32.c_str())) {
      Serial.println(" conectado!");
      mqtt.subscribe(TOPICO);
    } else {
      Serial.println(" falhou. Nova tentativa em 2 segundos.");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PINO_LED, OUTPUT);

  conectarWiFi();
  mqtt.setServer(BROKER_IP, 1883);
  mqtt.setCallback(receberMensagem);
}

void loop() {
  if (!mqtt.connected()) {
    conectarMQTT();
  }

  mqtt.loop();  // recebe as mensagens do broker
}
