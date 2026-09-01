# app17-11-MotorMultiClasse — 5 classes do motor (Forma 1)

Firmware do ESP32 que lê o acelerômetro a 500 Hz, calcula 12 features por janela de 2 s
(1000 amostras) e imprime 1 linha CSV por janela no Monitor Serial. Se WiFi e o broker da
IoT-platform estiverem disponíveis, também publica a mesma janela por MQTT — mas o firmware
funciona sozinho, só pela Serial, sem rede nenhuma.

> **Hardware físico.** Este app foi projetado para um MPU6500 acoplado a um mini motor com
> hélice, montado num gabarito 3D com as posições de inclinação. O `diagram.json` do Wokwi
> cobre o MPU + botões + LED (para compilar/testar a lógica no simulador), mas **as classes de
> inclinação exigem o motor e o gabarito reais** — não têm equivalente fiel no simulador.

## Montagem

| Função | GPIO |
|---|---|
| MPU SDA | 19 |
| MPU SCL | 18 |
| Botão **MOTOR** (liga/desliga o motor) | 26 |
| Botão **COLETA** (dispara uma rodada) | 25 |
| LED externo (aceso = coleta em andamento) | 27 |
| LED onboard (aceso = motor ligado) | 2 |
| Motor IN1 | 22 |
| Motor IN2 | 23 |

Sensor ativo no código: **MPU6500**. Para usar o MPU6050 original (GY-521), troque a linha
`MPU6500 mpu(Wire);` por `MPU6050 mpu(Wire);` no topo do `.cpp` (comentário já indica onde).

## As 5 classes (sequência fixa, cíclica)

```
desligado -> operando -> inclinado_frente -> inclinado_tras -> anomalia -> (repete)
```

| classe | motor | posição no gabarito |
|---|---|---|
| `desligado` | desligado | plano |
| `operando` | ligado | plano |
| `inclinado_frente` | ligado | posição "frente" do gabarito |
| `inclinado_tras` | ligado | posição "trás" do gabarito |
| `anomalia` | ligado | plano, **com fita/massa colada na hélice** |

O firmware sempre sabe qual é a classe atual (imprime no Monitor Serial) — não é preciso
anotar nada à parte.

## Protocolo de coleta

1. **Botão MOTOR (26):** liga/desliga o motor. Toggle simples. Bloqueado enquanto uma rodada
  está em andamento.
2. **Botão COLETA (25):** dispara uma rodada completa da classe atual — um toque só, não é
  toggle e não é possível pular etapa. A sequência:
  - o firmware confere se o motor está no estado que a classe pede; se não bater, recusa e
    avisa no Monitor Serial — ajuste o botão MOTOR e toque de novo;
  - espera 1 s (tempo para tirar a mão do conjunto);
  - coleta 30 janelas de 2 s (60 s no total), imprimindo 1 linha CSV por janela;
  - para sozinho e avança para a próxima classe da sequência.
3. Repita o passo 2 para cada classe. Ao completar as 5, a sequência **recomeça do início** —
  isso conta como uma nova "rodada" (o número de rodada aparece em cada linha do CSV).
4. **Antes da classe `anomalia`**, o Monitor Serial avisa em destaque para colar a fita/massa
  na hélice. **Ao voltar para `desligado`** (fechando o ciclo), avisa para removê-la. Siga os
  avisos — sem isso a classe `operando` da rodada seguinte sai contaminada.
5. Colete pelo menos **2-3 voltas completas da sequência** (ideal: 5) antes de levar os dados
  para o notebook `2.6` — é o que a validação por rodada (`LeaveOneGroupOut`) precisa.

## Formato da saída (Serial e MQTT)

Marcadores de início/fim de cada rodada:

```
# INICIO classe=inclinado_frente rodada=03 motor=1
# FIM rodada=03 janelas=30
```

Cabeçalho impresso uma vez, no boot:

```
classe,rodada,janela,fs_real,mean_ax,mean_ay,mean_az,std_ax,std_ay,std_az,rms_mag,std_mag,p2p_mag,crest_mag,kurt_mag,zcr_mag
```

`fs_real` é a taxa de amostragem efetiva medida na própria janela (amostras / tempo decorrido)
— confira se fica perto de 500 Hz.

## Como rodar

### Opção A — Wokwi (só para testar a lógica/compilação)

Compile e abra o simulador; o `diagram.json` traz MPU + botões + LED. As classes de inclinação
não têm equivalente no simulador (ver aviso no topo deste README).

### Opção B — ESP32 físico

1. Compile e grave no ESP32.
2. Monte o MPU6500 no motor, encaixado no gabarito 3D.
3. Abra o Monitor Serial (115200 baud) e siga o protocolo de coleta acima.
4. Para MQTT: descomente o bloco **(B)** de WiFi no topo do `.cpp` e ajuste rede/IP. Sem isso,
  o firmware funciona normalmente só pela Serial (Fase 1).
