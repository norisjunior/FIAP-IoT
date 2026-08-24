/* Aplicacao 19 - VACCINE SENSE: PREDICAO

   Este e o mesmo dispositivo da Aplicacao 18. A caixa continua publicando
   suas medicoes no broker, uma vez por segundo.

   Fluxo conhecido: SENSORES -> JSON -> BROKER -> Node-RED -> InfluxDB.
   Novidade: o app Python consulta a ultima medicao e chama modelo.predict().

   Mesma ideia do app06, com duas diferencas:
   - em vez de UM numero, publicamos um JSON com varias grandezas;
   - a leitura dos sensores mora nos .hpp; aqui so tratamos do MQTT.

   O ESP32 nao executa o modelo. A decisao acontece na aplicacao Python.
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "VC_TemperaturaCarga.hpp"
#include "VC_AmbienteExterno.hpp"
#include "VC_Luz.hpp"
#include "VC_Criticidade.hpp"
#include "VC_Carga.hpp"

// Altere somente estes dados para a rede da sala.
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_SENHA = "";
// const char* WIFI_SSID = "SUA_REDE";
// const char* WIFI_SENHA = "SUA_SENHA";

// No Wokwi use host.wokwi.internal. Na sala, o IP da Ethernet do notebook.
const char* BROKER_IP = "host.wokwi.internal";
// const char* BROKER_IP = "172.16.10.101";

const char* TOPICO = "fiap/iot/vaccinesense";
const char* CLIENT_ID = "ESP32Noris001Vaccine";

// Pinagem oficial do Vaccine Sense (a mesma do diagram.json do 1o ano).
const int PIN_DS18B20 = 23;
const int PIN_DHT = 22;
const int PIN_TRIG = 19;
const int PIN_ECHO = 18;
const int PIN_CRITICIDADE = 34;
const int PIN_LDR = 35;
const int PIN_LED = 25;
const int PIN_BUZZER = 26;
const int PIN_BOTAO = 27;

// Quantos segundos fora da faixa segura ate a carga precisar de avaliacao.
// Valor DIDATICO: na vida real ele vem da bula de cada imunobiologico.
const uint32_t LIMITE_TEMPO_FORA = 120;

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

uint64_t tempoAgora = 0;
const uint64_t INTERVALO_COLETA = 1000;  // 1 Hz: uma medicao por segundo

uint32_t idLeitura = 0;
uint32_t tempoForaDaFaixa = 0;

// Cada coleta iniciada e uma rodada nova. O numero vai junto no payload,
// e no Colab voce diz o que cada rodada foi (normal, hostil, perigo).
uint32_t rodada = 0;

// Botao inicia e para a coleta (mesmo padrao do app17-9).
bool coletando = false;
bool botaoAntes = false;
uint64_t ultimoToqueBotao = 0;

// Estado que o LED sinaliza.
bool ambienteHostil = false;
bool cargaComprometida = false;
uint64_t tempoPisca = 0;
bool ledPiscaAceso = false;

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

    if (mqtt.connect(CLIENT_ID)) {
      Serial.println(" conectado!");
    } else {
      Serial.println(" falhou. Nova tentativa em 2 segundos.");
      delay(2000);
    }
  }
}

// Cada toque no botao inicia ou para a coleta.
// Comecar uma coleta zera o contador e o id: cada rodada e uma viagem nova.
void atualizarBotao() {
  bool botaoAgora = (digitalRead(PIN_BOTAO) == LOW);

  if (botaoAgora && !botaoAntes && millis() - ultimoToqueBotao > 200) {
    ultimoToqueBotao = millis();
    coletando = !coletando;

    if (coletando) {
      rodada++;
      idLeitura = 0;
      tempoForaDaFaixa = 0;
      Serial.println(">>> Coleta INICIADA - rodada " + String(rodada));
    } else {
      ambienteHostil = false;
      cargaComprometida = false;
      Serial.println(">>> Coleta PARADA");
    }
  }

  botaoAntes = botaoAgora;
}

// LED aceso  = ambiente hostil (regra, um if).
// LED piscando = carga comprometida (acumulador) - tem prioridade.
void atualizarLED() {
  if (cargaComprometida) {
    if (millis() - tempoPisca >= 300) {
      tempoPisca = millis();
      ledPiscaAceso = !ledPiscaAceso;
      digitalWrite(PIN_LED, ledPiscaAceso);
    }
    return;
  }

  digitalWrite(PIN_LED, ambienteHostil);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_BOTAO, INPUT_PULLUP);

  TemperaturaCarga::inicializar(PIN_DS18B20);
  AmbienteExterno::inicializar(PIN_DHT);
  Luz::inicializar(PIN_LDR);
  Criticidade::inicializar(PIN_CRITICIDADE);
  Carga::inicializar(PIN_TRIG, PIN_ECHO);

  conectarWiFi();

  mqtt.setServer(BROKER_IP, 1883);
  mqtt.setKeepAlive(120);
  mqtt.setBufferSize(512);

  Serial.println("Vaccine Sense pronto. Topico: " + String(TOPICO));
  Serial.println("Pressione o botao para iniciar a coleta.");
}

void loop() {
  if (!mqtt.connected()) {
    conectarMQTT();
  }
  mqtt.loop();

  atualizarBotao();
  atualizarLED();

  if (millis() - tempoAgora < INTERVALO_COLETA) {
    return;
  }
  tempoAgora = millis();

  if (!coletando) {
    return;
  }

  // Um namespace por sensor - cada leitura na sua propria variavel.
  // Sao essas variaveis que, no app19, alimentam o predict do modelo.
  float tempInterna = TemperaturaCarga::lerCelsius();
  float tempExterna = AmbienteExterno::lerTemperatura();
  float umidade     = AmbienteExterno::lerUmidade();
  int   luz         = Luz::ler();
  int   criticidade = Criticidade::lerNivel();
  float distancia   = Carga::lerDistanciaCm();

  if (!TemperaturaCarga::leituraValida(tempInterna) ||
      !AmbienteExterno::leituraValida(tempExterna, umidade)) {
    Serial.println("Leitura invalida: nada publicado.");
    return;
  }

  // Regra: um sensor decide sozinho, acende o LED.
  ambienteHostil = AmbienteExterno::ambienteHostil(tempExterna);

  // Acumulador: exposicao termica da viagem. So cresce.
  if (TemperaturaCarga::foraDaFaixaSegura(tempInterna, criticidade)) {
    tempoForaDaFaixa++;
  }
  cargaComprometida = (tempoForaDaFaixa > LIMITE_TEMPO_FORA);

  // Payload: quem enviou, id, as 6 medicoes e o acumulador.
  // O horario quem carimba e o Node-RED.
  JsonDocument doc;
  doc["device"] = CLIENT_ID;
  doc["rodada"] = rodada;
  doc["id"] = idLeitura;
  doc["tempInterna"] = tempInterna;
  doc["tempExterna"] = tempExterna;
  doc["umidade"] = umidade;
  doc["luz"] = luz;
  doc["criticidade"] = criticidade;
  doc["distancia"] = distancia;
  doc["tempoForaDaFaixa"] = tempoForaDaFaixa;

  String json;
  serializeJson(doc, json);

  mqtt.publish(TOPICO, json.c_str());
  Serial.println(json);

  idLeitura++;
}
