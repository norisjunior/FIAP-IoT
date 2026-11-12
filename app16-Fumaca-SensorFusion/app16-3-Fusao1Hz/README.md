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
| Arquivo | o que a placa mandou | o mesmo (`.log`) e o dataset pronto (`.csv`) |

O botão BOOT, a sequência de estímulos, as cores do LED e o painel são os mesmos.

**Não misture coletas do app16-2 e do app16-3 no mesmo dataset.** Com a correção de umidade,
o `tvoc` e o `eco2` saem diferentes para o mesmo ar.

## As colunas

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
| `coleta-AAAAMMDD-HHMM.csv` | o dataset: cabeçalho e uma linha por segundo |
| `coleta-AAAAMMDD-HHMM.log` | tudo o que a placa mandou, desde o boot; é o que o replay lê |
| `coleta-AAAAMMDD-HHMM.png` | os gráficos da coleta inteira |

- **Repetir uma coleta gravada**, sem a placa: `python coleta.py --replay coleta-AAAAMMDD-HHMM.log 20`
  (20 vezes mais rápido). O replay não grava dataset. A do professor:
  `python coleta.py --replay coleta_professor/coleta-20261111-2207.log 20`.
- **Para o dataset:** a rodada 1 (o aquecimento) fica de fora. O ar que vale é o das rodadas
  entre os estímulos.
- **O rótulo é o aperto do BOOT, não a chegada do estímulo:** a curva começa a mudar 15 a
  30 s depois, e a fumaça ainda aparece nos primeiros 100 s do `ar` seguinte.

## A coleta do professor

[`coleta_professor/`](coleta_professor/) tem a primeira sessão, de 11/11/2026, às 22:07: o
`.csv` (o dataset), o `.log` e o `.png`. As rodadas pares são os estímulos; as ímpares, o
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
