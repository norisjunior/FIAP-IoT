# app21-2-TesteFenomenos

Os 3 sensores da caixa de fumaça num painel ao vivo, com o botão BOOT marcando o estímulo
que está perto da caixa. Cada aperto passa para o próximo estímulo, e o nome dele aparece
no painel e na cor do LED.

**Pronto quando:** o vapor mexe no SPS30 e na umidade, o álcool só no SGP30, a fumaça nos
dois, e a chama na temperatura.

## A ideia

Quatro estímulos, escolhidos para que cada sensor engane pelo menos uma vez:

| Estímulo | partícula | gás | calor | umidade | Incêndio? |
|---|---|---|---|---|---|
| ar | — | — | — | — | não |
| vapor de água (chaleira) | ↑↑ | ↑ | — | ↑↑ | não |
| álcool | — | ↑↑ | — | — | não |
| fumaça (vela recém-apagada) | ↑↑ | ↑ | — | — | **sim** |
| chama (isqueiro) | — | ↑ | ↑ | — | **sim** |

- **Limiar só na partícula:** o vapor vira alarme falso, e a chama passa sem alarme.
- **Limiar só no gás:** o álcool vira alarme falso.
- **Limiar só no calor:** a fumaça passa sem alarme.
- **Só a combinação acerta todos.** É isso que justifica o modelo.

O vapor faz o papel do banho quente e da panela no fogão: partícula sem fogo, o alarme
falso clássico do detector de fumaça comum. A chama do isqueiro é o contrário: fogo que
queima limpo, quase sem partícula, e que o detector comum não vê.

## Antes

- O app21-0 mostrou os 3 sensores ok, e o app21-1 mostrou cada sensor no plotter.
- **Python para o `coleta.py`**, uma vez, no terminal desta pasta:

  ```powershell
  py -m venv .venv
  .\.venv\Scripts\Activate.ps1
  pip install -r requirements.txt
  ```

- **Material:** chaleira com água recém-fervida, isqueiro, vela num pires, álcool (o líquido
  de 46° serve) e um chumaço de algodão num pires. Um copo de água ao lado.
- **Onde:** uma sala pequena com a porta fechada, para nenhuma corrente de ar levar o
  estímulo embora. Não precisa de câmara.

## O teste

Cada estímulo é **uma ação só**, a **um palmo da caixa**. Depois dela, a coleta segue por
**1 minuto**: aperte `ar` quando o painel mostrar `há 1:00` ao lado do nome do estímulo.

| Aperto | Aparece | LED | O que fazer |
|---|---|---|---|
| (ligou) | `=== ESTIMULO: ar ===` | verde | esperar 1 min: o SGP30 aquece e o ventilador do SPS30 estabiliza |
| 1º | `vapor` | roxo | trazer a chaleira recém-fervida e destampada por 5 s, de lado e um pouco abaixo da placa |
| 2º | `ar` | verde | esperar o gráfico voltar |
| 3º | `chama` | azul | acender o isqueiro por 5 s, de lado e um pouco abaixo da altura da placa |
| 4º | `ar` | verde | esperar o gráfico voltar |
| 5º | `fumaca` | vermelho | acender a vela longe da caixa, apagar com um sopro e trazer o pires fumegando |
| 6º | `ar` | verde | levar a vela para fora, abrir a porta, abanar e esperar o `nc0_5` voltar perto do começo |
| 7º | `alcool` | amarelo | molhar o algodão com álcool e pôr o pires; tirar com `há 1:00` |
| 8º | `ar` | verde | esperar o gráfico voltar; a sequência recomeça no próximo aperto |

Quatro regras:

1. **Empatar a partícula.** O pico de `nc0_5` do vapor e o da fumaça precisam ficar na mesma
   faixa, alguns milhares. A dose muda pela distância, não pela repetição: vapor fraco,
   chaleira mais perto ou logo depois de ferver; fumaça passando de 10 mil, vela mais longe. Acerte a dose antes, num
   ensaio rápido. Se a fumaça tiver muito mais partícula que o vapor, um limiar só na
   partícula resolve, e o modelo deixa de ser necessário.
2. **Álcool por último.** O SGP30 demora minutos para voltar depois dele.
3. **O próximo estímulo só entra quando o gráfico voltou ao nível do ar.** Abanar com um
   caderno ajuda.
4. **Porta e ar-condicionado parados**, menos para arejar depois da fumaça. A sala trocando
   de ar muda a temperatura e o gás, e o modelo aprende o momento da coleta, não o estímulo.

Faça a coleta **duas vezes, em dias diferentes**: o modelo treina numa e é testado na
outra.

- **Chama nunca embaixo da placa:** o SPS30 e a cola quente não aguentam o calor.
- **Placa nunca em cima da chaleira:** ali a umidade passa de 80%, e a água condensa na
  placa aberta e na câmara do laser do SPS30.
- **Vela apagada e fora da sala antes de abrir o álcool:** o vapor de álcool pega fogo.
- **O sopro da vela vai para o lado, não na caixa:** a respiração sobe a umidade, e a
  umidade é o que separa o vapor da fumaça.
- **BOOT:** depois que a placa ligou, é um botão comum. Não o segure ao ligar a USB nem ao
  apertar RST: aí a placa entra no modo de gravação.

## Gravar a coleta

O `coleta.py` faz tudo sozinho: abre a porta, grava cada linha num arquivo e mostra um
painel ao vivo no navegador. Não precisa de Teleplot nem de monitor.

1. No terminal, dentro desta pasta: `.\.venv\Scripts\Activate.ps1` e depois
   `python coleta.py`. Ele acha a porta sozinho: é a única com o código USB `303A`
   (Espressif). Com mais de uma placa ligada, ele lista as portas e pede uma:
   `python coleta.py COM27`. O `pio device list` mostra o mesmo.
2. A placa reinicia, e o arquivo começa no boot. O navegador abre o painel: 8 gráficos
   (`pm2_5` e `pm10` no mesmo), com o fundo na cor do estímulo, a mesma do LED. No alto, o
   estímulo atual e há quanto tempo ele começou.
3. Para parar, **Parar** no painel (ou Ctrl+C no terminal). Ficam nesta pasta, fora do
   Git (a coleta que vale vai para `coleta_professor/`):
   - `coleta-AAAAMMDD-HHMM.csv`: tudo o que a placa mandou, desde o boot;
   - `coleta-AAAAMMDD-HHMM.png`: os gráficos da coleta inteira, prontos para o slide.

- **Painel:** `1 min`, `5 min` e `tudo` escolhem quanto tempo os gráficos mostram. Passe o
  mouse sobre um gráfico para ler os valores de todos naquele segundo. Fechar a aba não
  para a coleta: reabra pelo endereço que o terminal mostra.
- **Reiniciar a placa:** botão **Reiniciar placa**, ou Ctrl+T e depois Ctrl+D no terminal,
  duas vezes, como no monitor. A coleta continua no mesmo arquivo.
- **Repetir uma coleta gravada**, sem a placa: `python coleta.py --replay coleta-AAAAMMDD-HHMM.csv`.
  Um número no fim acelera (`... .csv 10` = 10 vezes mais rápido). A do professor:
  `python coleta.py --replay coleta_professor/coleta-20261111-1848.csv 20`.
- **`Acesso negado`:** outro programa está com a porta (o Teleplot ou um monitor). Feche-o.

## O que esperar

| Estímulo | SGP30 (gás) | SPS30 (partícula) | BME280 |
|---|---|---|---|
| vapor | `tvoc` sobe um pouco, os crus caem um pouco | `nc0_5`, `pm2_5` e `pm10` sobem | `umid` sobe muito |
| chama | `tvoc` sobe um pouco, `raw_h2` cai | não muda | `temp` sobe |
| fumaça da vela | `tvoc` sobe, os crus caem | `nc0_5` e `pm2_5` sobem | não muda |
| álcool | `tvoc` sobe, os crus caem muito | não muda | não muda |

- **O vapor no teste de 11/11/2026:** `nc0_5` de 72 para 2.225 com a chaleira de lado e um
  pouco abaixo, já fervendo mais fraco (e 8.294 com a placa em cima dela, logo depois de
  ferver: não repita), `umid` de 52 para 81%, `temp` +1 °C, `tvoc` de 110 para 331, `raw_h2` −3%. O SGP30 mexe pouco porque a umidade também
  muda a resistência dele. As gotas são maiores que a fumaça: o `pm10` fica 1,6 vez o
  `pm2_5` (na fumaça, de 1 a 1,4). E somem em 20 s, porque evaporam.
- **A chama no teste de 11/11/2026:** `temp` de 28,8 para 32,2 °C, `tvoc` de 0 para 423,
  `raw_h2` −7%, e o `nc0_5` parado. O gás é parecido com o da fumaça; o que separa as duas é
  a partícula. Do álcool, a chama se separa pelo calor.
- **`raw_h2` e `raw_ethanol` são resistência:** caem quando o gás sobe.

### Quando apertar `ar` e quanto esperar

Aperte `ar` com `há 1:00` no painel. Durante esse minuto, acompanhe uma curva por
estímulo:

| Estímulo | Curva | Espere no `ar` |
|---|---|---|
| vapor | `umid` e `nc0_5` (sobem) | 2 min: o `nc0_5` volta em segundos, mas o `pm2_5` e o `tvoc` levam uns 2 min |
| chama | `temp` (sobe) | 2 a 3 min: a placa guarda calor, e a `temp` volta devagar |
| fumaça | `nc0_5` (sobe); confira também o `tvoc` | 3 a 5 min, com a porta aberta e abanando |
| álcool | `raw_ethanol` (cai) | 5 min ou mais; é o último, então este `ar` fecha a coleta |

- **A curva reage atrasada:** começa a mudar uns 15 s depois da ação, e o pico vem 20 a 40 s
  depois. Com uma ação só, a curva sobe e começa a descer dentro do minuto: é o esperado.
- **Se a curva não mexeu em 1 minuto:** a dose foi pouca. Anote e acerte na próxima.
- **Voltou quando:** a curva está plana há uns 30 s e perto do valor de antes do estímulo.
  Não precisa ser o mesmo número: a linha de base muda um pouco ao longo do tempo.
- **Não decida pelo `tvoc`:** depois do álcool, ele fica alto por vários minutos e
  atrasado em relação aos crus. O `raw_ethanol` mostra a volta de verdade.
- **De onde vêm os tempos:** das coletas de 10 e 11/11/2026. Depois do álcool, o
  `raw_ethanol` levou uns 5 min para chegar perto do normal. Depois da fumaça forte (vela
  repetida a cada 15 s), 4 min de `ar` não bastaram: o `nc0_5` ainda estava dez vezes acima
  do começo.

## A coleta do professor

[`coleta_professor/`](coleta_professor/) tem a coleta de 11/11/2026, com o protocolo
inteiro uma vez: o `.csv`, o `.png` da coleta inteira e o `-estimulos.png`, a partir de
2.500 s, para o slide.

| Estímulo | Começa em | O que mexeu |
|---|---|---|
| vapor | 2.625 s | `nc0_5` até 807, `umid` +4 |
| chama | 3.124 s | `temp` de 29,1 para 31,2 °C, `tvoc` até 267 |
| fumaça | 3.701 s | `nc0_5` até 883 |
| álcool | 4.208 s | `tvoc` até 21.526, `raw_h2` −14%, `raw_ethanol` −12% |

- **Os primeiros 2.600 s são ar com a placa esquentando:** `temp` de 26 para 31 °C, `umid`
  de 57 para 42%, e os crus do SGP30 subindo, com picos. Para o dataset, use a partir de
  2.500 s: o ar do aquecimento é diferente do ar entre os estímulos, e o modelo aprenderia
  o momento da coleta.
- **O rótulo é o aperto do BOOT, não a chegada do estímulo:** a curva começa a mudar 15 a
  30 s depois, e a fumaça ainda aparece nos primeiros 100 s do `ar` seguinte.
- **Falta a segunda coleta**, em outro dia, para testar o modelo.

## Em sala

Sala com detector de incêndio não recebe chama nem fumaça.

- **Ao vivo:** ar, vapor e álcool. Nenhum deles tem chama. O vapor também dispara detector
  de fumaça (é o alarme falso), então a chaleira fica longe do detector do teto.
- **A chama e a fumaça:** em vídeo deste teste, filmando a caixa e o painel juntos, ou
  repetidas no próprio painel com `--replay`. O `.csv` da mesma gravação vai para o
  notebook, e a tabela de cima sai com os dados reais.

## O que aparece

As variáveis da leitura isolada (`tvoc`, `eco2`, `raw_h2`, `raw_ethanol`, `pm2_5`, `pm10`,
`nc0_5`, `temp`, `umid`) e mais uma:

| Variável | O que é |
|---|---|
| `estimulo` | 0 = ar, 1 = álcool, 2 = vapor, 3 = fumaça, 4 = chama |

No painel do `coleta.py`, o `estimulo` vira a cor de fundo de todos os gráficos (verde =
ar, roxo = vapor, azul = chama, vermelho = fumaça, amarelo = álcool): dá para ver cada
curva mudando logo depois da troca de cor.

## O que mudou em relação ao app21-1

- **Botão BOOT e a `SEQUENCIA` de estímulos:** linhas marcadas com `// ESTIMULO`.
- **LED:** mostra a cor do estímulo, com as mesmas cores das classes do coletor da trilha
  IMU (verde, amarelo, roxo, vermelho), mais o azul da chama.
- **Falta sensor:** o programa para e repete o aviso a cada 3 s, com o LED vermelho
  piscando, porque o teste precisa dos 3 sensores e vermelho fixo é fumaça. No app21-1, os
  sensores que iniciaram continuam aparecendo.
