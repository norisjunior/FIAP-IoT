/* ==========================================================================
   Physical Computing, Embedded AI, Robotics & Cognitive IoT

   Aplicação 07 - Dispositivo do GRUPO

   1) ASSINA  o tópico do professor -> acende o LED/buzzer quando ele chegar
      perto do sensor, de acordo com o limiar escolhido pelo grupo.
   2) PUBLICA no tópico do grupo    -> avisa o servidor quando o botão é
      apertado.

   Ligações:  LED + resistor 220R -> GPIO 2   (catodo no GND)
              Buzzer ativo        -> GPIO 4   ((-) no GND)
              Botão               -> GPIO 19 e GND
   ========================================================================== */

#include <WiFi.h>
#include <PubSubClient.h>

/* ===================== EDITE AQUI ===================== */
#define MEU_GRUPO   "01"              // "01" até "10"
#define MEU_LIMIAR  120               // cm - a regra é do SEU grupo
#define BROKER_IP   "host.wokwi.internal"   // IP do notebook do professor

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";
/* ====================================================== */

#define LED     21
#define BUZZER  19
#define BOTAO   18

#define TOPICO_PROF   "fiap/iot/2026/prof/#"                     // tudo do professor
#define TOPICO_DIST   "fiap/iot/2026/prof/dist"
#define TOPICO_MEU    "fiap/iot/2026/grupo/" MEU_GRUPO "/cmd"    // só do meu grupo
#define TOPICO_BOTAO  "fiap/iot/2026/grupo/" MEU_GRUPO "/botao"

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

void acionar(bool ligado) {
  digitalWrite(LED, ligado);
  //digitalWrite(BUZZER, ligado);   // 2o round da demo: descomente e regrave
}

/* ---- Aqui tratamos a mensagem: tópico e texto, já prontos ---- */
void aoReceberMensagem(String topico, String msg) {
  Serial.println("[RX] " + topico + " -> " + msg);

  if (topico == TOPICO_DIST) {
    float dist = msg.toFloat();
    acionar(dist > 0 && dist <= MEU_LIMIAR);
  } else {
    acionar(true);      // ping do professor, ou comando só para este grupo
    delay(300);
    acionar(false);
  }
}

/* ---- Ponte: o PubSubClient entrega bytes, aqui eles viram texto ---- */
void callbackMQTT(char* topico, byte* payload, int tamanho) {
  aoReceberMensagem(String(topico), String(payload, tamanho));
}

void conectarMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Conectando ao broker...");
    if (mqtt.connect("grupo" MEU_GRUPO)) {   // cada grupo com um ID diferente
      Serial.println(" conectado!");
      mqtt.subscribe(TOPICO_PROF);
      mqtt.subscribe(TOPICO_MEU);
    } else {
      Serial.println(" falhou. Tentando de novo em 3 s.");
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(BOTAO, INPUT_PULLUP);   // botão ligado ao GND: solto = HIGH

  Serial.print("Grupo " MEU_GRUPO " - conectando ao WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println(" conectado!");

  mqtt.setServer(BROKER_IP, 1883);
  mqtt.setCallback(callbackMQTT);
}

void loop() {
  if (!mqtt.connected()) conectarMQTT();
  mqtt.loop();                    // sem esta linha nada é recebido

  if (digitalRead(BOTAO) == LOW) {
    mqtt.publish(TOPICO_BOTAO, "1");
    Serial.println("[TX] " TOPICO_BOTAO);
    delay(300);                   // não repete enquanto o botão está apertado
  }
}
