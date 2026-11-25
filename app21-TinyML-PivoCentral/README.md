# TinyML no pivô central — a rede neural decide na placa e manda na bomba

Um pivô central irriga a lavoura girando em círculo: um cano longo, apoiado em torres com
rodas, cada torre com o seu motor. A placa do grupo (ESP32 + MPU) fica numa dessas torres e
classifica o motor a cada segundo, como desde a coleta do dataset. A diferença está em dois
lugares:

1. **Quem decide é uma rede neural, dentro da placa.** A floresta da aula passada coube na
   flash como `if`/`else`. Aqui é uma rede neural com o mesmo desenho da rede da API
   (8 → 16 → 4), convertida para **TensorFlow Lite** e executada pelo interpretador do TFLite
   na própria placa.
2. **A decisão vira uma ordem para outra máquina.** A torre manda na bomba do pivô (M2M): com
   o motor operando, irriga; inclinado ou em anomalia, desliga. A nuvem e a pessoa também
   mandam na bomba, e a bomba sabe quem vence.

```text
torre (ESP32 + MPU + rede neural) ─► decide ─► LEDs e buzzer
   ├─► FIAPIoT/motor/borda/<nome>     a janela e a classe (a nuvem confere, como na aula passada)
   └─► FIAPIoT/pivo/<nome>/cmd/local  a ordem da placa para a bomba
                                            │
n8n: chuva prevista ─► .../cmd/cloud ──────►│
n8n: chat (pessoa)  ─► .../cmd/human ──────►│
                                            ▼
                         bomba (ESP32 + servo + LED): quem manda?
                            └─► FIAPIoT/pivo/<nome>/estado ─► n8n ─► PostgreSQL (pivo_bomba)
```

| Pasta | O que é | Quem grava |
|---|---|---|
| [`colab/`](colab/) | o notebook: treina a rede, converte para TFLite, gera os dois `.hpp` | aluno |
| [`device-torre-devkit/`](device-torre-devkit/) | a torre: o firmware da aula passada com a rede no lugar da floresta | aluno |
| [`device-torre-esp32c3/`](device-torre-esp32c3/) | a mesma torre, na ESP32-C3 do professor | professor |
| [`device-bomba-devkit/`](device-bomba-devkit/) | a bomba do pivô: servo (a válvula) e LED | professor |
| [`n8n/`](n8n/) | a nuvem (chuva) e a pessoa (chat) | professor; aluno, se quiser |

A API que confere a placa continua a da aula da nuvem
([`app18-Inferencia_AI_Sinais-MultiClass_IMU/api/`](../app18-Inferencia_AI_Sinais-MultiClass_IMU/api/)),
e os dois fluxos que guardam e conferem continuam os da aula passada
([`app20-Inferencia-Borda/n8n/`](../app20-Inferencia-Borda/n8n/)): a torre publica o mesmo JSON,
no mesmo tópico.

---

## Quem manda na bomba

A bomba guarda a última ordem de cada um e obedece ao mais forte que ainda vale:

| Quem | Decide por quê | Tópico | Vale por |
|---|---|---|---|
| **a pessoa** | pediu no chat: ligar, desligar ou devolver | `FIAPIoT/pivo/<nome>/cmd/human` | 30 min (ou até devolver) |
| **a nuvem** | mais de 10 mm de chuva previstos nas próximas 2 h: desliga | `FIAPIoT/pivo/<nome>/cmd/cloud` | 2 h |
| **a placa** | a torre operando: liga; qualquer outra classe: desliga | `FIAPIoT/pivo/<nome>/cmd/local` | 90 s (a placa repete a cada 30 s) |

Sem nenhuma ordem válida, a bomba desliga. A regra inteira, no firmware da bomba:

```cpp
if (vale(pessoa, VALIDADE_PESSOA)) {
  ligar = pessoa.ligar;  quem = "pessoa";  motivo = pessoa.motivo;
} else if (vale(nuvem, VALIDADE_NUVEM)) {
  ligar = nuvem.ligar;   quem = "nuvem";   motivo = nuvem.motivo;
} else if (vale(placa, VALIDADE_PLACA)) {
  ligar = placa.ligar;   quem = "placa";   motivo = placa.motivo;
}
```

**Por que prazo de validade.** Sem prazo, quem fala por último vence: a placa repete "ligar"
a cada 30 s e desfaz, meio minuto depois, a ordem da nuvem ou a da pessoa. Com prazo, a
ordem de cima segura a bomba enquanto vale, e a de baixo fica guardada para quando ela
vencer.

As ordens são JSON:

```json
{"dispositivo": "grupo03", "bomba": true,  "motivo": "operando"}
{"dispositivo": "nuvem",   "bomba": false, "motivo": "chuva prevista: 25 mm em 2 h"}
{"dispositivo": "pessoa",  "bomba": true,  "motivo": "pedido no chat"}
{"dispositivo": "pessoa",  "devolver": true}
```

E a bomba conta o que fez, a cada mudança:

```json
{"dispositivo": "BombaDoPivo", "pivo": "grupo03", "bomba": false,
 "quem_manda": "nuvem", "motivo": "chuva prevista: 25 mm em 2 h"}
```

---

## Antes de começar

- A bancada da aula passada no ar: a placa, a API, o n8n com o Postgres e os dois fluxos que
  guardam e conferem (`app20 - Ingestao da predicao da borda...` e
  `app20 - Ingestao da comparacao borda x nuvem...`).
- O dataset do grupo no InfluxDB, ou o dataset de referência (o Plano B do notebook).

## 1) Treinar a rede (Colab)

[`colab/treinamento_rede_tflite.ipynb`](colab/treinamento_rede_tflite.ipynb): a mesma leitura,
o mesmo corte por rodada e o mesmo scaler do notebook da floresta. Depois:

1. treina a rede no Keras: 8 entradas → 16 neurônios (ReLU) → 4 saídas (softmax), **212
   pesos**;
2. converte para TensorFlow Lite (`modelo_motor.tflite`) e confere que o `.tflite` decide igual
   à rede do Keras;
3. mostra o que acontece ao quantizar para int8 (aqui, o arquivo **cresce**);
4. gera `ModeloMotorScaler.hpp` (o mesmo formato da aula passada) e `ModeloMotorRede.hpp` (os
   bytes do `.tflite` como um vetor em C++);
5. baixa os três num `.zip`.

Execute a primeira célula de código duas vezes (na primeira, o Colab reinicia a sessão). Sem
token do InfluxDB, a seção 2 avisa e o notebook segue com o dataset de referência.

Medido com o dataset de referência (12/11, 360 janelas; treino nas rodadas 1 e 2, teste na 3;
TensorFlow 2.21.0):

| O quê | Valor |
|---|---|
| Acerto na rodada 3 | 120 de 120 (a rede do Keras e o `.tflite`) |
| `modelo_motor.tflite` | 2.604 bytes (float32); em int8, 2.752 bytes |
| `ModeloMotorRede.hpp` | 227 linhas |
| As 360 janelas | a rede da placa acerta 360; a da API, 359; a floresta, 360 |
| Rede da placa × rede da API | concordam em 359 de 360 (a janela de `anomalia` que a API chama de `inclinado_tras`) |

## 2) O firmware da torre

[`device-torre-devkit/`](device-torre-devkit/) é o firmware da aula passada com duas mudanças:
a floresta sai e entra a rede (TFLite), e a decisão vira uma ordem para a bomba. O passo a
passo está em [`CONSTRUIR-O-FIRMWARE.md`](device-torre-devkit/CONSTRUIR-O-FIRMWARE.md).

1. Copie os dois `.hpp` do `.zip` direto para `src/`, por cima dos que vieram no projeto (os do
   dataset de referência).
2. Troque `DEFINA_UM_NOME` pelo nome da placa (o mesmo do dataset e dos gatilhos do n8n).
3. Grave e abra o Monitor Serial:

```text
Model Description: MLIR Converted.
Model Version: 3
Arena Used Bytes: ...
...
--- Janela fechada ---
  mean_ax=0.003  mean_ay=-0.001  mean_az=0.999
  std_ax=0.007   std_ay=0.007   std_az=0.009
  std_mag=0.009  p2p_mag=0.063
  PREDIÇÃO:  3 -> operando (100%)
  Inferencia: ... us
----------------------
>> Ordem para a bomba: LIGAR (operando)
```

`Arena Used Bytes` é quanto da memória de trabalho (`ARENA_SIZE`, 8 KB) a rede usou.
`Inferencia` é o tempo do scaler e da rede, em microssegundos. A ordem para a bomba só
aparece quando muda (e é repetida, sem aparecer, a cada 30 s).

### Floresta × rede, na mesma placa

Compilado com `pio run` (`esp32dev`, `espressif32@6.12.0`):

| Firmware | Flash | RAM |
|---|---|---|
| só decide, com a floresta (aula passada) | 301.089 B (23,0%) | 23.732 B (7,2%) |
| só decide, com a rede (TFLite) | 572.117 B (43,6%) | 37.820 B (11,5%) |
| decide e publica, com a floresta (aula passada) | 787.085 B (60,0%) | 47.068 B (14,4%) |
| **a torre: decide, publica e manda na bomba, com a rede** | **1.061.073 B (81,0%)** | **61.164 B (18,7%)** |

A rede ocupa 2,6 KB; quem pesa é o **interpretador** do TFLite: cerca de 264 KB de flash, e
8 KB de RAM para a arena. A biblioteca (`MicroTFLite`) registra todas as operações do TFLite,
e a nossa rede usa duas (`FULLY_CONNECTED` e `SOFTMAX`).

### Na ESP32-C3 do professor

[`device-torre-esp32c3/`](device-torre-esp32c3/): o mesmo firmware no hardware do app18
(I2C 20/21, LEDs nos GPIO 1, 3, 4 e 10, buzzer no 7, RGB no 8, motor de vibração no 0),
com `MQTT_CLIENT_ID "IoTDevMultiClasse001"`. Flash 80,1%, RAM 16,8%. É a placa que a bomba
obedece por padrão.

## 3) A bomba (professor)

[`device-bomba-devkit/`](device-bomba-devkit/): uma DevKit com um servo (a válvula, GPIO 25)
e um LED (a bomba, GPIO 27). Assina `FIAPIoT/pivo/<torre>/cmd/+`, aplica a regra de quem
manda e publica o estado. Ao ligar, obedece a `IoTDevMultiClasse001`, a C3 do professor. Para
passar a bomba para a placa de um grupo, sem regravar:

```bash
mosquitto_pub -h <broker> -t "FIAPIoT/pivo/bomba/obedecer" -m "grupo03"
```

A bomba avisa a torre antiga, apaga as ordens guardadas, assina as ordens de `grupo03` e fica
desligada até a primeira ordem da torre nova (em até 30 s). Mensagem vazia (ou um RESET) volta
para a C3. Detalhes no [README da bomba](device-bomba-devkit/README.md).

No Serial:

```text
Ordem da placa: LIGAR (operando)
>> BOMBA LIGADA - quem manda: placa (operando)
   quem manda agora: placa
Ordem da nuvem: DESLIGAR (chuva prevista: 25 mm em 2 h)
>> BOMBA DESLIGADA - quem manda: nuvem (chuva prevista: 25 mm em 2 h)
   quem manda agora: nuvem
Ordem da placa: LIGAR (operando)
   quem manda agora: nuvem
```

A última ordem da placa foi guardada, mas não mandou: a nuvem vale por 2 h.

Flash 58,9%, RAM 13,8%.

## 4) A nuvem: a previsão de chuva

[`n8n/Fluxo-1-pivo-nuvem.json`](n8n/Fluxo-1-pivo-nuvem.json), 9 nós:

```text
Estado da bomba (MQTT) ─► Formata para o banco ─┬─► Armazena o estado (pivo_bomba)
                                                └─► A placa ligou a bomba?
                                                      ─► Previsao de chuva (Open-Meteo)
                                                      ─► Chuva nas proximas 2 h
                                                      ─► Vai chover?
                                                      ─► Ordem da nuvem ─► Ordem para a bomba (cmd/cloud)
```

- Guarda **o que a bomba fez**, e não as ordens: é isso que responde "está irrigando?".
- Só consulta a chuva quando a bomba acaba de ligar por ordem da placa. A previsão é da
  Open-Meteo, sem chave, para a latitude e a longitude do nó **Previsao de chuva** (São Paulo;
  troque pela sua fazenda).
- Mais de 10 mm nas próximas 2 h: manda desligar. Para a demonstração, no nó **Chuva nas
  proximas 2 h**, troque `const SIMULAR_MM = null;` por `25`, e volte depois.

Credenciais: MQTT e Postgres (as da aula passada). Troque `DEFINA_UM_NOME` pelo nome da torre
no gatilho **Estado da bomba** e no nó **Ordem para a bomba**.

## 5) A pessoa: o chat

[`n8n/Fluxo-2-pivo-chat.json`](n8n/Fluxo-2-pivo-chat.json), 11 nós: o agente com o Gemini, a
memória, duas consultas ao banco (`bomba_agora` e `bomba_historico`) e uma saída estruturada
com `resposta` e `ordem` (`LIGAR`, `DESLIGAR`, `DEVOLVER` ou `NENHUMA`). Com ordem, o fluxo
publica em `cmd/human` **antes** de responder no chat: assim a ordem sai mesmo que a página
do chat feche.

Perguntas para testar: "Está irrigando?", "Quem desligou a bomba e por quê?", "Ligue a
bomba", "Pode voltar para o automático", "Como estava ontem à noite?".

Credenciais: Postgres e Google Gemini (a chave do grupo). Troque `DEFINA_UM_NOME` no nó
**Ordem para a bomba**.

## 6) Testar por partes

Sem placa nenhuma, só o broker e o n8n:

```bash
# a bomba conta que ligou por ordem da placa: o fluxo 1 grava e consulta a chuva
mosquitto_pub -h localhost -t "FIAPIoT/pivo/grupo03/estado" \
  -m '{"dispositivo":"BombaDoPivo","pivo":"grupo03","bomba":true,"quem_manda":"placa","motivo":"operando"}'

# as ordens que chegam à bomba
mosquitto_sub -h localhost -t "FIAPIoT/pivo/grupo03/cmd/#" -v
```

Testado em 18/11 num n8n 2.47.7 descartável: com a previsão real (0 mm), a nuvem não manda
nada; com `SIMULAR_MM = 25`, a ordem da nuvem sai em 0,9 s; o estado vai para `pivo_bomba`;
no chat (com um Gemini de mentira), "desligue a bomba" publica
`{"dispositivo":"pessoa","bomba":false,"motivo":"pedido no chat"}` mesmo sem a página do chat
aberta, e "volte para o automático" publica `{"dispositivo":"pessoa","devolver":true}`.

Em 19/11, com o Gemini de verdade (`models/gemini-3.5-flash-lite`): 12 de 12 conversas certas,
uns 3 s por resposta. Por exemplo:

```text
> Está irrigando agora?
  Não, a bomba está desligada. Quem manda é a nuvem, com o motivo "chuva prevista: 25 mm em
  2 h", desde as 07:50.
> Ligue a bomba, por favor.
  A ordem para ligar a bomba foi enviada. Ela vale por 30 minutos e passa por cima da placa e
  da nuvem.                       (cmd/human: {"dispositivo":"pessoa","bomba":true,...})
> Pode voltar para o automático.
  O controle foi devolvido; a nuvem e a placa voltam a mandar.   (cmd/human: devolver)
```

## Estrutura

```text
app21-TinyML-PivoCentral/
├── colab/
│   └── treinamento_rede_tflite.ipynb
├── device-torre-devkit/          o aluno grava este
│   ├── platformio.ini            + johnosbb/MicroTFLite
│   ├── CONSTRUIR-O-FIRMWARE.md
│   └── src/
│       ├── torre-device.cpp
│       ├── ModeloMotorRede.hpp   a rede (dataset de referência)
│       └── ModeloMotorScaler.hpp
├── device-torre-esp32c3/         a torre na C3 do professor
├── device-bomba-devkit/          a bomba do pivô (professor)
│   └── src/bomba-device.cpp
└── n8n/
    ├── Fluxo-1-pivo-nuvem.json
    └── Fluxo-2-pivo-chat.json
```

## Se der errado

| Sintoma | Causa provável |
|---|---|
| `ModeloMotorRede.hpp: No such file or directory` | o `.hpp` não está direto em `src/` |
| `Erro: a rede neural nao carregou` | o `ModeloMotorRede.hpp` não veio do notebook (ou a arena é pequena demais) |
| Sempre a mesma classe, em qualquer posição | os dois `.hpp` vieram de treinos diferentes: copie os dois do mesmo `.zip` |
| A ordem para a bomba não aparece no Serial | ela só aparece quando muda: passe o motor de `operando` para outra posição |
| A bomba não reage à placa | a bomba obedece a outra torre (veja `"pivo"` no `estado`; troque em `FIAPIoT/pivo/bomba/obedecer`), ou a nuvem ou a pessoa estão mandando (veja "quem manda agora" no Serial da bomba) |
| A nuvem nunca manda desligar | não vai chover: use `SIMULAR_MM` no nó **Chuva nas proximas 2 h** |
| O chat diz que não há dados | a tabela `pivo_bomba` só existe depois que a bomba publicar o primeiro estado |
