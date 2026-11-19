# Só decidir — ESP32-C3 Super Mini Plus (com motor de vibração)

**Placa do professor, fora do conteúdo da aula.** É o firmware de
[`../device-local-devkit/`](../device-local-devkit/README.md) na montagem da ESP32-C3 Super Mini
Plus com motor de vibração: decide na placa e acende a saída, sem Wi-Fi, sem broker, sem n8n.

Os pinos, o LED RGB, o buzzer, o motor, o modelo e o build são os da
[versão que publica](../device-MQTT-esp32c3/README.md). Sem a rede, não há `MQTT_CLIENT_ID` nem
tópico; o Serial mostra a janela, a predição e o tempo da inferência, como na versão DevKit.

Flash / RAM: 21,2% / 4,9% (a versão que publica: 58,5% / 12,5%).

**Wokwi:** troque `#define MPU_TYPE` para `MPU6050` e comente
`mpu.calibrateAccelGyro(&calib);`. O LED amarelo no GPIO 0 acende no lugar do motor de vibração.
