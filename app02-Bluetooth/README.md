# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 02 - Passador de Slides via Bluetooth

Esta aplicação transforma o ESP32 em um passador de slides sem fio. Com a biblioteca [ESP32 BLE Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard), o ESP32 se apresenta ao computador como um teclado Bluetooth: um botão envia a tecla "seta para a direita" (avança o slide) e o outro, a "seta para a esquerda" (volta o slide). Um LED indica se o passador está conectado.

### Sensores e Atuadores

**Entradas:**
- Botão verde "AVANÇAR" - Pino GPIO 18 (outro terminal no GND; usa o pull-up interno)
- Botão amarelo "RETROCEDER" - Pino GPIO 19 (outro terminal no GND; usa o pull-up interno)

**Atuadores:**
- LED vermelho com resistor de 220 Ω - Pino GPIO 23 (aceso = conectado ao computador)

### Funcionamento

1. Ao ligar, o ESP32 ativa o Bluetooth e aparece na lista de dispositivos do computador com o nome definido em `DEVICE_NAME` (padrão: `PassadorSlides`).
2. Enquanto não houver conexão, o LED fica apagado.
3. Conectado, o LED acende e o `loop()` verifica os botões:
   - AVANÇAR pressionado: envia `KEY_RIGHT_ARROW` e escreve "AVANÇAR" no Monitor Serial;
   - RETROCEDER pressionado: envia `KEY_LEFT_ARROW` e escreve "RETROCEDER".
4. Depois de cada envio, espera 300 ms, para que um toque passe um slide só.

### Como Usar

1. Troque o nome em `#define DEVICE_NAME "PassadorSlides"` por um nome seu. Numa sala com vários passadores, nomes iguais confundem o pareamento.
2. Compile e grave pelo PlatformIO (Upload).
3. No computador, abra as configurações de Bluetooth e pareie com o nome escolhido. O LED acende.
4. Abra uma apresentação e use os botões. O Monitor Serial (115200) mostra AVANÇAR e RETROCEDER a cada toque.

### Observações

- Plataforma fixada em `espressif32@6.12.0` (Arduino Core 2.0.17): a biblioteca BLE Keyboard não compila no Core 3.x.
- O passador funciona como um teclado: a tecla vai para a janela em foco. Deixe a apresentação em primeiro plano.
- No macOS, o README da biblioteca avisa que a conexão pode ser instável para algumas pessoas.
- As ligações do circuito estão em `diagram.json` (Wokwi).
