# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## **Aplicação 25 - 1 - Coleta de dados para detecção de gestos usando ESP32 e MPU6050**

Sistema de coleta de dados de movimento usando ESP32 e sensor MPU6050 para treinamento de modelos de Machine Learning no Edge Impulse.

## Hardware Necessário

- ESP32 DevKit
- MPU6050 (acelerômetro e giroscópio)
- 3 LEDs (vermelho, verde, azul)

## Conexões

**MPU6050:**
- VCC → 3V3
- GND → GND
- SDA → GPIO 22
- SCL → GPIO 23
- AD0 → GND (endereço I2C 0x68)

**LEDs:**
- LED Vermelho → GPIO 21
- LED Verde → GPIO 19
- LED Azul → GPIO 18

## Configuração

O sistema coleta dados a **50Hz** (20ms entre amostras) com 6 eixos:
- Acelerômetro: x, y, z (m/s²)
- Giroscópio: x, y, z (rad/s)

## Como Usar

1. **Compile e faça upload do código**

2. **Abra o Monitor Serial (apenas para constatar que a aplicação está UP)**
   - F1 -> PlatformIO New Terminal

   ```bash
   pio device monitor -b 115200
   ```
5. **Feche o monitor serial**
   - Assim, a porta COM estará liberada para uso pela ferramenta de encaminhamento de dados do Edge Impulse (*edge-impulse-data-forwarder*)


4. **Capture dados no Edge Impulse**
   - Acesse [studio.edgeimpulse.com](https://studio.edgeimpulse.com)
   - Vá em "Data acquisition"
   - Conecte via Serial
   - Configure: 50Hz, 1500ms de janela
   - Realize o gesto e clique em "Start sampling"

## Formato de Saída

```
ax,ay,az,gx,gy,gz
9.234567,-1.456789,0.123456,0.012345,-0.023456,0.034567
```

Dados separados por vírgula, 6 casas decimais.

## Simulação

Compatível com Wokwi Simulator - use o arquivo `diagram.json` incluído.

## Bibliotecas

- Adafruit MPU6050
- Adafruit Unified Sensor
- Adafruit BusIO

(Instaladas automaticamente pelo PlatformIO)
