# Firmware SmartBag

Abra esta pasta no PlatformIO. O programa é `src/app19-SmartBag.ino`.

```powershell
pio run
pio device monitor -b 115200
```

No Wokwi, use `diagram.json` e `wokwi.toml` desta pasta. Configure o broker no início do `.ino` e habilite o gateway local quando usar `host.wokwi.internal`.

Botões ligados ao GND com `INPUT_PULLUP`: GPIO 27 inicia/para a coleta; GPIO 26 avança a situação com a coleta parada. Ao voltar da última à primeira, incrementa `rodada`. O baseline é calculado ao iniciar a primeira coleta dessa rodada. Parar/retomar não incrementa rodada. LED aceso = coleta em andamento. O Serial apenas exibe dados.

Ordem do código: configurações → setup/loop → baseline → conexão → publicação. Sensores continuam em funções nos headers `ESP32Sensors*.hpp`.

Veja o [README](../README.md) e o [guia de coleta](../Guia-NexoLog.md).

Como no app17-7, `rodada = 1` na partida e aumenta somente ao completar a sequência de situações. Não há `novaViagem()`, sessão aleatória ou contador separado de viagem.

O JSON leva somente as seis features, device, rodada e situacao. `coletando` é apenas estado interno: controla o LED e habilita a publicação a cada segundo. Máximos começam em NAN e são atualizados por fmaxf; sem leitura, chegam como null.
