
# 1) Ajustar sua plataforma (Docker + Mosquitto) para MQTT-TLS

## 0: Parar a plataforma:

```
sudo ./stop-secure-llm-iot-platform.sh
```

## 1.1. Gerar certificados (CA + servidor)

No seu host (WSL2), dentro de `LLMIoTStack/mqtt-broker/`:

```
cd LLMIoTStack/mqtt-broker
mkdir -p certs
cd certs
```

### CA (autoridade certificadora local)
```
sudo openssl genrsa -out ca.key 2048
```

```
sudo openssl req -x509 -new -nodes -key ca.key -sha256 -days 3650 \
  -subj "/CN=FIAP-IoT-CA" -out ca.crt
```

### Certificado do servidor (Mosquitto)
```
sudo openssl genrsa -out server.key 2048
```
```
sudo openssl req -new -key server.key -subj "/CN=mqtt-broker" -out server.csr
```

#### Crie o arquivo server.ext com o conteúdo a seguir.
```
sudo vim server.ext
```
Conteúdo:

```
subjectAltName = DNS:mqtt-broker,DNS:localhost,DNS:host.docker.internal,DNS:host.wokwi.internal,IP:127.0.0.1
```


```
sudo openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial \
  -out server.crt -days 825 -sha256 -extfile server.ext
```

### Permissões (mosquitto roda como 1883)
```
cd ..
sudo chown -R 1883:1883 certs
sudo chmod 640 certs/*.key
sudo chmod 644 certs/*.crt
```



## 1.2. Atualizar mosquitto.conf para TLS na 8883 (e opcionalmente desligar 1883)

Substitua seu `mqtt-broker/mosquitto.conf` por este (estando dentro do diretório `LLMIoTStack/mqtt-broker`):

```bash
sudo vim mosquitto.conf
```


```conf
persistence true
persistence_location /mosquitto/data/

# Autenticação
allow_anonymous false
password_file /mosquitto/config/passwd

# Controle de acesso por tópico
acl_file /mosquitto/config/acl

# =======================
# MQTT com TLS (8883)
# =======================
listener 8883
protocol mqtt

cafile /mosquitto/config/certs/ca.crt
certfile /mosquitto/config/certs/server.crt
keyfile /mosquitto/config/certs/server.key

require_certificate false
tls_version tlsv1.2

# (opcional) Se quiser DESABILITAR MQTT sem TLS, NÃO declare listener 1883.
# Se quiser manter 1883 (lab), descomente:
# listener 1883
```


## 1.3. Expor a porta 8883 no docker-compose.yml

No serviço mosquitto, ajuste as portas para publicar apenas a porta segura. Estando no diretório `LLMIoTStack`, edite o arquivo:
```bash
sudo vim docker-compose.yml
```

Ajuste o container do mosquitto:
```yaml
  mosquitto:
    image: eclipse-mosquitto:latest
    container_name: mqtt-broker
    ports:
      - "8883:8883"
      # - "1883:1883"   # opcional (sem TLS)
      - "9001:9001"
    volumes:
      - ./mqtt-broker:/mosquitto/config
```

## 1.4 Inicie a plataforma novamente

```bash
cd LLMIoTStack
sudo docker compose up -d
```

## 1.5 Lembre-se de conferir se os usuários e senhas estão criados

### Para verificar:

Estando no diretório `LLMIoTStack`:

```bash
sudo cat mqtt-broker/passwd
```

Devem constar as senhas dos usuários admin, FIAPIoTapp28Dev1 e FIAPIoTapp28B

Caso não estejam cadastradas as senhas, cadastrar com o seguinte comando (use a mesma senha que está na aplicação .ino) - Exemplo para o dispositivo FIAPIoTapp28Dev1:

```bash
sudo docker exec -it mqtt-broker mosquitto_passwd /mosquitto/config/passwd FIAPIoTapp28Dev1
```

## 1.6 Verifique as regras de ACL para acesso a publish e subscribe em tópicos

### Para verificar:

Estando no diretório `LLMIoTStack`:

```bash
sudo cat mqtt-broker/acl
```

Devem constar as regras, com algo similar a:

```conf
user FIAPIoTapp28Dev1
topic write FIAPIoT/smartagro/cmd/local

user FIAPIoTapp28B
topic read FIAPIoT/smartagro/cmd/+
```

Caso não haja as regras, crie-as editando o arquivo `mqtt-broker/acl`:
```bash
sudo vim mqtt-broker/acl
```


## 1.7 Reinicie a plataforma

```bash
cd LLMIoTStack
sudo docker restart mqtt-broker
```


## 1.8 Teste rápido do TLS (no host):

# necessário precisa do pacote mosquitto-clients no host
```bash
sudo apt-get update && sudo apt-get install -y mosquitto-clients
```

```bash
mosquitto_sub -h localhost -p 8883 --cafile mqtt-broker/certs/ca.crt \
  -u admin -P FIAP1234 -t 'FIAPIoT/smartagro/cmd/#' -d
```


# 2) Ajuste mínimo no código dos dispositivos IoT (ESP32)

O código no ESP usa `WiFiClient wifiClient;` e porta `1883`.

Para TLS, a mudança necessária é:

- trocar WiFiClient por WiFiClientSecure

- trocar porta para 8883

- carregar o CA com setCACert

- conectar no MQTT com usuário/senha

## 1. Criar arquivo separado para o certificado

Dentro de:

```Device1-TinyML/src/```

Crie um novo arquivo:

```mqtt_ca_cert.h```

Conteúdo:

```C
#ifndef MQTT_CA_CERT_H
#define MQTT_CA_CERT_H

static const char MQTT_CA_CERT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
COLE_AQUI_O_CONTEUDO_COMPLETO_DO_ca.crt
-----END CERTIFICATE-----
)EOF";

#endif
```
Para obter o conteúdo completo do ca.crt, acesse o diretório `LLMIoTStack/mqtt-broker/certs` e mostre o conteúdo do arquivo ca.crt:
```bash
cd mqtt-broker/certs

cat ca.crt
```

Você obterá algo similar a:
```
-----BEGIN CERTIFICATE-----
MIIDDTCCAfWgAwIBAgIUFi3TNr8eLx49FHz6RK7+MZg29xAwDQYJKoZIhvcNAQEL
BQAwFjEUMBIGA1UEAwwLRklBUC1Jb1QtQ0EwHhcNMjYwMzAyMTYzNTMyWhcNMzYw
MjI4MTYzNTMyWjAWMRQwEgYDVQQDDAtGSUFQLUlvVC1DQTCCASIwDQYJKoZIhvcN
AQEBBQADggEPADCCAQoCggEBANV4vxRAvsq4jSJEIRRrsIQ9RKoyriTYTBGsjC+R
8Px67vSqsN1c+VxrWUObYAmzuC3gyNYHFVPHTjgzDlPyPj+Lt0Nx1oCyK48Wsojl
ADb13ZUYdGGqrF/1+P4evsRsxZsHaiTI2J6ue5j01UZVvF4IVpJnpQhV/R/XQidm
KO27seB2q6RpfNZC33miBYEv6pFFy/T9gnMIMx4gLlmQLOy6sPY0jRUuzez9fED7
nCNN7vX4Nc/hQVATIq62mdlgA4HcR/3qPDPY426pWilyvKesjOs5SSc2L/B4mcYE
gjm/moiSNdztS7pbPX07yKc2uky1pibYW39R88YWlsiuTrkCAwEAAaNTMFEwHQYD
VR0OBBYEFLC6wZLBWsTgTtmLzut31h1OuO4uMB8GA1UdIwQYMBaAFLC6wZLBWsTg
TtmLzut31h1OuO4uMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEB
AKJQlc38sYq74w0kLgCaHHXSdiluj5RQbF37tAebtq1D2YcWNZhgCWXIDhkdwxhX
EmKkS01FyD7nkl5UIuzZ/WdmNfHS8SEP0cVxDnagOMXUMc1BlU4X5uNnYGHfShA3
Hs8VbpWywbRsdmvYkOWDNxq0ro0e+/l+mhjmXihIw6l4vbjhkKZwijutbJkI81zi
zAgUs6EIHY94q+yfKYcxagUVmSPwICJvOUeJjPo5vlX8a2s79jN/Nx7aaKF06GyJ
ceh+I5W20JCQfMXCCycdcawgvCL9CATTy6XZ32MmNArn8GBDT6VRHQC+ssVBsAw+
qX1U8x3YZFhcg3fMb6u0PHw=
-----END CERTIFICATE-----
```

É isso que deve constar como conteúdo da variável MQTT_CA_CERT[].

Observações importantes:

- Uso de `PROGMEM`: garante que o certificado fique em flash, não na RAM.

- Não altere nenhuma linha do certificado.

- Preserve BEGIN e END.

## 2. Ajustes no código:

### No preâmbulo:

#### Inclua a "biblioteca" contendo o certificado nos dispositivos (publisher e subscriber)

Adicione o include no topo do arquivo:
```C
#include "mqtt_ca_cert.h"
```

#### Substitua

```C
#include <WiFiClient.h>

WiFiClient wifiClient;

#define MQTT_PORT 1883
```


#### por:

```C
#include <WiFiClientSecure.h>

WiFiClientSecure wifiClient;

#define MQTT_PORT 8883
```

### No setup(), antes de mqttClient.setServer(...):

#### Inclua:

```C
wifiClient.setCACert(MQTT_CA_CERT);
```


