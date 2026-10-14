# Modelo WiFi

## Conexão WiFi

O loop verifica `WiFi.status() != WL_CONNECTED` e chama `conectarWiFi()`. A função usa `WiFi.begin(WIFI_SSID, WIFI_SENHA)`, com canal automático, aguarda conectar e então mostra `[WiFi] Conectado` e o IP. O IP fica dentro da função, inclusive ao reconectar.

`INTERVALO_RECONEXAO` e `ultimaTentativaWiFi` são `uint32_t`. O controle usa 5 s. Enquanto a função aguarda a rede, o restante do programa pausa.

Os TODOs de MQTT continuam como exercício. Veja [passo a passo](README-MQTT-Passo-a-passo.md), com publicação e assinatura explícita quando houver subscribe.
