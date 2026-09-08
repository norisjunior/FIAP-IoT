# app-0-mic-SOM — o microfone produz números

Etapa 0. A versão mais curta possível: liga o INMP441 e imprime o valor que sai dele.

Não tem cabeçalho próprio, não tem janela, não tem feature. É o equivalente do
`analogRead()` do KY-038, na primeira aula de sensor analógico.

## Pinos

| INMP441 | ESP32 |
|---|---|
| VDD | 3V3 (**nunca** 5V) |
| GND | GND |
| L/R | GND |
| SCK | GPIO 26 |
| WS | GPIO 25 |
| SD | GPIO 33 |

## O código, inteiro

```cpp
#include <Arduino.h>
#include <I2S.h>

void setup() {
  Serial.begin(115200);

  I2S.setAllPins(26, 25, 33, -1, 33);      // SCK, WS, SD
  I2S.begin(I2S_PHILIPS_MODE, 16000, 32);  // 16000 amostras/s, 32 bits
}

void loop() {
  int som = I2S.read();

  Serial.printf(">som:%d\n", som);
}
```

`<I2S.h>` já vem no core do ESP32 — não precisa instalar nada.

## Uso

```bash
pio run -t upload
```

Teleplot a 115200. Fale, assobie, bata palma. Sai número.

## Três perguntas que este código responde

**1. Por que precisa das duas linhas de `setup`, se o KY-038 não precisava de nenhuma?**

Porque o INMP441 é **escravo**: ele não tem relógio. Só solta dado enquanto o ESP32 gera o
clock. Antes de existir uma amostra, alguém tem que dizer a que taxa e com quantos bits.

No KY-038 a tensão está sempre no pino, e o `analogRead()` só olha quando quer.

**2. Por que os números são tão grandes?**

O INMP441 entrega 24 bits alinhados à esquerda dentro de um espaço de 32 bits. O valor cru
chega na casa dos milhões. Na etapa 1 a gente encolhe isso — e escolher **quanto** encolher
vira assunto (é o mesmo papel do 2G/4G/8G/16G no acelerômetro, Aula 12).

**3. Não existe uma biblioteca do INMP441, como a do DHT22?**

Não, e por um bom motivo: **o INMP441 não tem registradores.** Não tem endereço, não aceita
comando, você não escreve nada nele. A única configuração do chip é o pino **L/R**, e ela se
faz com um fio.

Biblioteca existe para encapsular protocolo próprio — DHT22, DS18B20 e MPU6050 têm; o INMP441
não tem. Uma "biblioteca do INMP441" seria, na verdade, uma biblioteca do periférico I2S do
ESP32 — e é exatamente isso que o `<I2S.h>` é.

> A existência de biblioteca diz algo sobre o sensor: protocolo próprio → tem lib; barramento
> padrão sem registradores → não tem, e não faz falta.

## Onde isso trava

`I2S.read()` entrega **uma amostra por chamada**, e cada `Serial.printf` custa muito mais tempo
do que os 62 µs que separam duas amostras a 16 kHz. O microfone continua produzindo enquanto o
ESP32 imprime, e o que não é lido a tempo se perde.

Resultado no gráfico: **o sinal sai picotado**, com buracos. Dá para ver que tem som, mas não
dá para ver a onda.

É esse defeito que a etapa 1 conserta, lendo em bloco.

## Problemas

| Sintoma | Causa provável |
|---|---|
| só zeros ou lixo constante | canal errado — o L/R deste módulo pode cair no canal oposto |
| aparecem `-1` no meio | é o buffer vazio; a etapa 1 resolve |
| nada acontece | L/R não está no GND, ou SD/WS/SCK trocados |

Sem Wokwi: não existe peça INMP441 no simulador.
