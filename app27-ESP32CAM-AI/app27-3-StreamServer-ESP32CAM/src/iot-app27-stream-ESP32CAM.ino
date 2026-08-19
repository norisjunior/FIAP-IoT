/**
 * ESP32CAM - MJPEG Stream Server
 *
 * Expõe dois endpoints para consumo externo:
 *   /stream  -> MJPEG contínuo (para exibir em <img> ou OpenCV)
 *   /capture -> JPEG único     (para captura pontual)
 *
 * Exemplos de consumo:
 *   HTML:   <img src="http://<IP>/stream">
 *   Python: cv2.VideoCapture("http://<IP>/stream")
 *   curl:   curl http://<IP>/capture --output frame.jpg
 *
 * BE SURE TO SET "TOOLS > CORE DEBUG LEVEL = INFO"
 */

#define WIFI_SSID "IoTNJ"
#define WIFI_PASS "Th1ng$IoT"
#define HOSTNAME  "esp32cam"

#include <eloquent_esp32cam.h>
#include <eloquent_esp32cam/extra/esp32/wifi/sta.h>
#include <WebServer.h>

using eloq::camera;
using eloq::wifi;

WebServer server(80);

// ------------------------------------------------------------------
// /capture  ->  JPEG único
// ------------------------------------------------------------------
void handleCapture() {
    if (!camera.capture().isOk()) {
        server.send(503, "text/plain", camera.exception.toString());
        return;
    }
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send_P(200, "image/jpeg",
                  (const char*) camera.frame->buf,
                  camera.frame->len);
    esp_camera_fb_return(camera.frame);
}

// ------------------------------------------------------------------
// /stream  ->  MJPEG multipart contínuo
// ------------------------------------------------------------------
void handleStream() {
    WiFiClient client = server.client();

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Cache-Control: no-cache");
    client.println();

    while (client.connected()) {
        if (!camera.capture().isOk()) continue;

        client.println("--frame");
        client.println("Content-Type: image/jpeg");
        client.print("Content-Length: ");
        client.println(camera.frame->len);
        client.println();
        client.write(camera.frame->buf, camera.frame->len);
        client.println();

        esp_camera_fb_return(camera.frame);
    }
}

// ------------------------------------------------------------------
// Setup
// ------------------------------------------------------------------
void setup() {
    delay(3000);
    Serial.begin(115200);
    Serial.println("___ESP32CAM MJPEG STREAM SERVER___");


    // Método	Resolução	Uso típico
    // thumbnail()	96×96	modelos tiny
    // qqvga()	160×120	
    // face()	240×240	Edge Impulse face
    // qvga()	320×240	stream leve
    // hvga()	480×320	
    // vga()	640×480	stream ideal
    // svga()	800×600	
    // xga()	1024×768	
    // hd()	1280×720	
    // sxga()	1280×1024	
    // uxga()	1600×1200	máximo (captura)


    camera.pinout.aithinker();
    camera.brownout.disable();
    camera.resolution.vga();   // 240x240 - bom para modelos Edge Impulse
    camera.quality.best();

    while (!camera.begin().isOk())
        Serial.println(camera.exception.toString());

    while (!wifi.connect().isOk())
        Serial.println(wifi.exception.toString());

    server.on("/capture", HTTP_GET, handleCapture);
    server.on("/stream",  HTTP_GET, handleStream);
    server.begin();

    Serial.println("Camera OK");
    Serial.println("WiFi OK");
    Serial.print("Stream:  http://");
    Serial.print(WiFi.localIP());
    Serial.println("/stream");
    Serial.print("Capture: http://");
    Serial.print(WiFi.localIP());
    Serial.println("/capture");
}

// ------------------------------------------------------------------
// Loop
// ------------------------------------------------------------------
void loop() {
    server.handleClient();
}
