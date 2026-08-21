/* Aplicacao 06 - PUBLICADOR MQTT

   O ESP32 mede a distancia e publica o valor no broker.
   Fluxo da aula: SENSOR -> PUBLISH -> BROKER
*/

#include <WiFi.h>
#include <PubSubClient.h>

// Altere somente estes dados para a rede da sala.
const char* WIFI_SSID = "NorisIoT";
const char* WIFI_SENHA = "Secure10T";
const char* BROKER_IP = "172.16.10.101";

const char* TOPICO = "fiap/iot/distancia";

const int PINO_TRIGGER = 22;
const int PINO_ECHO = 23;
const float DISTANCIA_MAXIMA = 400.0;

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

void conectarMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Conectando ao broker MQTT...");

    if (mqtt.connect("esp32-professor")) {
      Serial.println(" conectado!");
    } else {
      Serial.println(" falhou. Nova tentativa em 2 segundos.");
      delay(2000);
    }
  }
}

float medirDistancia() {
  digitalWrite(PINO_TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(PINO_TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(PINO_TRIGGER, LOW);

  unsigned long tempoDoEco = pulseIn(PINO_ECHO, HIGH, 23200);  // no maximo 400 cm

  // Sem eco, pulseIn devolve 0. Para a aula, isso significa "muito longe".
  if (tempoDoEco == 0) {
    return DISTANCIA_MAXIMA;
  }

  return tempoDoEco / 58.0;  // converte microssegundos para centimetros
}

void setup() {
  Serial.begin(115200);
  pinMode(PINO_TRIGGER, OUTPUT);
  pinMode(PINO_ECHO, INPUT);

  conectarWiFi();
  mqtt.setServer(BROKER_IP, 1883);
}

void loop() {
  if (!mqtt.connected()) {
    conectarMQTT();
  }
  mqtt.loop();

  float distancia = medirDistancia();
  String mensagem = String(distancia, 0);

  mqtt.publish(TOPICO, mensagem.c_str());
  Serial.println("Distancia publicada: " + mensagem + " cm");

  delay(500);
}
