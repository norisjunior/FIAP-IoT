# app17-5 — Coleta RAW pela Serial, com Python

> **Material de referência.** Não faz parte do roteiro das aulas: lá, a coleta é feita
> pelo próprio ESP32 (app17-7). Este app mostra a outra forma: guardar o sinal bruto e
> deixar o Python pôr o horário e a label.

O ESP32 imprime **só `ax,ay,az` a 100 Hz na Serial**, continuamente, em **g**. O
[`coletor_raw.py`](coletor_raw.py) lê essas linhas, acrescenta **timestamp** e **label** e
grava um CSV.

Guardar o raw permite, no Colab, **ver o sinal inteiro**, recortar o trecho exato do
fenômeno e testar outras janelas. As features prontas do app17-7 não permitem isso.

## Montagem

Só **ESP32 + MPU sobre a mesa**: sem botões, sem LED. A condição é escolhida no Python.

| MPU | ESP32 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO22 |
| SCL | GPIO23 |

## Como rodar

1. Grave o firmware (ou abra o simulador do Wokwi e deixe a aba **visível**).
2. **Feche o Serial Monitor**: só um programa abre a porta por vez.
3. Instale e rode o coletor, uma vez por rodada:

```bash
python -m pip install -r requirements.txt

python coletor_raw.py COM6 normal 60       # MPU parado
python coletor_raw.py COM6 anomalia 60     # chacoalhando o MPU
```

No Wokwi, a porta é `rfc2217://localhost:4000`: o [`wokwi.toml`](wokwi.toml) já expõe a
Serial.

Cada execução gera um arquivo `raw_<label>_<data_hora>.csv` com
`timestamp,ax,ay,az,label`.

## Protocolo

- **Uma execução = uma rodada = uma condição estável.** Monte a condição antes de rodar.
- O coletor descarta os primeiros 5 s (você ainda está posicionando o sensor).
- ~1 min por rodada dá ~6000 leituras.
- Faça mais de uma rodada por condição: o treino e o teste se dividem por rodada.

## Limitação

O **timestamp é o do computador**, na hora em que a linha chegou, e não o instante exato da
medição no ESP32: há atraso da Serial/USB.
