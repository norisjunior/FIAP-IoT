# app14 — publicação NexoLog

## Conexão WiFi

O loop verifica `WiFi.status() != WL_CONNECTED` e chama `conectarWiFi()`. A função usa `WiFi.begin(WIFI_SSID, WIFI_SENHA)`, com canal automático, aguarda conectar e então mostra `[WiFi] Conectado` e o IP. O IP fica dentro da função, inclusive ao reconectar.

`INTERVALO_RECONEXAO` e `ultimaTentativaWiFi` são `uint32_t`. O controle usa 5 s. Enquanto a função aguarda a rede, o restante do programa pausa.

## Conexão MQTT

Com WiFi conectado, o loop chama `conectarMQTT()` quando o cliente está desconectado; conectado, chama `mqttClient.loop()`. O controle de intervalo usa `uint32_t` e 5 s, como no app14.

`MQTT_BROKER` é `host.wokwi.internal` para a simulação; na placa física, use o endereço do broker na sua rede. `MQTT_PORTA` é 1883. Troque o RA de exemplo em `MQTT_CLIENT_ID` pelo seu e mantenha um ID distinto para cada dispositivo. PubSubClient está fixado em 2.8, com `setKeepAlive(120)` e `setSocketTimeout(2)`. Esse timeout é da resposta MQTT; a conexão TCP também é síncrona.

Este app publica e não assina tópicos. `DISPOSITIVO_ID` mantém `NexoLogEquipe01` no campo `device` do JSON. Veja [roteiro](CONSTRUIR-O-FIRMWARE.md) e [contexto da atividade](../README.md).
