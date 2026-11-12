# app16-1-LeituraIsolada

Cada sensor da caixa de fumaça sozinho, no plotter, 1 leitura por segundo. É o "ver o dado
cru" da caixa, como o app17-2 é para o MPU.

**Pronto quando:** o álcool mexe só no SGP30, e o vapor mexe no SPS30 e na umidade.

## Antes

- O app16-0 mostrou os 3 sensores ok (ligação no README da pasta de cima).
- **Teleplot** instalado no VS Code. O formato `>nome: valor` é o mesmo do app17-2. Cada
  variável ganha um gráfico próprio.
- O LED RGB da placa fica **verde** quando os 3 sensores iniciaram e **vermelho** quando
  falta algum. Os sensores que iniciaram continuam aparecendo.

## O que aparece

| Sensor | Variável | O que é |
|---|---|---|
| SGP30 | `tvoc`, `eco2` | gás calculado pelo sensor (ppb e ppm) |
| | `raw_h2`, `raw_ethanol` | valor cru; **cai** quando o gás sobe, porque é resistência |
| SPS30 | `pm2_5`, `pm10` | massa de partículas (µg/m³) |
| | `nc0_5` | contagem de partículas finas (por cm³) |
| BME280 | `temp`, `umid` | temperatura (°C) e umidade relativa (%) |

- **Aquecimento do SGP30:** nos primeiros ~15 s ele mostra `tvoc` = 0 e `eco2` = 400. É
  normal.
- **Sem compensação de umidade:** o SGP30 lê sozinho aqui. A correção pela umidade do
  BME280 entra no app16-3.

## Próximo

O app16-2 (`app16-2-TesteFenomenos/`) repete este plotter com o botão BOOT marcando o
estímulo (ar, vapor, chama, fumaça, álcool), para o teste com fenômenos físicos.
