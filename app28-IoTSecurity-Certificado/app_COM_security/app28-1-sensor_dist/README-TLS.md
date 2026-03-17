# ESP32 + MQTT com TLS — Usando o certificado da AC no código

Neste roteiro, você vai pegar a chave pública da Autoridade Certificadora (AC) criada no exercício standalone (Parte 3) e incorporá-la no projeto ESP32 para se conectar ao broker MQTT via TLS (porta 8883).

---

## Pré-requisitos

- Inicialização da plataforma secure-LLM-IoT-platform
- Projeto ESP32 com MQTT funcionando (sem TLS)

---

## Passo 1 — Obter o conteúdo do certificado da AC

No terminal WSL (Ubuntu), exiba o conteúdo do `ca.crt`:

```bash
cat ~/aula_mqtt/certs/ca.crt
```

Saída esperada (exemplo — o conteúdo será diferente no seu caso):

```
-----BEGIN CERTIFICATE-----
MIIDkTCCAnmgAwIBAgIUE7x...
... (várias linhas em base64) ...
...q8J2r3dF1g==
-----END CERTIFICATE-----
```

**Copie todo o conteúdo**, incluindo as linhas `-----BEGIN CERTIFICATE-----` e `-----END CERTIFICATE-----`.

---

## Passo 2 — Criar o arquivo de certificado no projeto

No seu projeto PlatformIO, crie o arquivo `src/certs.hpp` com o seguinte conteúdo:

```cpp
#pragma once

// Certificado público da Autoridade Certificadora (CA)
// Gerado em: ~/aula_mqtt/certs/ca.crt
const char* ca_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDkTCCAnmgAwIBAgIUE7x...\n" \
"... (cole aqui linha por linha) ...\n" \
"...q8J2r3dF1g==\n" \
"-----END CERTIFICATE-----\n";
```

### Como formatar

Cada linha do certificado deve virar uma string C com `\n` no final. Exemplo real:

**Conteúdo original do `ca.crt`:**
```
-----BEGIN CERTIFICATE-----
MIIC9jCCAd6gAwIBAgIUBz3r
Hk2F+aBcDeFgHiJkLmNoPqRs
-----END CERTIFICATE-----
```

**Formatado no `certs.hpp`:**
```cpp
const char* ca_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIC9jCCAd6gAwIBAgIUBz3r\n" \
"Hk2F+aBcDeFgHiJkLmNoPqRs\n" \
"-----END CERTIFICATE-----\n";
```

> **Dica:** Você pode usar o comando abaixo no terminal para gerar o formato automaticamente:
>
> ```bash
> awk 'NF {printf "\"%s\\n\" \\\n", $0}' ~/aula_mqtt/certs/ca.crt
> ```
>
> Copie a saída e cole dentro do `certs.hpp`, adicionando `const char* ca_cert = \` no início e `;` no final.

---

## Passo 3 — Atualizar o `platformio.ini`

Nenhuma biblioteca extra é necessária. O `WiFiClientSecure` já faz parte do framework Arduino para ESP32.

```ini
[env:esp32]
platform = espressif32
framework = arduino
board = esp32dev
lib_deps =
    bblanchon/ArduinoJson @ ^7.4.1
    knolleary/PubSubClient @ ^2.8
```

---

## Passo 4 — Ajustar o código `.ino`

Três mudanças em relação ao código sem TLS:

### 4.1 — Trocar os includes e importar o certificado

**Antes (sem TLS):**
```cpp
#include <WiFi.h>
#include <WiFiClient.h>
```

**Depois (com TLS):**
```cpp
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "certs.hpp"
```

### 4.2 — Trocar o tipo do cliente WiFi

**Antes (sem TLS):**
```cpp
WiFiClient wifiClient;
```

**Depois (com TLS):**
```cpp
WiFiClientSecure wifiClient;
```

### 4.3 — Configurar o certificado da AC e a porta

No `setup()`, **após** conectar ao WiFi e **antes** de `client.setServer(...)`:

```cpp
// Configura o certificado da AC para validar o broker
wifiClient.setCACert(ca_cert);
```

E ajuste a porta do broker:

**Antes (sem TLS):**
```cpp
#define MQTT_PORT 1883
```

**Depois (com TLS):**
```cpp
#define MQTT_PORT 8883
```

---

## Passo 5 — Ajustar o IP do broker

No Wokwi, o ESP32 simulado acessa o broker via `host.docker.internal` (mesmo host do Docker). Ajuste o `MQTT_HOST`:

```cpp
#define MQTT_HOST "host.docker.internal"
```

> **Atenção:** Esse endereço deve estar listado no SAN (Subject Alternative Name) do certificado do servidor, conforme configurado no Passo 3.2 do roteiro standalone (`DNS.3 = host.docker.internal`).

---

## Resumo das mudanças (sem TLS → com TLS)

| Item | Sem TLS | Com TLS |
|---|---|---|
| Include | `WiFiClient.h` | `WiFiClientSecure.h` + `certs.hpp` |
| Tipo do cliente | `WiFiClient` | `WiFiClientSecure` |
| Porta | `1883` | `8883` |
| Configuração extra | — | `wifiClient.setCACert(ca_cert)` |
| Arquivo novo | — | `src/certs.hpp` |

---

## Estrutura final do projeto

```
projeto-esp32-mqtt-tls/
├── platformio.ini
├── src/
│   ├── app.ino
│   ├── certs.hpp                  ← certificado da AC
│   └── ESP32SensorsDistancia.hpp  ← (se usar sensor)
├── diagram.json
└── wokwi.toml
```

---

## Verificação

1. Suba o broker com TLS (Parte 3 do roteiro standalone)
2. Compile e rode o ESP32 no Wokwi
3. No Serial Monitor, deve aparecer: `Conectado ao MQTT!`
4. No Wireshark, o tráfego deve aparecer como **TLS** (cifrado), não como MQTT em texto puro