# App04 — ESP32 transmite JSON ao servidor Python

Exemplo piloto de alinhamento das bibliotecas e da conexão WiFi. A estrutura do código continua com funções simples, configurações no início do `.ino` e módulos `ESP32Sensors` em `.hpp`.

## Sensores e montagem

- DHT: GPIO 27; `DHT_MODEL` pode ser `DHT11` ou `DHT22`.
- MPU: SDA no GPIO 18 e SCL no GPIO 19.
- LED: GPIO 26.
- O `diagram.json` foi preservado.

No início de `src/iot-aula02-app04.ino`, escolha o modelo:

```cpp
#define MPU_TYPE MPU6050  // Wokwi
```

Para um kit com MPU6500, troque somente `MPU6050` por `MPU6500`. A biblioteca continua sendo FastIMU 1.3.0. O arquivo `iot-aula02-app04-flixperiph.txt` é uma alternativa histórica, não compilada; o exemplo ativo usa FastIMU. Esse arquivo histórico também tem pino de DHT diferente: siga o `.ino` ativo e o diagrama deste projeto.

A chamada continua simples:

```cpp
ESP32Sensors::Accel::ACCEL motor = ESP32Sensors::Accel::medirAccel();
```

A struct tem `x`, `y` e `z` em m/s². A FastIMU entrega em g; a conversão fica no header, preservando os valores e as chaves enviados ao servidor. A configuração mantém ±16 g, filtro próximo de 21 Hz e taxa interna de 1 kHz. O único endereço de variável (`&leitura`) fica no header porque é exigido por `getAccel` da biblioteca. Não há alocação dinâmica adicionada ao exemplo.

## WiFi e transmissão

No Wokwi: `WIFI_SSID = "Wokwi-GUEST"` e senha vazia. No kit: informe a rede e senha do laboratório. `WiFi.begin(WIFI_SSID, WIFI_SENHA)` deixa a escolha do canal automática nos dois casos. `host` continua sendo `host.wokwi.internal`; no kit, informe o IP do computador com o servidor Python. A porta continua 5000.

`conectarWiFi()` mantém o controle de intervalo do app14, com tempos em `uint32_t` e canal automático. Ao iniciar uma conexão, aguarda `WL_CONNECTED` com `while` e `delay(500)`. Somente então imprime `[WiFi] Conectado` e o IP obtido; a mesma função mostra o novo IP após uma reconexão. Não imprime mensagens durante a tentativa.

O loop somente verifica `WiFi.status() != WL_CONNECTED` e chama `conectarWiFi()`. Não há flag nem impressão do IP no loop. Enquanto aguarda a conexão, o programa permanece no `while`, conforme a simplificação didática escolhida.

A primeira tentativa ocorre quando o intervalo inicial de 5 s tiver passado, sem acrescentar `delay` no setup. O app04 mantém seu intervalo de 5 s entre ciclos; leituras/transmissões ficam pausadas durante a espera de uma conexão iniciada. A conexão TCP continua condicionada ao WiFi conectado. Não há armazenamento/reenvio offline.

## Compilação

```powershell
pio run
```

Monitor Serial: 115200. Dependências fixadas em `platformio.ini`. A revisão piloto compila o modelo padrão MPU6050 e também confere a seleção MPU6500; isso não substitui verificação dos sensores no hardware/simulador. Não foi executada simulação nem gravação.

## APIs conferidas

- [FastIMU 1.3.0: MPU6050](https://github.com/LiquidCGS/FastIMU/blob/1.3.0/src/sensors/F_MPU6050.hpp)
- [FastIMU 1.3.0: MPU6500](https://github.com/LiquidCGS/FastIMU/blob/1.3.0/src/sensors/F_MPU6500.hpp)
- [Implementação e unidades da FastIMU](https://github.com/LiquidCGS/FastIMU/blob/1.3.0/src/sensors/F_MPU6050.cpp)
- [DHT 1.4.6](https://github.com/adafruit/DHT-sensor-library/blob/1.4.6/DHT.h)
- [WiFi do Arduino-ESP32 2.0.17](https://github.com/espressif/arduino-esp32/blob/2.0.17/libraries/WiFi/src/WiFiSTA.h)
