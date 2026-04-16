# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 27.1 - Coleta de Imagens com ESP32-CAM para Edge Impulse

Servidor web para coleta de imagens usando ESP32-CAM, destinado ao treinamento de modelos de classificação de imagens ou detecção de objetos no Edge Impulse.

## Hardware

- ESP32-CAM (AI-Thinker)

## Configuração

Edite as credenciais WiFi no código:
```cpp
#define WIFI_SSID "sua_rede"
#define WIFI_PASS "sua_senha"
#define HOSTNAME "esp32cam"
```

## Como Usar

1. Compile e faça upload do código

2. Abra o Monitor Serial para obter o endereço IP:
   ```bash
   pio device monitor -b 115200
   ```

3. Acesse o servidor de coleta de imagens no navegador:
   - `http://<IP_DO_ESP32>` ou
   - `http://esp32cam.local` (se seu roteador suportar mDNS)

4. Capture imagens através da interface web e faça download

5. Faça upload das imagens no [Edge Impulse Studio](https://studio.edgeimpulse.com) em "Data acquisition"

## Especificações da Câmera

- Resolução: 240x240 (formato quadrado, ideal para Edge Impulse)
- Qualidade: máxima

## Biblioteca

- [EloquentEsp32cam](https://github.com/eloquentarduino/EloquentEsp32cam)
