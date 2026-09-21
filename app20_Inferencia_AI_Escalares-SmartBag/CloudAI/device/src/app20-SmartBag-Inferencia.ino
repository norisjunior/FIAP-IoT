/* app20-SmartBag-Inferencia — a bag mede, pergunta para a nuvem e obedece.

   As seis features sao as MESMAS do app19, com o mesmo calculo e as mesmas
   unidades: temperatura, umidade, delta_distancia, luz, mov_max e incl_max.
   Divergir aqui faz o modelo errar sem avisar.

   O que sumiu em relacao ao app19: os dois BOTOES. La eles existiam porque
   aquilo era um GERADOR DE DATASET, onde uma pessoa rotulava cada amostra.
   Aqui quem rotula e o modelo, entao nao ha o que apertar. O GPIO 27, que era
   o botao COLETA, virou saida: de entrada do rotulo humano para saida do
   rotulo do modelo.

       ENTREGA_OK        LED verde    (27)
       REVISAR_ENTREGA   LED vermelho (21)
       tudo apagado      a nuvem nao respondeu

   O LED 21 e o mesmo pino do alerta do app15. La quem acendia era um limiar
   desenhado no Node-RED; aqui e um modelo treinado. O caminho e o mesmo, a
   decisao e que mudou de natureza.
*/
/*
PARA USAR NO WOKWI:
- Ajustar as credenciais WiFi e o MQTT_SERVER (ou usar as linhas do Wokwi abaixo)
- Feche a bag ANTES de ligar: o baseline da distancia e medido uma vez, no setup()
*/

/* ---- Includes ---- */
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "ESP32SensorsAmbiente.hpp"
#include "ESP32SensorsDistancia.hpp"
#include "ESP32SensorsAccel.hpp"
#include "ESP32SensorsLDR.hpp"

/* ---- Config Hardware ---- */
const uint8_t DHT_PIN = 4;
const uint8_t DHT_MODEL = DHT22;
const uint8_t TRIG_PIN = 19;
const uint8_t ECHO_PIN = 18;
const uint8_t SCL_PIN = 23;
const uint8_t SDA_PIN = 22;
const uint8_t LDR_PIN = 35;
const uint8_t LED_REVISAR = 21;   // era o LED de coleta do app19
const uint8_t LED_OK = 27;        // era o botao COLETA do app19

/* ---- Config Wi-Fi e MQTT ---- */
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
#define MQTT_SERVER "host.wokwi.internal"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "SmartBagEquipe01"
#define MQTT_PUB_TOPIC "FIAPIoT/smartbag/equipe01/dados"
#define MQTT_SUB_TOPIC "FIAPIoT/smartbag/equipe01/cmd"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

/* ---- Controle de intervalo ---- */
const unsigned long INTERVALO_COLETA = 1000;
const unsigned long INTERVALO_MPU = 50;
const unsigned long INTERVALO_DHT = 2500;
const unsigned long INTERVALO_RECONEXAO = 5000;
// Sem resposta por este tempo, as duas saidas apagam. Um LED aceso precisa
// significar "o modelo disse isso agora", nao "o modelo disse isso um dia".
const unsigned long VALIDADE_RESPOSTA = 5000;
unsigned long tempoAnterior = 0, ultimoMPU = 0, ultimoDHT = 0;
unsigned long ultimaTentativaWiFi = 0, ultimaTentativaMQTT = 0;
unsigned long ultimaResposta = 0;
bool temResposta = false;

/* ---- Medicoes ---- */
ESP32Sensors::Ambiente::AMBIENTE ambiente = {NAN, NAN, NAN, false};
float distBase = NAN, movMax = NAN, inclMax = NAN;

/* ---- Prototipos ---- */
void calibrarDistancia();
void conectarWiFi();
void conectarMQTT();
void enviarFeatures();
void receberClasse(char* topico, byte* conteudo, unsigned int tamanho);
void apagarSaidas();

void setup() {
  Serial.begin(115200);
  pinMode(LED_OK, OUTPUT);
  pinMode(LED_REVISAR, OUTPUT);
  apagarSaidas();

  ESP32Sensors::Ambiente::inicializar(DHT_PIN, DHT_MODEL);
  ESP32Sensors::Distancia::inicializar(TRIG_PIN, ECHO_PIN);
  ESP32Sensors::Accel::inicializar(SCL_PIN, SDA_PIN);
  ESP32Sensors::LDR::inicializar(LDR_PIN);

  // Confere a ligacao dos dois LEDs antes de depender do modelo. Se um nao
  // piscar aqui, o problema e o fio, nao a nuvem.
  digitalWrite(LED_OK, HIGH); delay(400); digitalWrite(LED_OK, LOW);
  digitalWrite(LED_REVISAR, HIGH); delay(400); digitalWrite(LED_REVISAR, LOW);

  // Baseline uma vez so, no boot: aqui nao ha botao para recalibrar.
  // A bag precisa estar FECHADA e PARADA quando o ESP32 liga.
  calibrarDistancia();

  conectarWiFi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(receberClasse);
  mqttClient.setBufferSize(1024);
  mqttClient.setKeepAlive(120);

  Serial.println("Feche a bag antes de ligar: o baseline e medido no boot.");
  Serial.printf("  GPIO %2d  LED verde    = ENTREGA_OK\r\n", LED_OK);
  Serial.printf("  GPIO %2d  LED vermelho = REVISAR_ENTREGA\r\n", LED_REVISAR);
  Serial.println("  Tudo apagado = a nuvem ainda nao respondeu");

  // Linha de titulo do CSV. Serial.println ja termina em \r\n .
  Serial.println("temperatura,umidade,delta_distancia,luz,mov_max,incl_max");
}

void loop() {
  // O DHT22 e lento: guardamos a ultima leitura boa e enviamos ela.
  unsigned long agora = millis();
  if (agora - ultimoDHT >= INTERVALO_DHT) {
    ultimoDHT = agora;
    ESP32Sensors::Ambiente::AMBIENTE leitura = ESP32Sensors::Ambiente::medirAmbiente();
    if (leitura.valido) ambiente = leitura;
  }

  // O MPU e rapido: 20 amostras por segundo, guardamos so os maiores.
  agora = millis();
  if (agora - ultimoMPU >= INTERVALO_MPU) {
    ultimoMPU = agora;
    AccelData accel = ESP32Sensors::Accel::medirAccel();
    movMax = fmaxf(movMax, ESP32Sensors::Accel::medirMovimentacao(accel));
    inclMax = fmaxf(inclMax, ESP32Sensors::Accel::medirInclinacao(accel));
  }

  // O HC-SR04 e lido junto do envio: a distancia nao muda em milissegundos.
  if (millis() - tempoAnterior >= INTERVALO_COLETA) {
    enviarFeatures();
    movMax = inclMax = NAN;   // recomeca a procurar os picos
    tempoAnterior = millis();
  }

  // A resposta envelhece. Sem classe nova, apaga tudo.
  if (temResposta && millis() - ultimaResposta >= VALIDADE_RESPOSTA) {
    temResposta = false;
    apagarSaidas();
    Serial.println("[NUVEM] Sem resposta. Saidas apagadas.");
  }

  if (WiFi.status() != WL_CONNECTED) {
    if (millis() - ultimaTentativaWiFi >= INTERVALO_RECONEXAO) {
      ultimaTentativaWiFi = millis();
      WiFi.reconnect();
    }
  } else if (!mqttClient.connected()) {
    if (millis() - ultimaTentativaMQTT >= INTERVALO_RECONEXAO) {
      ultimaTentativaMQTT = millis();
      conectarMQTT();
    }
  } else {
    mqttClient.loop();
  }
}

void calibrarDistancia() {
  float soma = 0;
  int validas = 0;
  unsigned long inicio = millis();
  while (validas < 5 && millis() - inicio < 5000) {
    float dist = ESP32Sensors::Distancia::medirDistancia();
    if (isfinite(dist)) { soma += dist; validas++; }
    delay(60);
  }
  distBase = validas == 5 ? soma / validas : NAN;
  Serial.printf("[BASELINE] %.2f cm\r\n", distBase);
  if (!isfinite(distBase)) {
    Serial.println("[BASELINE] Sem medida. Confira o HC-SR04 e reinicie: sem baseline nao ha inferencia.");
  }
}

void conectarWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setSleep(false);
}

void conectarMQTT() {
  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println("[MQTT] Conectado");
    mqttClient.subscribe(MQTT_SUB_TOPIC);
  } else {
    Serial.printf("[MQTT] Falha: %d\r\n", mqttClient.state());
  }
}

void apagarSaidas() {
  digitalWrite(LED_OK, LOW);
  digitalWrite(LED_REVISAR, LOW);
}

/* ---- Publica as seis features, na ordem do contrato ----
   Nao vai rodada nem situacao: eram identificacao de dataset, e o modelo nunca
   as recebeu no treino. Tambem nao vai timestamp — a amostra vale agora. */
void enviarFeatures() {
  float dist = ESP32Sensors::Distancia::medirDistancia();
  float delta = dist - distBase;
  int luz = ESP32Sensors::LDR::ler();

  Serial.printf("%.1f,%.1f,%.2f,%d,%.2f,%.1f\r\n",
                ambiente.temp, ambiente.umid, delta, luz, movMax, inclMax);

  // Uma feature ausente nao vira zero: o modelo foi treinado com seis numeros
  // reais, e zero e um valor plausivel que ele interpretaria como medida boa.
  // Sem as seis, nao ha pergunta a fazer.
  if (!isfinite(ambiente.temp) || !isfinite(ambiente.umid) || !isfinite(delta) ||
      !isfinite(movMax) || !isfinite(inclMax)) {
    Serial.println("[SENSOR] Feature ausente. Amostra nao enviada.");
    return;
  }

  JsonDocument doc;
  doc["device"] = MQTT_CLIENT_ID;
  doc["temperatura"] = ambiente.temp;
  doc["umidade"] = ambiente.umid;
  doc["delta_distancia"] = delta;
  doc["luz"] = luz;
  doc["mov_max"] = movMax;
  doc["incl_max"] = inclMax;

  String payload;
  serializeJson(doc, payload);
  if (!mqttClient.connected() || !mqttClient.publish(MQTT_PUB_TOPIC, payload.c_str())) {
    Serial.println("[MQTT] Amostra nao enviada");
  }
}

/* ---- A classe chega pronta da nuvem ----
   Nao ha nada de temporal aqui alem da validade: acendeu, FICA aceso ate
   chegar a proxima mensagem — e ela chega a cada segundo. */
void receberClasse(char* topico, byte* conteudo, unsigned int tamanho) {
  // O payload MQTT nao termina em '\0', por isso o String recebe o tamanho junto.
  String classe(conteudo, tamanho);
  classe.trim();

  // Mostra o que chegou ANTES de julgar: se o n8n publicar um JSON inteiro em
  // vez do nome da classe, e aqui que se ve.
  Serial.println("--- MQTT recebido ---");
  Serial.printf("  topico:  %s\r\n", topico);
  Serial.printf("  payload: \"%s\"\r\n", classe.c_str());

  apagarSaidas();

  if (classe == "ENTREGA_OK") {
    digitalWrite(LED_OK, HIGH);
    temResposta = true;
    ultimaResposta = millis();
    Serial.println("  MODELO:  ENTREGA_OK      -> LED verde aceso");

  } else if (classe == "REVISAR_ENTREGA") {
    digitalWrite(LED_REVISAR, HIGH);
    temResposta = true;
    ultimaResposta = millis();
    Serial.println("  MODELO:  REVISAR_ENTREGA -> LED vermelho aceso");

  } else {
    // Nome desconhecido e falha de comunicacao, nao ENTREGA_OK. Tudo apagado
    // avisa que algo esta errado; acender o verde mentiria.
    temResposta = false;
    Serial.println("  MODELO:  classe DESCONHECIDA - tudo apagado");
    Serial.println("           esperado, sem aspas e sem JSON:");
    Serial.println("           ENTREGA_OK | REVISAR_ENTREGA");
  }

  Serial.println("---------------------");
}
