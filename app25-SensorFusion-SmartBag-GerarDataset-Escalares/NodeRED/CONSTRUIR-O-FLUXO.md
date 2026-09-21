# Construir o fluxo do app19 no Node-RED, do zero

Duas iterações. Cada uma roda.

1. A amostra da bag chegando no debug.
2. A mesma amostra gravada no InfluxDB.

No app15 o Node-RED **decidia** — dois switches em série e um comando de volta para o
ESP32. Aqui ele não decide nada: o app19 é um **gerador de dataset**, e quem vai decidir
é o modelo, depois de treinado. O fluxo é só transporte.

Firmware do app19 gravado e publicando:
[CONSTRUIR-O-FIRMWARE.md](../device/CONSTRUIR-O-FIRMWARE.md).

Plataforma no ar: entre no diretório `IoT-platform` que você recebeu e rode

```bash
docker compose up -d
```

Sobem Mosquitto, Node-RED, n8n, InfluxDB e Grafana juntos. Editor do Node-RED em
http://localhost:1880, com **admin / FIAPIoT**. Para parar, `docker compose down` na
mesma pasta — nunca com remoção de volumes, ou os fluxos somem.

> Desative os fluxos do app14/app15 antes de ativar este. Eles assinam outro tópico, mas
> deixar três fluxos ligados no mesmo Node-RED só atrapalha a leitura do debug.

---

## Iteração 1 — Ver a amostra chegar

**a) Um nó `mqtt in`.**

| Campo | Valor |
|---|---|
| Server | `mosquitto`, porta `1883` — não `localhost` |
| Topic | `FIAPIoT/smartbag/equipe01/dados` |
| QoS | `0` |
| Output | `a String` |
| Name | `Dados da bag` |

`mosquitto` e não `localhost` porque o Node-RED e o broker são dois containers na mesma
rede. Se você subiu o Node-RED fora do Docker, aí sim é `localhost`.

**b) Um nó `json`**, ligado na saída dele. Property `payload`, Action `Always convert to
JavaScript Object`. Name `JSON para objeto`.

O `mqtt in` entrega texto. O `json` transforma em objeto para os nós seguintes lerem
`msg.payload.temperatura` em vez de fatiar string.

**c) Um nó `debug`**, ligado no `json`. Output `msg.payload`, Name `Objeto recebido`.

Deploy. No Wokwi, aperte **COLETA**.

**Funcionou?** Uma mensagem por segundo na aba debug, cada uma com nove chaves:

```json
{ "device": "SmartBagEquipe01", "rodada": 1, "situacao": "parada_fechada",
  "temperatura": 24.3, "umidade": 40, "delta_distancia": 0.12,
  "luz": 2847, "mov_max": 0.34, "incl_max": 2.1 }
```

- [ ] Chega **uma** mensagem por segundo, não mais
- [ ] Para de chegar quando você aperta COLETA de novo
- [ ] `situacao` muda quando você aperta SITUAÇÃO com a coleta parada

| Deu errado | Onde olhar |
|---|---|
| Nada chega | O ESP32 **só publica durante a coleta**. LED apagado = nada no tópico |
| Nada chega, LED aceso | Tópico: `smartbag`, não `nexolog`. E o Serial mostra `[MQTT] Amostra nao enviada`? |
| Erro na credencial | `mosquitto`, não `localhost` |
| Chega texto com aspas escapadas | Faltou o nó `json`, ou a Action dele não é `obj` |
| `delta_distancia` é `null` | Sem eco no HC-SR04, ou o baseline falhou. É esperado e o Colab remove essas linhas |

---

## Iteração 2 — Gravar no InfluxDB

Instale `node-red-contrib-influxdb` pelo **Manage palette**, se ainda não tiver.

**a) Um nó `function`**, ligado também no `json` (em paralelo com o debug — o debug
continua útil). Name `Campos e tags`:

```javascript
// device, rodada e situacao saem dos campos e viram tags. Mandar nos dois
// lugares e conflito de schema, como no app15.
const {device, rodada, situacao, ...campos} = msg.payload;
msg.payload = [campos, {device, rodada: String(rodada), situacao}];
return msg;
```

É a **mesma função do app15**, com duas tags a mais. O `influxdb out` espera
`[campos, tags]`: o primeiro objeto vira os valores medidos, o segundo vira as etiquetas
pelas quais você filtra depois.

`rodada` vai como texto porque tag no InfluxDB é sempre string. Se for como número, o
Influx converte de qualquer jeito — mas aí o `String()` fica implícito, e implícito é o
que quebra no meio da coleta.

**Por que `rodada` e `situacao` são tags e não campos?** Porque é por elas que o Colab
vai agrupar e filtrar. Campo é o que você mede; tag é como você acha o que mediu. E
`situacao` é justamente o que vira o `target` — ela precisa estar indexada.

**b) Um nó `influxdb out`**, ligado na função. Name `Gravar amostras`.

| Campo | Valor |
|---|---|
| Measurement | `smartbag_raw_adc_2026` |
| Organization | a sua |
| Bucket | o seu |

No lápis do servidor: Version `2.0`, URL `https://<sua-regiao>.aws.cloud2.influxdata.com`,
Token o seu, de **Load Data > API Tokens**.

O token **não vem no arquivo importado** — o Node-RED guarda token como credencial e a
exportação sempre remove. Cada aluno cola o dele.

> **É o InfluxDB Cloud, não o da IoT-platform.** A plataforma sobe um `influxdb:2.7`
> local, e ele serve muito bem para dashboard — mas o Colab de coleta consulta com
> **SQL**, pelo `InfluxDBClient3`, e essa API só existe no InfluxDB v3 (Cloud). Apontar
> este nó para `influxdb:8086` grava normalmente e só quebra lá na frente, no Colab, com
> um erro que não menciona versão nenhuma. Da `IoT-platform`, o app19 usa o Mosquitto e o
> Node-RED.

Deploy e colete de novo.

- [ ] O nó `Gravar amostras` fica verde, sem mensagem de erro
- [ ] No InfluxDB, `SELECT * FROM smartbag_raw_adc_2026` traz as amostras
- [ ] `rodada` e `situacao` aparecem como tags, não como colunas de valor

**Campo `null` é simplesmente pulado na escrita** — as outras cinco features do mesmo
segundo entram normalmente. Isso foi conferido no app15; a função não precisa filtrar
nada.

| Deu errado | Onde olhar |
|---|---|
| `batch schema conflict` | `device`, `rodada` ou `situacao` foi junto nos **campos** e nas tags. Confira o `...campos` |
| Grava, mas o Colab não acha | Measurement errado. É `smartbag_raw_adc_2026`, que separa esta coleta em RAW dos ensaios antigos em lux |
| `unauthorized` | Token colado no lugar errado, ou sem permissão de escrita no bucket |

---

## Por que uma measurement nova

`smartbag_raw_adc_2026` não é enfeite. A feature `luz` deste app é **RAW de 0 a 4095**,
lida direto com `analogRead()`. Os ensaios anteriores gravaram **lux**, convertido. São
escalas diferentes e até sentidos diferentes — no módulo usado, mais luz **reduz** o RAW.

Misturar as duas numa measurement só produz um CSV que treina um modelo sem sentido, e
o erro não aparece em lugar nenhum: são números plausíveis nas duas escalas. Por isso a
separação é no nome da measurement, não numa convenção que alguém precisa lembrar.

## Depois da coleta

Duas rodadas completas das sete situações, sem reiniciar o ESP32 — o protocolo está no
[Guia-SmartBag.md](../Guia-SmartBag.md). Depois,
[app19_coleta_e_rotulagem.ipynb](../colab/app19_coleta_e_rotulagem.ipynb) consulta esta
measurement por SQL, rotula pela situação e exporta o CSV.
