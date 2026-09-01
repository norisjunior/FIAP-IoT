# App30 — MQTT com TLS (Transport Layer Security)

Semáforo inteligente com MQTT protegido por TLS + credenciais.

- **app30-1** — Smart Device: lê sensor de distância e publica no broker via TLS (porta 8883) quando detecta pessoa (≤ 50 cm)
- **app30-2** — Cloud: assina o tópico e controla o semáforo (LEDs) conforme as detecções

---

## Passo 1 — Iniciar a plataforma

Execute no WSL2:

```bash
cd FIAP-IoT/MQTT-TLS-IoT-platform
sudo ./start-MQTTTLS-iot-platform.sh
```

Aguarde a mensagem `Plataforma IoT (TLS) configurada e iniciada!`.

Ao final do script, o **certificado CA** será exibido no terminal. Copie o conteúdo entre `-----BEGIN CERTIFICATE-----` e `-----END CERTIFICATE-----` (inclusive) — você vai precisar dele no Passo 4.

> Para exibir o certificado novamente a qualquer momento:
> ```bash
> cat LLMIoTStack/mqtt-broker/certs/ca.crt
> ```

---

## Passo 2 — Cadastrar os usuários no broker MQTT

Execute no WSL2:

```bash
# Usuário do Smart Device (app30-1)
sudo docker exec mqtt-broker mosquitto_passwd -b /mosquitto/config/passwd smartdevice smart456

# Usuário do Cloud / Semáforo (app30-2)
sudo docker exec mqtt-broker mosquitto_passwd -b /mosquitto/config/passwd cloud cloud123
```

---

## Passo 3 — Configurar ACL e reiniciar o broker

Edite o arquivo `LLMIoTStack/mqtt-broker/acl` e adicione ao final:

```
user smartdevice
topic write semaforo/distancia

user cloud
topic read semaforo/distancia
```

Reinicie o broker para aplicar as mudanças:

```bash
sudo docker restart mqtt-broker
```

---

## Passo 4 — Configurar o certificado CA no app30-1

Abra o arquivo [app30-1-smartdevice-distancia/src/mqtt_ca_cert.h](app30-1-smartdevice-distancia/src/mqtt_ca_cert.h).

Substitua o conteúdo entre as aspas `R"EOF(` e `)EOF"` pelo certificado CA gerado pela sua plataforma (obtido no Passo 1):

```cpp
static const char MQTT_CA_CERT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
<cole aqui o conteúdo do seu ca.crt>
-----END CERTIFICATE-----
)EOF";
```

> **Por que isso é necessário?** O certificado gerado pela plataforma é único a cada execução do script. O ESP32 usa esse certificado para validar a identidade do broker antes de estabelecer a conexão TLS.

---

## Passo 5 — Verificar o app30-1 (Smart Device com TLS)

Abra o projeto `app30-1-smartdevice-distancia` no Wokwi e execute.

Observe no código [src/iot-app30-TLS-1-smartdevice-dist.ino](app30-1-smartdevice-distancia/src/iot-app30-TLS-1-smartdevice-dist.ino) as diferenças em relação ao app28-1:

| app28-1 (sem TLS) | app30-1 (com TLS) |
|---|---|
| `#include <WiFiClient.h>` | `#include <WiFiClientSecure.h>` |
| `WiFiClient wifiClient` | `WiFiClientSecure wifiClient` |
| `MQTT_PORT 1883` | `MQTT_PORT 8883` |
| — | `wifiClient.setCACert(MQTT_CA_CERT)` |

O dispositivo irá:
- Estabelecer conexão TLS com o broker na porta **8883**
- Conectar com usuário `smartdevice` / senha `smart456`
- Publicar no tópico `semaforo/distancia` quando detectar pessoa

---

## Passo 6 — Configurar o app30-2 (Cloud / Semáforo)

Abra o arquivo [app30-2-cloud-semaforo_inteligente/src/iot-app30-TLS-2-cloud-semaforo.ino](app30-2-cloud-semaforo_inteligente/src/iot-app30-TLS-2-cloud-semaforo.ino).

Faça as seguintes alterações para adicionar TLS e autenticação:

**1. Troque o include e o client:**
```cpp
// Remova:
#include <WiFiClient.h>
WiFiClient wifiClient;

// Adicione:
#include <WiFiClientSecure.h>
#include "mqtt_ca_cert.h"
WiFiClientSecure wifiClient;
```

**2. Altere a porta e adicione a senha:**
```cpp
#define MQTT_PORT       8883          // era 1883
const char* MQTT_PASS = "cloud123";   // <-- adicione esta linha
```

**3. Configure o certificado CA no `setup()`, antes de `mqttClient.setServer`:**
```cpp
wifiClient.setCACert(MQTT_CA_CERT);
```

**4. Passe as credenciais no `connect()`:**
```cpp
// era:
mqttClient.connect(MQTT_DEVICEID)

// passa a ser:
mqttClient.connect(MQTT_DEVICEID, MQTT_DEVICEID, MQTT_PASS)
```

**5. Copie o arquivo `mqtt_ca_cert.h`** do app30-1 para a pasta `src/` do app30-2.

---

## Passo 7 — Executar o app30-2

Abra o projeto `app30-2-cloud-semaforo_inteligente` no Wokwi e execute.

O semáforo irá:
- Conectar ao broker com TLS (porta 8883), usuário `cloud` / senha `cloud123`
- Permanecer verde por padrão
- Ao receber 2 detecções em 60 segundos: amarelo (2 s) → vermelho (5 s) → verde

---

## Passo 8 — Observar no Wireshark

Inicie uma captura e filtre pela porta TLS:

```
tcp.port == 8883
```

Observe o handshake TLS (`Client Hello`, `Server Hello`, `Certificate`). Diferente do app28, o payload MQTT agora trafega **criptografado** — não é possível ler as mensagens ou credenciais em texto claro.

Compare filtrando também a porta 1883 para ver a diferença visual entre tráfego sem e com TLS.
