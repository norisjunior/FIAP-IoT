# App14 — NexoLog no Node-RED

Abra [app14_NexoLog](app14_NexoLog_PUB_only) no PlatformIO. Compile e inicie o Wokwi ou grave no ESP32.

O firmware publica JSON a cada 1 segundo, com um relógio por sensor: DHT22 a 2,1 s (cache), MPU a 50 ms guardando o pico do segundo, ultrassônico junto do envio. Não recebe comandos. O LED fica apagado, reservado para a próxima etapa.

## Aula

1. Importe [dashboard.json](Plataformas_config/NodeRED/dashboard.json), configure o broker e faça Deploy.
2. Acompanhe MQTT-in, JSON e Debug.
3. Abra `separarDadosSensores` e siga suas quatro saídas até os widgets e o gráfico.
4. Altere a distância no Wokwi de 10 para 40 cm e volte para 10.
5. Acesse `http://localhost:1880/ui/`. O Node-RED aqui só mostra: quem decide é o n8n.

Para montar em vez de importar: [firmware](app14_NexoLog_PUB_only/CONSTRUIR-O-FIRMWARE.md), [Node-RED](Plataformas_config/NodeRED/CONSTRUIR-O-FLUXO.md), [n8n](Plataformas_config/n8n/CONSTRUIR-O-FLUXO.md).

Depois, acrescente [InfluxDB](Plataformas_config/NodeRED/Fluxo_2_envio_InfluxDB.json) e [n8n](Plataformas_config/n8n/fluxo_mqtt.json). O n8n assina o mesmo `dados` do ESP32 e decide sozinho: um Switch com os quatro limiares, quatro nós montando o texto e um único Telegram.

## MPU com FastIMU

Em `src/ESP32SensorsAccel.hpp`, selecione `MPU_TYPE`: `MPU6050` para o diagrama Wokwi ou `MPU6500` para essa placa física. Endereço I2C: `0x68` (AD0 em GND). Sem calibração automática nesta aula.

FastIMU retorna aceleração em g. O módulo converte para m/s² para preservar o payload. A movimentação é a magnitude da aceleração menos a gravidade, também em m/s²: caixa parada fica próxima de 0. O campo publicado é o pico do último segundo, com amostragem a 50 ms.

[Montagem e configuração comuns](Guia-NexoLog.md)
