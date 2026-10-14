# App06 — Publicador MQTT de distância

O ESP32 mede a distância com HC-SR04 e publica em `fiap/iot/distancia`, a cada 500 ms. O payload continua sendo uma distância inteira em texto. TRIG: GPIO 22; ECHO: GPIO 23. O circuito do Wokwi foi preservado.

## Configuração para a aula

No Wokwi, use `Wokwi-GUEST` e senha vazia. O canal é automático: `WiFi.begin` recebe apenas SSID e senha. `MQTT_BROKER = "host.wokwi.internal"` acessa o broker no seu computador pelo gateway do Wokwi. O app07 usa o mesmo broker e tópico.

No kit físico, informe SSID/senha da rede e broker `172.16.10.101` (ou o endereço do broker da sua sala). A porta é 1883, sem autenticação e sem TLS, como neste exemplo original.

Troque `12345678` em `MQTT_CLIENT_ID` pelo seu RA. Cada aluno precisa de um ID diferente, e os dois devices também usam sufixos diferentes. IDs repetidos no broker derrubam a sessão anterior. Não use só a MAC para isso: as simulações Wokwi compartilham uma MAC padrão.

## Sensor em header

`ESP32Sensors::Distancia::inicializar(...)` configura os pinos. `medirDistancia()` faz o pulso no TRIG, mede o ECHO com `pulseIn` e retorna `DISTANCIA`, com `cm`, `duracaoUs` e `valido`. A fórmula `duracao * 0.034 / 2` fica visível no header, como nos slides. Não é necessária biblioteca externa para o HC-SR04.

Para preservar o payload do exemplo que usava NewPing 1.9.7, este app converte `duracaoUs / 57` usando divisão inteira, mantendo 400 cm para ausência de eco ou distância fora do limite. O limite de duração corresponde a `(400 + 1) * 57`. O tempo de aquisição não é idêntico ao da NewPing: o pulso agora é 10 us (antes 12 us) e o `pulseIn` tem seu próprio mecanismo de timeout. A mudança de aquisição precisa ser observada no hardware/simulador quando o professor fizer essa validação.

## WiFi e MQTT

As funções ficam no próprio `.ino`, seguindo o app14. WiFi e MQTT usam o mesmo controle por `millis()`: uma tentativa por intervalo de 5 s, com horários em `uint32_t`. `conectarWiFi()` é idêntica à dos pilotos app04/app07, e o controle de reconexão MQTT segue a mesma estrutura do app07.

No loop: se WiFi está desconectado, chama `conectarWiFi()`; senão, se MQTT está desconectado, chama `conectarMQTT()`; senão, processa `mqttClient.loop()`. O publicador não faz subscribe; no assinante, acrescentamos a chamada explícita após conectar.

A mensagem `[WiFi] Conectado` e o IP ficam dentro de `conectarWiFi()`. Depois de iniciar a conexão, a função aguarda `WL_CONNECTED` com `while` e `delay(500)` e mostra o IP obtido. Se uma reconexão obtiver outro IP, ele será impresso pela mesma função. Não há flag nem mensagens WiFi no loop ou durante a tentativa.

O controle por intervalo do app14 permanece, com `uint32_t`. A primeira tentativa acontece após o intervalo inicial de 5 s, sem acrescentar `delay` no setup. Durante a espera da conexão, coleta e processamento do loop ficam pausados. A duração do ECHO e os tempos de coleta usam `uint32_t`.

O `connect()` da PubSubClient é síncrono. `setSocketTimeout(2)` limita a espera pela resposta MQTT a 2 s; a conexão TCP também pode esperar, portanto o tempo total da tentativa pode ser maior. Tentativas espaçadas não garantem que a aquisição a cada 500 ms continue pontual durante uma falha de rede. Não há armazenamento/reenvio das publicações perdidas.

## Compilação

```powershell
pio run
```

PubSubClient fixada em 2.8; monitor a 115200. A revisão piloto faz somente compilação, sem simular ou gravar.

## Referências

- [pulseIn — Arduino](https://docs.arduino.cc/language-reference/en/functions/advanced-io/pulseIn/)
- [NewPing 1.9.7 usada pelo exemplo anterior](https://dl.registry.platformio.org/download/teckel12/library/NewPing/1.9.7/NewPing-1.9.7.tar.gz)
- [PubSubClient 2.8 — API](https://github.com/knolleary/pubsubclient/blob/v2.8/src/PubSubClient.h)
- [PubSubClient — conexão síncrona](https://github.com/knolleary/pubsubclient/blob/v2.8/src/PubSubClient.cpp)
- [Rede e MAC no Wokwi](https://docs.wokwi.com/guides/esp32-wifi)
