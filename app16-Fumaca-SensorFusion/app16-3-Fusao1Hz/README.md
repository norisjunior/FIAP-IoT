# app16-3-Fusao1Hz

O coletor do dataset da caixa de fumaça. É o app16-2 com o que o dataset precisa: uma linha
por segundo com as colunas do dataset de fumaça do Kaggle, a rodada e o rótulo marcado pelo
botão BOOT.

**Pronto quando:** o painel mostra as curvas e o `coleta-AAAAMMDD-HHMM.csv` ganha uma linha
por segundo.

## O que muda em relação ao app16-2

| | app16-2 (teste dos fenômenos) | app16-3 (dataset) |
|---|---|---|
| Colunas | 9 | 15, as do Kaggle mais `rodada` e `pm10` |
| Umidade | o SGP30 lê sozinho | o BME280 informa a umidade, e o SGP30 corrige o gás |
| Rodada | não tem | +1 a cada aperto do BOOT |
| Serial | uma variável por linha (`>nome: valor`) | CSV: o cabeçalho no boot e uma linha por segundo |
| Arquivo | o que a placa mandou | o mesmo (`.log`) e o dataset pronto (`.csv`) |

O botão BOOT, a sequência de estímulos, as cores do LED e o painel são os mesmos.

**Não misture coletas do app16-2 e do app16-3 no mesmo dataset.** Com a correção de umidade,
o `tvoc` e o `eco2` saem diferentes para o mesmo ar.

## As colunas

A placa manda o CSV pronto no Serial, e o `coleta.py` grava cada linha como chegou:

```
Sensores ok. A rodada 1 e o aquecimento: espere a caixa estabilizar e aperte BOOT.
amostra,rodada,temp,umid,tvoc,eco2,raw_h2,raw_ethanol,pressao,pm1_0,pm2_5,nc0_5,nc1_0,nc2_5,pm10,estimulo
=== ESTIMULO: ar (rodada 1) ===
1,1,25.22,63.49,0,400,14148,18961,933.51,9.48,10.03,65.25,75.45,75.67,10.03,0
2,1,25.24,63.49,0,400,14142,18957,933.53,10.39,14.02,63.18,78.95,82.58,17.81,0
```

O `.csv` tem as colunas do Kaggle, na mesma ordem. O Kaggle tem três colunas que não vêm de
sensor: o horário (`UTC`), o contador (`CNT`) e o rótulo (`Fire Alarm`). Aqui elas são a
`amostra`, a `rodada` e o `estimulo`.

| app16-3 | Kaggle | Sensor | O que é |
|---|---|---|---|
| `amostra` | `UTC`, `CNT` | — | segundos desde que a placa ligou; um número que falta é uma leitura que falhou |
| `rodada` | — | — | +1 a cada aperto do BOOT; a rodada 1 é o aquecimento |
| `temp` | Temperature[C] | BME280 | °C |
| `umid` | Humidity[%] | BME280 | % |
| `tvoc` | TVOC[ppb] | SGP30 | gás total, calculado pelo sensor (ppb) |
| `eco2` | eCO2[ppm] | SGP30 | CO₂ estimado a partir do H₂, não medido (ppm) |
| `raw_h2` | Raw H2 | SGP30 | valor cru; **cai** quando o gás sobe |
| `raw_ethanol` | Raw Ethanol | SGP30 | valor cru; **cai** quando o gás sobe |
| `pressao` | Pressure[hPa] | BME280 | hPa |
| `pm1_0`, `pm2_5` | PM1.0, PM2.5 | SPS30 | massa de partículas (µg/m³) |
| `nc0_5`, `nc1_0`, `nc2_5` | NC0.5, NC1.0, NC2.5 | SPS30 | contagem de partículas (por cm³) |
| `pm10` | — | SPS30 | massa das partículas maiores; o Kaggle não tem |
| `estimulo` | Fire Alarm | botão BOOT | 0 ar, 1 álcool, 2 vapor, 3 fumaça, 4 chama; incêndio = fumaça ou chama |

**Gravar não é usar.** A `amostra`, a `rodada` e a `pressao` ficam no arquivo, mas não viram
feature. No Kaggle, só o horário acertava 99,8% e só a pressão acertava 94%. Os dois mudam
com o momento da gravação, não com a fumaça (ver `LICOES-APRENDIDAS.md`). Com a pressão
gravada aqui, dá para mostrar o mesmo vazamento no nosso próprio dado.

## Antes

- O app16-0 mostrou os 3 sensores ok, e o app16-2 rodou o teste dos fenômenos.
- **Python para o `coleta.py`**, uma vez, no terminal desta pasta:

  ```powershell
  py -m venv .venv
  .\.venv\Scripts\Activate.ps1
  pip install -r requirements.txt
  ```

- **Ligue a caixa uns 40 min antes.** Na coleta de 11/11/2026, a placa levou uns 40 min
  esquentando: a temperatura foi de 26 para 31 °C, e a umidade, de 57 para 42%. É calor da
  própria placa. Gravar o firmware de novo não esfria a placa; desligar a USB, sim.

## A coleta

O protocolo é o do app16-2 (seção "O teste" do README dele): a mesma sequência, uma ação
por estímulo, um palmo de distância e 1 minuto de coleta antes de apertar `ar`.

1. Grave o firmware e rode `python coleta.py` nesta pasta. O painel abre, e a placa
   reinicia na **rodada 1 (ar)**: é o aquecimento.
2. Espere as curvas ficarem planas (uns 40 min se a caixa acabou de ligar). Então siga a
   sequência pelo BOOT: vapor, ar, chama, ar, fumaça, ar, álcool, ar.
3. Para parar, **Parar** no painel (ou Ctrl+C no terminal).

As regras que mais pesam:
- **Empatar a partícula:** o pico de `nc0_5` do vapor e o da fumaça na mesma faixa.
- **Chama na medida:** a `temp` sobe 2 a 3 °C, não mais.
- **Álcool por último:** o SGP30 leva uns 5 min para voltar depois dele.
- **O próximo estímulo só entra quando o gráfico voltou ao nível do ar,** a `temp`
  também. Fumaça com a placa ainda morna da chama é reconhecida pela temperatura.
- **Porta e ar-condicionado parados**, menos para arejar depois da fumaça.

**Duas sessões, em dias diferentes:** o modelo treina numa e é testado na outra.

## O que fica gravado

Ficam nesta pasta, fora do Git. A coleta que vale vai para `coleta_professor/`.

| Arquivo | O que é |
|---|---|
| `coleta-AAAAMMDD-HHMM.csv` | o dataset: cabeçalho e uma linha por segundo, como a placa mandou |
| `coleta-AAAAMMDD-HHMM.log` | tudo o que a placa mandou, desde o boot: as linhas do CSV e as mensagens |
| `coleta-AAAAMMDD-HHMM.png` | os gráficos da coleta inteira |

- **Repetir uma coleta gravada**, sem a placa: `python coleta.py --replay coleta-AAAAMMDD-HHMM.log 20`
  (20 vezes mais rápido). O replay não grava dataset. Ele lê o `.log` ou o `.csv`; com o
  `.csv`, o painel fica sem as mensagens da placa. A do professor:
  `python coleta.py --replay coleta_professor/coleta-20261111-2207.csv 20`.
- **Para o dataset:** a rodada 1 (o aquecimento) fica de fora. O ar que vale é o das rodadas
  entre os estímulos.
- **O rótulo é o aperto do BOOT, não a chegada do estímulo:** a curva começa a mudar 15 a
  30 s depois, e a fumaça ainda aparece nos primeiros 100 s do `ar` seguinte.

## A coleta do professor

[`coleta_professor/`](coleta_professor/) tem três sessões, cada uma com o `.csv` (o dataset),
o `.log` e o `.png`. A primeira é de 11/11/2026, às 22:07. As rodadas pares são os estímulos; as ímpares, o
ar.

| Rodada | Estímulo | Começa na amostra | O que mexeu |
|---|---|---|---|
| 1 | ar (aquecimento) | 1 | — |
| 2 | vapor | 733 | `nc0_5` até 1.061, `umid` +5 pontos |
| 4 | chama | 1.182 | `temp` de 29,0 para 36,9 °C, `tvoc` até 1.225 |
| 6 | fumaça | 1.773 | `nc0_5` até 6.049 |
| 8 | álcool | 2.552 | `tvoc` até 23.702, `raw_h2` −13% |

Três desvios do protocolo, que ficam como estão:

- **Aquecimento curto:** 12 min, e a `temp` ainda subia na rodada do vapor.
- **Fumaça soprada direto no sensor:** o pico de `nc0_5` ficou 5,7 vezes o do vapor. Não
  estragou o dataset: o melhor limiar de partícula acerta só 67% (a média do acerto em
  incêndio e em normal), porque a chama não tem partícula.
- **Chama forte:** +7,9 °C. A placa levou 16 min para esfriar, e a fumaça entrou depois de
  8, com a `temp` em 30 °C. A `temp` sozinha marca toda a fumaça como incêndio: é o
  vazamento de momento da pressão do Kaggle, agora no nosso dado.

A primeira linha que chegou era sobra de antes do reinício da placa (amostra 116) e saiu do
`.csv`; o `.log` guarda tudo. O `coleta.py` já descarta essa sobra.

O `.log` dessa sessão é de quando a placa mandava uma variável por linha (`>nome: valor`).
O `coleta.py` de hoje não lê esse formato: para repetir a sessão, use o `.csv`.

### A segunda sessão: 13/11/2026, às 10:34

Outro dia, a mesma sequência, já com o CSV no Serial (`coleta-20261113-1034.*`). 5.152
linhas, nenhuma falha de leitura.

| Rodada | Estímulo | Começa na amostra | O que mexeu |
|---|---|---|---|
| 1 | ar (aquecimento) | 1 | 32 min; a `temp` já começou em 31,3 °C |
| 2 | vapor | 1.904 | `nc0_5` até 3.686, `umid` +3 pontos |
| 4 | chama | 2.598 | `temp` de 31,8 para 32,6 °C |
| 6 | fumaça | 3.656 | `nc0_5` até 5.426 |
| 8 | álcool | 4.526 | `tvoc` até 22.176, `raw_h2` −10% |

- **Partícula empatada:** o pico de `nc0_5` da fumaça ficou 1,5 vez o do vapor (em 11/11,
  5,7). Na massa (`pm2_5`), o vapor passou a fumaça: 2.326 contra 1.303.
- **Chama fraca:** +0,8 °C (o protocolo pede 2 a 3 °C).
- **Caixa quente a sessão inteira:** 31,4 a 33,0 °C (em 11/11, ~29 °C fora da chama). A
  árvore treinada em 11/11 diz `fire` com pouca partícula acima de 32,05 °C: no teste em
  13/11, alarme falso em 47% dos segundos sem fogo (o ar e o álcool), nenhum fogo perdido. É
  o data drift do outro dia.
- **A linha de base mudou:** no ar, `nc0_5` perto de 30 (em 11/11, 70 a 100), `umid` perto de
  44%, pressão média 930,5 hPa (em 11/11, 933,6).

### A terceira sessão: 17/11/2026, às 18:04

Gravada porque, nesse dia, a fumaça em sala úmida deu `not fire` no modelo de 11/11
(`coleta-20261117-1804.*`). 2.434 linhas, nenhuma falha de leitura.

**Fica como registro: não entra no dataset do aluno.** O dataset e o modelo seguem as sessões
de 11/11 e 13/11. Só esta sessão tem 20 colunas: o coletor gravou, nesse dia, o resto do SPS30
(`pm4_0`, `nc4_0`, `nc10` e o `tamanho` típico da partícula, em µm), antes do `estimulo`. O
coletor voltou às 16 colunas em 26/11/2026, e o replay do `coleta.py` lê esta sessão também.

| Rodada | Estímulo | Começa na amostra | O que mexeu |
|---|---|---|---|
| 1 | ar (aquecimento) | 1 | 6 min; a `temp` começou em 32,9 °C |
| 2 | vapor | 386 | `nc0_5` até 1.169, `umid` +3 pontos, `tamanho` até 1,43 µm |
| 4 | chama | 757 | `temp` de 30,3 para 30,7 °C |
| 6 | fumaça | 1.423 | `nc0_5` até 6.569, `tamanho` perto de 0,9 µm |
| 8 | álcool | 2.124 | `tvoc` e `eco2` no teto do sensor (60.000), `raw_h2` −25% |

- **Fumaça em sala úmida:** a `umid` ficou entre 49,7 e 50,1% durante a fumaça. Nas sessões
  de 11/11 e 13/11, toda fumaça foi a 43–47%. O modelo de 11/11 (7 features) dá `fire` em 0%
  dos segundos de fumaça desta sessão: ele separa fumaça de vapor pela `umid` ≤ 47,3%.
- **O `tamanho` separa vapor e fumaça:** depois do preparo do dataset, vapor 1,10 a 1,43 µm
  (mediana 1,21), fumaça 0,71 a 1,21 (0,88), ar, chama e álcool perto de 0,54. A gota do vapor
  é maior que a partícula da fumaça.
- **Ambiente sem climatização:** a `temp` acompanhou o dia, de 32,9 para 29,4 °C ao longo da
  sessão, e a `umid` subiu de 44,8 para 52%. Não dá para esperar a caixa estabilizar quando é a
  sala que muda.
- **Chama fraca:** +0,4 °C e nenhuma partícula. De novo, nenhum sensor da caixa a vê.
- **Partícula sem empate:** o pico de `nc0_5` da fumaça ficou 5,6 vezes o do vapor (na massa,
  `pm2_5`, 1,9 vez).
- **Pouco segundo por estímulo:** depois do preparo, 36 de fumaça, 45 de vapor, 43 de chama e
  36 de álcool. Uma segunda volta na sequência (seguir apertando o BOOT) dobra isso.

## Depois: dois notebooks

O dataset guarda as 5 classes separadas. Quem junta é o notebook (em `colab/`, a fazer),
sobre o mesmo CSV:

- `treinamento_fumaca_5classes.ipynb`: ar, álcool, vapor, fumaça e chama;
- `treinamento_fumaca_fire.ipynb`: **fire** (fumaça e chama) × **not fire** (ar, vapor e
  álcool), o mesmo alvo do `Fire Alarm` do Kaggle.

## Decisões

- **1 Hz:** o SGP30 precisa ser lido a cada 1 s, e o SPS30 tem leitura nova a cada 1 s.
- **Correção de umidade:** é o uso previsto do SGP30. Quem fizer a inferência depois (nuvem
  ou borda) precisa ler os sensores do mesmo jeito: com a correção.
- **Linha de base do SGP30:** não é salva nem restaurada. O `tvoc` e o `eco2` de um dia não
  se comparam com os de outro. Isso é data drift, e a segunda sessão mostra.
- **A linha só sai com os 3 sensores lidos.** Se um falha, o Serial avisa (`Erro de leitura
  na amostra N`), e aquele segundo fica fora do dataset.
