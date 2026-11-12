# app17-IMU-GerarDataset-Sinais

Trilha IMU da disciplina de IoT (FIAP): do acelerômetro (MPU6050/MPU6500) a um dataset de
features por janela no InfluxDB, que o app18 (inferência) e o app19 (LLM) consomem.
Estado, decisões e pendências: [PLANO.md](PLANO.md) — leia antes de mexer, atualize ao terminar.

## Pastas

| Pasta | O que é | Papel |
|---|---|---|
| `app17-0-Plot` | scripts Python que desenham a Serial em tempo real | apoio |
| `app17-1-ModeloMPU` | descobre qual chip está na placa (WHO_AM_I) | roteiro |
| `app17-2-ColetaVisual` | lê os 3 eixos com a FastIMU | roteiro |
| `app17-3-JanelaDadosAcc` | janela de 100 amostras a 100 Hz, impressa na Serial | roteiro |
| `app17-4-JanelaFeaturesAcc` | as 8 features da janela na Serial (+ `colab/`, `coleta_professor/`) | roteiro |
| `app17-5-AccRawSerial` | sinal bruto pela Serial + `coletor_raw.py` | referência, fora do roteiro |
| `app17-6-BinaryAccFeaturesInflux` | binário normal/anomalia, features `rms_*`, MQTT → InfluxDB | referência, fora do roteiro |
| `app17-7-MultiClassAccFeaturesInflux` | **o coletor da aula**: 4 classes, MQTT → InfluxDB, Colab de treino | roteiro |
| `app17-8-MultiClassAccFeaturesInfluxDevkitv1MOTOR` | o app17-7 no rig do professor: mini motor com hélice, ponte H | placa pronta, fora da aula |
| `app17-10-MultiClassAccFeaturesInfluxESP32C6` | o app17-7 na ESP32-C6-DevKitC-1 (pioarduino) | placa pronta, fora da aula |
| `app17-11-MultiClassAccFeaturesInfluxESP32C3SERVO` | o app17-7 no ESP32-C3 Super Mini Plus com microservo | placa pronta, fora da aula |

"Placa pronta" = montada pelo professor para o aluno que não conseguir montar a própria. Não
entra nos slides: o objetivo da aula é o protocolo de coleta do dataset, não a placa.

Não existe app17-9: o binário para ESP32-C6 foi excluído pelo professor em 2026-11-10.

## Regras da trilha

- **Sensor:** FastIMU 1.3.0. Setup sempre `init → calibrateAccelGyro → init(calib) →
  setAccelRange(8)`. O `init()` reseta o chip e volta a ±16 g; qualquer configuração antes do
  segundo `init()` se perde.
- **Sem `setAccelLPF`/`setGyroLPF`:** o `init()` já grava os padrões, e filtro não é objetivo
  da aula.
- **Comentário só com título.** Uma linha dizendo o que faz; a explicação é da aula. O aviso do
  Wokwi fica curto no título: `// Calibração (no Wokwi, comente esta linha)`.
- **Sem NTP.** O carimbo é `ts_ms = millis()` (ms desde o boot). O horário do ponto no InfluxDB
  é o da chegada no Node-RED; alinhar com o relógio real é customização no notebook.
- **Toda versão do coletor multiclasse publica igual:** tópico `FIAPIoT/motor/multiclasse`,
  measurement `vibracao_multiclasse`, mesmo payload, mesmo fluxo do Node-RED (mesmos IDs de
  nó). Os dados de todas entram no mesmo treino.
- **LED RGB (C3 e C6):** branco esperando; na coleta, verde `operando`, amarelo
  `inclinado_frente`, roxo `inclinado_tras`, vermelho `anomalia` (`COR_ESPERA`, `COR_CLASSE`).
- **Wokwi:** só simula o MPU6050 e não tem a FIFO do MPU — comentar a calibração, senão
  `IntegerDivideByZero`. Rede: bloco `(A)` Wokwi, bloco `(B)` físico (padrão).
- **PlatformIO:** `espressif32@6.12.0`. O C6 usa pioarduino 53.03.13 isolado em
  `platforms-pioarduino`. O C3 Super Mini usa `ARDUINO_USB_CDC_ON_BOOT=1`.
- **Placa ou rig novo = pasta nova**, clonada do app17-7 (ou do app17-8), com a tabela
  "O que muda" no README e o app listado em "Outras versões deste app" do app17-7.
