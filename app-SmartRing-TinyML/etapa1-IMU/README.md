# Etapa 1 — Hardware e ambiente

Comprovar que a XIAO nRF52840 Sense Plus compila, grava e conversa pela USB no PlatformIO, e que
a IMU entrega os seis eixos. Sem TinyML e sem BLE nesta etapa.

**Critério de conclusão:** ver os seis eixos da IMU no Serial Monitor, com valores coerentes ao
movimentar a placa, a uma taxa estável e medida.

A etapa tem dois testes, nesta ordem — o primeiro isola ambiente, upload e serial, para que um
problema ali não seja confundido com problema da IMU.

## Teste 1.1 — placa viva

O `src/main.cpp` pisca o LED vermelho e imprime o tempo desde o boot uma vez por segundo.

1. Abra **esta pasta** (`etapa1-IMU`) no VS Code com *File → Open Folder*, para o PlatformIO
   reconhecer o projeto.
2. **Build** (✓ na barra inferior). O primeiro build baixa a plataforma da Seeed, o compilador ARM
   e o framework — leva alguns minutos.
3. Conecte a placa com um cabo USB-C **de dados** e confira a porta no Gerenciador de Dispositivos
   (Portas COM).
4. **Upload** (→).
5. **Monitor** (ícone de tomada).

**Passa se:**

- o build termina com `SUCCESS` e mostra o uso de RAM e Flash;
- o upload termina com `Device programmed.`;
- o LED vermelho pisca (1 s aceso, 1 s apagado);
- o monitor mostra `XIAO nRF52840 Sense Plus: placa viva` (se o monitor abrir a tempo) e depois
  uma linha por segundo, com `t` subindo cerca de 1000 a cada linha.

## Teste 1.2 — IMU com 6 eixos

Próximo passo, depois que o 1.1 passar.

## Particularidades da placa

| Situação | O que fazer |
|---|---|
| O upload não acha a placa, ou a porta COM sumiu | Dois cliques rápidos no botão de **reset** (o minúsculo, ao lado do USB-C). A placa entra no bootloader, aparece uma unidade USB e uma porta COM nova. Repita o upload. |
| Primeiro upload sobre o firmware de fábrica | Pode precisar dos dois cliques no reset — o firmware de fábrica não obedece necessariamente ao pedido de reinício do PlatformIO. |
| A COM muda de número durante o upload | Normal: o PlatformIO abre a porta a 1200 bps, a placa reinicia no bootloader (outra porta) e o `adafruit-nrfutil` grava por ela. |
| No Windows a placa aparece como "XIAO nRF52840", sem "Sense" | Normal com esta board (PID 0x8044). |
| `Serial` não compila | Falta `#include <Adafruit_TinyUSB.h>`: neste core a USB serial vem da TinyUSB. |
| Primeiras linhas não aparecem no monitor | O `setup()` espera o monitor só por 3 s. Sem limite, `while (!Serial)` travaria a placa na bateria. Aperte reset uma vez com o monitor aberto. |

## Registro

Preenchido a partir dos logs reais.

| Item | Valor |
|---|---|
| Plataforma | `Seeed Studio Xiao Series (1.1.1+sha.a6fd41d)` |
| Framework | `framework-arduinoadafruitnrf52 @ 1.11300.0` = Seeed nRF52 Boards 1.1.13 |
| Biblioteca USB | Adafruit TinyUSB Library 1.7.0 (vem com o core) |
| Toolchain | `toolchain-gccarmnoneeabi @ 1.70201.0` (GCC 7.2.1) |
| Uso de RAM / Flash (teste 1.1) | 4680 B de 237568 B (2,0%) / 28840 B de 811008 B (3,6%) — build de 09/11/2026 |
| Upload e monitor | *a preencher no primeiro upload* |
| Porta COM (aplicação / bootloader) | *a preencher no primeiro upload* |
