# ESP32 + MQTT com TLS (VSCode + Wokwi + PlatformIO)

Incorporar o certificado da AC (criada no roteiro standalone, Parte 3) no projeto ESP32 para conexão MQTT via TLS na porta 8883.

---

## Passo 1 — Copiar o certificado da AC

No terminal WSL, exiba o conteúdo do `ca.crt`:

```bash
cat ~/aula_mqtt/certs/ca.crt
```

Copie **todo** o conteúdo (incluindo as linhas `BEGIN` e `END`).

---

## Passo 2 — Criar o arquivo `src/mqtt_ca_cert.hpp`

No projeto PlatformIO, crie o arquivo `src/mqtt_ca_cert.hpp`:

```cpp
#ifndef MQTT_CA_CERT_H
#define MQTT_CA_CERT_H

static const char MQTT_CA_CERT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
<cole aqui o conteúdo do ca.crt>
-----END CERTIFICATE-----
)EOF";

#endif
```

---

## Passo 3 — Ajustar o código `.ino`

Três mudanças em relação ao código sem TLS:

**3.1 — Includes**

```cpp
// Substituir:
#include <WiFiClient.h>

// Por:
#include <WiFiClientSecure.h>
#include "mqtt_ca_cert.hpp"
```

**3.2 — Tipo do cliente WiFi**

```cpp
// Substituir:
WiFiClient wifiClient;

// Por:
WiFiClientSecure wifiClient;
```

**3.3 — Configurar certificado e porta**

```cpp
#define MQTT_HOST "host.docker.internal"
#define MQTT_PORT 8883
```

No `setup()`, após conectar ao WiFi e antes do `client.setServer(...)`:

```cpp
wifiClient.setCACert(MQTT_CA_CERT);
```

---

## Resumo das mudanças

| Item | Sem TLS | Com TLS |
|---|---|---|
| Include | `WiFiClient.h` | `WiFiClientSecure.h` + `mqtt_ca_cert.hpp` |
| Tipo do cliente | `WiFiClient` | `WiFiClientSecure` |
| Porta | `1883` | `8883` |
| Host | IP do broker | `host.docker.internal` |
| Setup extra | — | `wifiClient.setCACert(MQTT_CA_CERT)` |

---

## Estrutura do projeto

```
projeto-esp32-mqtt-tls/
├── platformio.ini
├── src/
│   ├── app.ino
│   ├── mqtt_ca_cert.hpp           ← certificado da AC
│   └── ESP32SensorsDistancia.hpp  ← (se usar sensor)
├── diagram.json
└── wokwi.toml
```

---

## Verificação

1. Broker com TLS rodando (Parte 3 do roteiro standalone)
2. Compile e rode no Wokwi → Serial Monitor: `Conectado ao MQTT!`
3. Wireshark: tráfego aparece como **TLS** (cifrado), não MQTT em texto puro