# App31-1 — AIoT + MQTT com Autenticação (Usuário e Senha)

Sistema de irrigação inteligente com TinyML e MQTT protegido por credenciais.

- **Device1-TinyML** — Roda modelo TFLite, decide se deve irrigar e publica a decisão em `FIAPIoT/smartagro/cmd/local`
- **Device2-BombaAgua** — Recebe os comandos e aciona a bomba (servo + LED)

> As credenciais já estão configuradas no código. Basta cadastrá-las no broker.

---

## Passo 1 — Iniciar a plataforma

Execute no WSL2:

```bash
cd FIAP-IoT/MQTT-UserPass-IoT-platform
sudo ./start-MQTTACL-iot-platform.sh
```

Aguarde a mensagem `Plataforma IoT configurada e iniciada!`.

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

## Passo 4 — Executar os dispositivos

Abra cada projeto no VSCode e execute via Wokwi:

1. `Device1-TinyML` — publica a cada 30 s a decisão do modelo (`bomba: true/false`)
2. `Device2-BombaAgua` — assina `FIAPIoT/smartagro/cmd/+` e aciona o servo conforme o comando recebido
