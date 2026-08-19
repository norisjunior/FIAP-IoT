# App31-2 — AIoT + MQTT com TLS

Sistema de irrigação inteligente com TinyML e MQTT protegido por TLS + credenciais (porta 8883).

- **Device1-TinyML-TLS** — Roda modelo TFLite e publica a decisão em `FIAPIoT/smartagro/cmd/local` via TLS
- **Device2-BombaAgua-TLS** — Recebe os comandos e aciona a bomba (servo + LED) via TLS

> As credenciais e a configuração TLS já estão no código. O único passo obrigatório antes de executar é **atualizar o certificado CA** (veja Passo 4).

---

## Passo 1 — Iniciar a plataforma

Execute no WSL2:

```bash
cd FIAP-IoT/MQTT-TLS-IoT-platform
sudo ./start-MQTTTLS-iot-platform.sh
```

Aguarde a mensagem `Plataforma IoT (TLS) configurada e iniciada!`.

Ao final, o script exibe o **certificado CA** gerado. Você vai precisar dele no Passo 4.

> Para exibir novamente a qualquer momento:
> ```bash
> cat LLMIoTStack/mqtt-broker/certs/ca.crt
> ```

---

## Passo 2 — Cadastrar os usuários no broker MQTT

```bash
sudo docker exec mqtt-broker mosquitto_passwd -b /mosquitto/config/passwd FIAPIoTapp31Dev1 FIAP1234
sudo docker exec mqtt-broker mosquitto_passwd -b /mosquitto/config/passwd FIAPIoTapp31B FIAP1234
```

---

## Passo 3 — Configurar ACL e reiniciar o broker

Edite `LLMIoTStack/mqtt-broker/acl` e adicione ao final:

```
user FIAPIoTapp31Dev1
topic write FIAPIoT/smartagro/cmd/local

user FIAPIoTapp31B
topic read FIAPIoT/smartagro/cmd/+
```

Reinicie:

```bash
sudo docker restart mqtt-broker
```

---

## Passo 4 — Atualizar o certificado CA nos dispositivos

> **ATENÇÃO:** O certificado CA é gerado a cada execução do script. O `mqtt_ca_cert.h` incluído no projeto contém um certificado de exemplo que **não vai funcionar** com a sua plataforma. Você deve substituí-lo.

Copie o conteúdo do `ca.crt` da sua plataforma e substitua o certificado em **ambos** os arquivos abaixo:

- [Device1-TinyML-TLS/src/mqtt_ca_cert.h](Device1-TinyML-TLS/src/mqtt_ca_cert.h)
- [Device2-BombaAgua-TLS/src/mqtt_ca_cert.h](Device2-BombaAgua-TLS/src/mqtt_ca_cert.h)

O formato deve ser:

```cpp
static const char MQTT_CA_CERT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
<cole aqui o conteúdo do seu ca.crt>
-----END CERTIFICATE-----
)EOF";
```

---

## Passo 5 — Executar os dispositivos

Abra cada projeto no VSCode e execute via Wokwi:

1. `Device1-TinyML-TLS` — publica a cada 30 s a decisão do modelo (`bomba: true/false`) via TLS na porta 8883
2. `Device2-BombaAgua-TLS` — assina `FIAPIoT/smartagro/cmd/+` e aciona o servo conforme o comando recebido via TLS
