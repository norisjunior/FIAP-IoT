# App29 — MQTT com Autenticação (Usuário e Senha)

Semáforo inteligente com MQTT protegido por credenciais.

- **app29-1** — Smart Device: lê sensor de distância e publica no broker quando detecta pessoa (≤ 50 cm)
- **app29-2** — Cloud: assina o tópico e controla o semáforo (LEDs) conforme as detecções

---

## Passo 1 — Iniciar a plataforma

Execute no WSL2:

```bash
cd FIAP-IoT/secure-LLM-IoT-platform
sudo ./start-MQTTACL-llm-iot-platform.sh
```

Aguarde a mensagem `Plataforma IoT configurada e iniciada!`.

---

## Passo 2 — Cadastrar os usuários no broker MQTT

Execute no WSL2:

```bash
# Usuário do Smart Device (app29-1)
sudo docker exec mqtt-broker mosquitto_passwd -b /mosquitto/config/passwd smartdevice smart456

# Usuário do Cloud / Semáforo (app29-2)
sudo docker exec mqtt-broker mosquitto_passwd -b /mosquitto/config/passwd cloud cloud123
```

### Configurar ACL (quem pode publicar/assinar)

Edite o arquivo `LLMIoTStack/mqtt-broker/acl` e adicione ao final:

```
user smartdevice
topic write semaforo/distancia

user cloud
topic read semaforo/distancia
```

### Reiniciar o broker para aplicar as mudanças

```bash
sudo docker restart mqtt-broker
```

---

## Passo 3 — Executar o app29-1 (Smart Device)

Abra o projeto `app29-1-smartdevice-distancia` no Wokwi e execute.

O dispositivo irá:
- Conectar ao broker com usuário `smartdevice` / senha `smart456`
- Publicar no tópico `semaforo/distancia` quando detectar pessoa

---

## Passo 4 — Configurar o app29-2 (Cloud / Semáforo)

Abra o arquivo [app29-2-cloud-semaforo_inteligente/src/iot-app29-2-cloud.ino](app29-2-cloud-semaforo_inteligente/src/iot-app29-2-cloud.ino).

Localize o bloco de configuração MQTT e **adicione a senha**:

```cpp
/* ---- Config MQTT ---- */
#define MQTT_HOST       "host.wokwi.internal"
#define MQTT_PORT       1883
#define MQTT_SUB_TOPIC  "semaforo/distancia"
#define MQTT_DEVICEID   "cloud"
const char* MQTT_PASS = "cloud123";   // <-- adicione esta linha
```

> Sem essa linha o código não compila!

---

## Passo 5 — Executar o app29-2

Abra o projeto `app29-2-cloud-semaforo_inteligente` no Wokwi e execute.

O semáforo irá:
- Conectar ao broker com usuário `cloud` / senha `cloud123`
- Permanecer verde por padrão
- Ao receber 2 detecções em 60 segundos: amarelo (2 s) → vermelho (5 s) → verde

---

## Passo 6 — Observar no Wireshark

Inicie uma captura na interface de rede (ou loopback) e filtre:

```
tcp.port == 1883
```

Observe o handshake MQTT (`CONNECT`, `CONNACK`) e as mensagens (`PUBLISH`, `SUBSCRIBE`). Note que o payload JSON e as credenciais trafegam **em texto claro** — essa é a vulnerabilidade que o app29 resolve com TLS.
