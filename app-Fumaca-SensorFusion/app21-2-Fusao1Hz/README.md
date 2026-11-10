# app21-2-Fusao1Hz

Os 3 sensores da caixa de fumaça numa linha só, 1 linha por segundo. Uma linha é um
exemplo do dataset. No app21-3, a mesma linha vira o JSON publicado no MQTT.

**Pronto quando:** o Serial imprime o cabeçalho e depois 1 linha por segundo.

## O que acontece

1. **Liga:** o programa confere os 3 sensores. Se falta algum, o LED fica **vermelho** e o
   Serial diz qual (rode o app21-0).
2. **Aquecimento (20 s, LED amarelo):** lê os sensores, mas não imprime.
   - O SGP30 fica fixo em `tvoc` = 0 e `eco2` = 400.
   - O ventilador do SPS30 ainda está estabilizando.
3. **Medindo (LED verde):** cabeçalho + 1 linha por segundo.

```
amostra,pm1_0,pm2_5,pm10,nc0_5,nc1_0,nc2_5,raw_h2,raw_ethanol,tvoc,eco2,temp,umid
1,3.1,4.0,4.4,21.7,25.0,25.3,13110,19980,35,410,24.8,52.1
```

Os nomes são os do payload do `GUIA-CODIGO-SENSOR-FUSION.md` e ficam próximos aos do
dataset do Kaggle, de propósito.

## Decisões

- **1 Hz:** o SGP30 precisa ser lido a cada 1 s, e o SPS30 tem leitura nova a cada 1 s.
- **Umidade:** o BME280 informa a umidade ao SGP30 a cada leitura, e o SGP30 corrige o gás.
- **Pressão fica de fora:** no dataset do Kaggle, a pressão sozinha acertava 94% por
  vazamento (ver `LICOES-APRENDIDAS.md`).
- **Linha de base do SGP30:** não é salva nem restaurada. TVOC e eCO2 de um dia não se
  comparam com os de outro; isso entra como data drift no app21-3.
