# App07 — Assinante MQTT com LED

O ESP32 assina `fiap/iot/distancia`, recebe a distância em texto e acende o LED do GPIO 21 quando o valor é menor ou igual a 50 cm. O callback `receberMensagem()` foi preservado integralmente. O `diagram.json` foi preservado.

## Configuração para a aula

No Wokwi: `Wokwi-GUEST`, senha vazia e `MQTT_BROKER = "host.wokwi.internal"`. O canal é automático; `WiFi.begin` recebe apenas SSID e senha. O app06 usa o mesmo broker e tópico.

No kit físico, configure SSID/senha e broker `172.16.10.101` (ou o endereço da sua sala). Porta 1883, sem autenticação ou TLS, como no original.

Troque `12345678` em `MQTT_CLIENT_ID` pelo seu RA. O sufixo `app07-01` distingue o assinante do publicador. IDs precisam ser diferentes entre alunos e entre devices; a MAC padrão do Wokwi não garante isso.

## Conexão e callback

As funções ficam no `.ino`, com a estrutura do app14. WiFi e MQTT usam `millis()` e uma tentativa por intervalo de 5 s; os tempos são `uint32_t`. `conectarWiFi()` é idêntica à dos pilotos app04/app06, e o controle de reconexão MQTT segue a mesma estrutura do app06. No assinante, acrescentamos `mqttClient.subscribe(TOPICO);` após cada conexão MQTT bem-sucedida, incluindo reconexões.

O loop usa a sequência do app14: WiFi desconectado → conectar WiFi; senão, MQTT desconectado → conectar MQTT; senão → `mqttClient.loop()`. A espera original de 100 ms permanece.

A mensagem `[WiFi] Conectado` e o IP são impressos dentro de `conectarWiFi()`, após aguardar `WL_CONNECTED` com `while` e `delay(500)`. A função mostra o IP obtido em cada conexão iniciada, inclusive reconexões. Não há flag nem impressão de IP no loop e não há mensagem durante a tentativa.

O controle por intervalo do app14 permanece; a primeira tentativa acontece após o intervalo inicial de 5 s, sem acrescentar `delay` no setup. Enquanto aguarda a conexão, o processamento do loop fica pausado.

`PubSubClient::connect()` continua síncrono. O timeout de 2 s configurado limita a espera pela resposta MQTT; não limita toda a conexão TCP. Por isso uma tentativa ainda pode atrasar o processamento do callback. O exemplo não acrescenta tarefas, ponteiros de função próprios, filas ou alocação dinâmica.

## Compilação

```powershell
pio run
```

PubSubClient fixada em 2.8; monitor a 115200. A revisão piloto faz somente compilação, sem simular ou gravar.

## Referências

- [PubSubClient 2.8 — API](https://github.com/knolleary/pubsubclient/blob/v2.8/src/PubSubClient.h)
- [PubSubClient — conexão e callback](https://github.com/knolleary/pubsubclient/blob/v2.8/src/PubSubClient.cpp)
- [WiFi do Arduino-ESP32 2.0.17](https://github.com/espressif/arduino-esp32/blob/2.0.17/libraries/WiFi/src/WiFiSTA.h)
- [Rede e MAC no Wokwi](https://docs.wokwi.com/guides/esp32-wifi)
