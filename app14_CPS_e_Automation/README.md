# App14 — NexoLog no Node-RED

Abra [app14_NexoLog](app14_NexoLog) no PlatformIO. Compile e inicie o Wokwi ou grave no ESP32.

O firmware lê DHT22, ultrassônico e MPU a cada 2,5 segundos e publica JSON. Não recebe comandos. O LED fica apagado, reservado para a próxima etapa.

## Aula

1. Importe [dashboard.json](Plataformas_config/NodeRED/dashboard.json), configure o broker e faça Deploy.
2. Acompanhe MQTT-in, JSON e Debug.
3. Abra `separarDadosSensores` e siga suas quatro saídas até os gauges e o gráfico.
4. Use `Simular: Normal` e `Simular: Tampa aberta`. Depois altere a distância no Wokwi de 10 para 40 cm.
5. Observe o Switch e o estado da entrega. Acesse `http://localhost:1880/ui/`.

Depois, acrescente [InfluxDB](Plataformas_config/NodeRED/Fluxo_2_envio_InfluxDB.json) e [n8n](Plataformas_config/n8n/fluxo_mqtt.json). No n8n, configure credenciais e Chat ID. Os eventos saem apenas na mudança de estado.

## MPU com FastIMU

Em `src/ESP32SensorsAccel.hpp`, selecione `MPU_TYPE`: `MPU6050` para o diagrama Wokwi ou `MPU6500` para essa placa física. Endereço I2C: `0x68` (AD0 em GND). Sem calibração automática nesta aula.

FastIMU retorna aceleração em g. O módulo converte para m/s² para preservar o payload. A movimentação é a magnitude da aceleração menos a gravidade, também em m/s²: caixa parada fica próxima de 0.

[Montagem e configuração comuns](Guia-NexoLog.md)
