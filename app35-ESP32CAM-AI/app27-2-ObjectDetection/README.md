# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Detecção de objetos na ESP32-CAM com Edge Impulse

O modelo que você treinou no Edge Impulse roda **dentro da ESP32-CAM**: a câmera
captura, o modelo diz que objeto está na imagem e onde, e **cada objeto detectado acende
o seu LED**.

Detectar é só a metade do trabalho. A outra metade é decidir **o que fazer** com a
detecção: o modelo dá a nota, e o seu código decide.

Você monta o código, mas **não do zero**: ele é o exemplo `esp32_camera` que vem dentro
da biblioteca que o Edge Impulse gera para o seu projeto. Você copia o exemplo e faz
**dois ajustes**:

1. o modelo da câmera (AI-Thinker);
2. os LEDs, um por classe.

## Hardware

- ESP32-CAM (AI-Thinker)
- Adaptador FTDI (USB-serial), ligado como no app de coleta de imagens
- 3 LEDs: vermelho, verde e azul
- 3 resistores de 220 Ω
- Protoboard e jumpers

## Ligação dos LEDs

| LED | GPIO | Classe (modelo do professor) |
|---|---|---|
| Vermelho | IO13 | `alicatecorte` |
| Verde | IO14 | `cubomagico` |
| Azul | IO15 | `earphonecase` |

Cada LED: **GPIO → resistor de 220 Ω → perna longa do LED (+)**; a **perna curta (−)
vai ao GND**.

```
ESP32-CAM
  IO13 ──[ 220 Ω ]──(+ LED vermelho −)── GND
  IO14 ──[ 220 Ω ]──(+ LED verde    −)── GND
  IO15 ──[ 220 Ω ]──(+ LED azul     −)── GND
```

Por que esses pinos: a câmera ocupa quase todos os GPIOs da ESP32-CAM. IO13, IO14 e
IO15 ficam no conector da placa e estão livres (são do cartão SD, que não usamos).
Evite:

| Pino | Por quê |
|---|---|
| IO4 | é o LED de flash da câmera |
| IO12 | se estiver em nível alto quando a placa liga, ela não inicia |
| IO0 | é usado para gravar e é o clock da câmera |
| IO16 | é usado pela PSRAM |
| U0R, U0T | são a serial com o FTDI |

Ao ligar a placa, os LEDs podem piscar rapidamente. É normal: IO14 e IO15 oscilam durante
a inicialização do ESP32.

## A pasta `lib/`

Ela vem com o `.zip` da biblioteca do modelo do professor,
`ei-fiap-iot-objrec-arduino-1.0.1-impulse-#2.zip`, que detecta alicate de corte, cubo
mágico e case de fone de ouvido. Para o seu projeto, **use o `.zip` do seu modelo no
lugar dele** (passos 1 e 2).

## Antes de começar

Você já tem, no Edge Impulse, um projeto treinado com:

| Bloco do impulse | Valor |
|---|---|
| Image data | 48 x 48, resize mode **squash** |
| Processing block | Image, color depth **Grayscale** |
| Learning block | Object Detection (Images) |

As imagens do treino vieram da própria ESP32-CAM, pelo app de coleta (240 x 240).

## Roteiro

### 1. Baixar a biblioteca

No Edge Impulse: **Deployment** → **Arduino library** → **Build**. Será baixado um
`.zip`, por exemplo `ei-fiap-iot-objrec-arduino-1.0.1-impulse-#2.zip`.

### 2. Descompactar dentro de `lib/`

Apague o `.zip` do professor, coloque o seu em `lib/` e descompacte-o ali mesmo. O
PlatformIO não lê o `.zip`: tem que descompactar. A estrutura deve ficar assim:

```
app27-2-ObjectDetection/
├── lib/
│   ├── ei-<seu-projeto>-arduino-<versão>.zip
│   └── <SEU_PROJETO>_inferencing/
│       ├── examples/
│       ├── src/
│       └── library.properties
├── src/
└── platformio.ini
```

**ATENÇÃO:** o nome da pasta é o nome do **seu** projeto no Edge Impulse:
`<SEU_PROJETO>_inferencing`.

### 3. Conferir o `platformio.ini`

```ini
[env:esp32cam]
platform = espressif32@6.12.0
framework = arduino
board = esp32cam
monitor_speed = 115200
```

O `monitor_speed = 115200` é porque o exemplo imprime a 115200. Sem essa linha, o
Monitor Serial abre a 9600 e mostra caracteres estranhos.

### 4. Copiar o exemplo para `src/`

Copie

```
lib/<SEU_PROJETO>_inferencing/examples/esp32/esp32_camera/esp32_camera.ino
```

para

```
src/objectrecognition.ino
```

O exemplo já vem com o `#include` certo, com o nome do seu projeto. No do professor:

```cpp
#include <FIAP-IoT-ObjRec_inferencing.h>
```

### 5. Ajuste 1: o modelo da câmera

O exemplo vem configurado para a placa ESP-EYE. A nossa é a ESP32-CAM AI-Thinker, com
outros pinos. Perto da linha 35, troque o comentário de lugar.

Como vem:

```cpp
#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_AI_THINKER // Has PSRAM
```

Como deve ficar:

```cpp
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
```

Com o modelo errado, o código **compila**, mas a câmera não inicia (veja
[Problemas comuns](#problemas-comuns)).

### 6. Compilar e gravar

A primeira compilação leva cerca de **3 minutos**: o PlatformIO compila a biblioteca
inteira do Edge Impulse. As seguintes levam menos de 1 minuto.

Dois avisos são **esperados** e podem ser ignorados:

- `"PWDN_GPIO_NUM" redefined` (e os outros pinos): aparecem logo depois de
  `Converting objectrecognition.ino`. Nessa conversão de `.ino` para `.cpp`, o PlatformIO
  lê todos os `#define` sem avaliar os `#if`. Na compilação de verdade, só o bloco
  `CAMERA_MODEL_AI_THINKER` vale.
- `'EI_CLASSFIER_OBJECT_DETECTION_COUNT' is used for the guaranteed minimum...`: aviso
  da própria biblioteca.

O que importa é o final: `[SUCCESS]`.

Para gravar, como no app de coleta: **IO0 ligado ao GND**, grave, tire o jumper e aperte
o **RESET**.

### 7. Conferir a detecção no Monitor Serial

```
pio device monitor
```

Saída esperada (valores ilustrativos):

```
Edge Impulse Inferencing Demo
Camera initialized

Starting continious inference in 2 seconds...
Predictions (DSP: 4 ms., Classification: 95 ms., Anomaly: 0 ms.):
Object detection bounding boxes:
  cubomagico (0.910156) [ x: 16, y: 24, width: 8, height: 8 ]
```

| Campo | Significado |
|---|---|
| `cubomagico` | a classe detectada (os nomes que você deu no labeling) |
| `0.910156` | a confiança do modelo, de 0 a 1 |
| `x`, `y` | a posição do objeto **na imagem de 48 x 48** que o modelo recebe |
| `width`, `height` | o tamanho da região, também em pixels de 48 x 48 |

Duas observações sobre essa saída:

- **Nada aparece abaixo de 0,5.** O limiar vem do treino e está na biblioteca
  (`EI_CLASSIFIER_OBJECT_DETECTION_THRESHOLD` em `model-parameters/model_metadata.h`).
- **Imagem só com o fundo = lista vazia.** É o comportamento certo: não há objeto.

O nosso modelo é do tipo **FOMO**: ele divide a imagem de 48 x 48 em uma grade de
6 x 6 células de 8 x 8 pixels e diz em quais células está cada objeto. Por isso `x` e
`y` andam de 8 em 8, e a região tem 8 de lado (ou mais, quando o objeto ocupa células
vizinhas). O FOMO responde **o que** e **onde** (o centro do objeto), não o contorno
exato.

**Confira antes de seguir:** cada objeto aparece com o nome certo, e só o fundo dá a
lista vazia. Se a detecção não funciona aqui, os LEDs não vão resolver: volte ao treino.

### 8. Ajuste 2: o que fazer depois da detecção (LEDs)

Até aqui, a ESP32-CAM só detecta e imprime. Agora ela age: cada classe acende o seu LED.
São três acréscimos no `objectrecognition.ino`.

**a) Pinos e limiar.** Logo depois das constantes da câmera:

```cpp
/* Constant defines -------------------------------------------------------- */
#define EI_CAMERA_RAW_FRAME_BUFFER_COLS           320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS           240
#define EI_CAMERA_FRAME_BYTE_SIZE                 3
```

acrescente:

```cpp
/* LEDs: um por classe ----------------------------------------------------- */
#define LED_R_PIN   13      // alicatecorte
#define LED_G_PIN   14      // cubomagico
#define LED_B_PIN   15      // earphonecase
#define LIMIAR      0.70    // confiança mínima para acender o LED (decisão sua)
```

**b) Configurar os pinos.** No `setup()`, logo depois de
`Serial.println("Edge Impulse Inferencing Demo");`:

```cpp
    pinMode(LED_R_PIN, OUTPUT);
    pinMode(LED_G_PIN, OUTPUT);
    pinMode(LED_B_PIN, OUTPUT);
```

**c) A decisão.** No `loop()`, procure o `for` que imprime as detecções:

```cpp
#if EI_CLASSIFIER_OBJECT_DETECTION == 1
    ei_printf("Object detection bounding boxes:\r\n");
    for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {
        ...
    }
```

Logo depois dele, antes de `// Print the prediction results (classification)`,
acrescente:

```cpp
    // Decisão: cada classe acende o seu LED
    digitalWrite(LED_R_PIN, LOW);
    digitalWrite(LED_G_PIN, LOW);
    digitalWrite(LED_B_PIN, LOW);
    for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {
        ei_impulse_result_bounding_box_t bb = result.bounding_boxes[i];
        if (bb.value < LIMIAR) {
            continue;
        }
        if (strcmp(bb.label, "alicatecorte") == 0) {
            digitalWrite(LED_R_PIN, HIGH);
        } else if (strcmp(bb.label, "cubomagico") == 0) {
            digitalWrite(LED_G_PIN, HIGH);
        } else if (strcmp(bb.label, "earphonecase") == 0) {
            digitalWrite(LED_B_PIN, HIGH);
        }
    }
```

Como a decisão funciona:

- **A cada imagem**, o código apaga os três LEDs e acende os das classes vistas. O LED
  fica aceso enquanto o objeto estiver na frente da câmera e apaga quando ele sai.
- **Dois objetos diferentes na imagem** acendem dois LEDs: o `for` passa por todas as
  detecções, não só pela melhor.
- **O `LIMIAR` é decisão sua**, não do modelo. A biblioteca já descarta o que fica
  abaixo de 0,5; o `LIMIAR` só faz diferença entre 0,5 e 1. Subir evita LED aceso por
  engano; descer deixa o LED mais sensível.
- **Os nomes no `strcmp` têm que ser exatamente os das suas classes**: copie como
  aparecem no Monitor Serial (passo 7). Troque os três nomes do exemplo pelos seus.

### 9. Testar os LEDs

Compile, grave e coloque um objeto por vez na frente da câmera:

| Na frente da câmera | LEDs |
|---|---|
| só o fundo | todos apagados |
| um objeto | só o LED da classe dele |
| dois objetos diferentes | os dois LEDs |

## Por que o exemplo funciona sem outros ajustes

O modelo não dá erro quando recebe uma imagem diferente da do treino: ele só erra. Por
isso vale conferir que a imagem chega igual:

| Etapa | No treino | Na inferência (exemplo) |
|---|---|---|
| Captura | 240 x 240, JPEG (app de coleta) | 320 x 240 (QVGA), JPEG |
| Recorte | nenhum: a imagem já é quadrada | corta o **quadrado central** de 240 x 240 |
| Redução | 48 x 48, squash | 48 x 48 |
| Cor | Grayscale | a biblioteca converte para cinza, com a mesma conta do treino |

O quadrado de 240 x 240 da coleta e o quadrado central do QVGA cobrem a mesma região
do sensor. O modelo recebe o objeto do mesmo tamanho e na mesma posição em que foi
treinado.

O que **quebra** essa igualdade: mudar a posição da câmera, o fundo ou a iluminação em
relação às fotos do treino. Por isso o app de coleta pede um local fixo e um fundo
preparado.

## Problemas comuns

| Sintoma | Causa provável |
|---|---|
| `Camera init failed with error 0x...` | modelo da câmera errado (passo 5) ou cabo flat da câmera solto |
| `fatal error: ..._inferencing.h: No such file or directory` | `.zip` não descompactado em `lib/` (passo 2) |
| Caracteres estranhos no Monitor Serial | falta `monitor_speed = 115200` (passo 3) |
| `Brownout detector was triggered` e reinício | alimentação fraca: use 5 V no FTDI e um cabo USB curto |
| Detecta a classe errada ou nada | câmera, fundo ou luz diferentes do treino (seção anterior) |
| Detecta no Monitor Serial, mas o LED não acende | LED invertido (a perna longa vai do lado do resistor), nome diferente no `strcmp` ou `LIMIAR` alto demais |
| A placa não inicia depois de ligar os LEDs | LED no IO12 ou no IO0: use IO13, IO14 e IO15 |
