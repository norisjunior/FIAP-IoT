# Plano — trilha IMU (app17 → app18 → app19)

## Estado em 2026-11-10

- **app17:** o professor validou todos os códigos do app17. O app17-8 rodou com o motor.
- **app18 e app19:** alterados em 2026-11-09 (±8 g, comentários, app19 de volta ao padrão
  físico). Todos compilam; validação no hardware ainda não registrada.
- **Dados e modelos:** os datasets e os modelos atuais (`.pkl`, `.hpp`) são de **±16 g** —
  ver a primeira pendência.

## Decisões de 2026-11-09 e 2026-11-10

| Decisão | Por quê |
|---|---|
| `setAccelRange(8)` depois do segundo `init()`, em todos os apps e guias | o `init()` reseta para ±16 g; até 2026-11-09 a trilha inteira rodou em ±16 g sem saber. Para as 4 classes, ±8 g basta e tem o dobro de resolução |
| sem `setAccelLPF(41)`/`setGyroLPF(42)` | o `init()` já grava esses valores; explicá-los toma tempo de aula e não é o objetivo |
| comentário só com título (app17-3 a app17-11, app18, app19) | o código é o que o aluno lê na tela; a explicação é da fala do professor |
| sem NTP; `ts_epoch_ms` virou `ts_ms = millis()` | não sincronizar relógio pela internet; o tempo do ESP32 basta, e quem quiser alinha no notebook |
| app19: calibração ativa, rede `(B)` e `MPU6500` | desfaz o commit `bd25eba`, que tinha deixado o Wokwi como padrão |
| app17-8 (motor DC), app17-10 (C6), app17-11 (C3 + servo) | rigs de demonstração e variantes de placa, todos alimentando o mesmo treino |
| LED RGB com uma cor por estado | padrão definido pelo professor para as placas com RGB |
| XIAO nRF52840 Sense fica fora do app17 | vai para o app21 (`app-Fumaca-SensorFusion`), em outro chat |

## Pendências

1. **Recoletar a ±8 g e retreinar.** Inclui o `.hpp` do motor no app18/EdgeAI. Em `g`, sem
   saturação, as features quase não mudam, mas o dataset novo é o que vale.
2. **Slides.** `FIAP-IoT-develop/app17-IMU/INSTRUÇÕES PARA POWERPOINT - Aula 17 v2 IMU.md`
   ainda fala de `ts_epoch_ms` e NTP (linhas ~135, 335, 610–635) e lista o filtro
   `setAccelLPF(41) + setGyroLPF(42)` na tabela de paridade (~653). O guia do app18/CloudAI no
   eval já foi atualizado: o quadro "Sem NTP" saiu e a tabela do payload usa `ts_ms`.
3. **Numeração.** Sem app17-9 desde a exclusão; decidir se app17-10 e app17-11 descem um número.
4. **`MQTT_CLIENT_ID` igual em todas as versões.** Dois rigs ligados ao mesmo tempo se derrubam
   no broker; trocar o ID de um deles nesse caso.
5. **XIAO nRF52840 (app21).** Perguntas em aberto, para o chat do app21: envio sem Wi-Fi (USB +
   ponte Python ou BLE), botão único D1 na placa de expansão, IMU da Sense ou MPU6500 no Grove.
   Antes, confirmar se a placa é Sense ou Sense Plus (a board do PlatformIO muda) e usar a mesma
   linha `platform_packages` do Smart Ring.

## Referências

- Tabela "⚖ paridade" nos `CONSTRUIR-O-FIRMWARE.md` do app18 (EdgeAI e CloudAI) e do
  app19 (Chatbot-EdgeAI).
- No develop, `app17-IMU/`: `INSTRUÇÕES PARA POWERPOINT - Aula 17 v2 IMU.md` e
  `DECISÕES - app18 CloudAI e EdgeAI.md`.
