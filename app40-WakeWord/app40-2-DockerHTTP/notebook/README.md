# Wake word no notebook: microfone do computador

```text
microfone ──► wakeword.py ──POST ~24000 amostras──► container :1337 ──► {WakeWord: 0.93, ...}
```

O container é o modelo do Edge Impulse (MFCC + rede). O `wakeword.py` captura o áudio,
manda a janela e decide.

**Passo a passo:** [ROTEIRO.md](ROTEIRO.md). Computador sem microfone? Use
[`../celular/`](../celular/).

## Antes de começar

- Docker Desktop aberto e Python 3.12.
- No Edge Impulse, a classe da palavra se chama exatamente **`WakeWord`**.
- A chave de API do comando `docker run` é **sua**: não publique.

## Conferir o modelo

Com o container rodando, abra <http://localhost:1337/api/info> e confira:

| Campo | Esperado | No `wakeword.py` |
|---|---|---|
| `frequency` | ~`16000` | `TAXA` |
| `input_features_count` | ~`24000` | `JANELA` (lido sozinho) |
| `labels` | contém `WakeWord` | `CLASSE` |

## Como funciona

A janela tem 1,5 s e anda de 0,5 em 0,5 s, para a palavra caber inteira em alguma
janela. O áudio vai em **int16** e **16 kHz**, como no treino. Se divergir, o modelo
não dá erro: só responde errado.

## Limiar

`LIMIAR = 0.80`. Dispara sozinho → suba. Não reage → desça.

## Problemas comuns

| Sintoma | Causa |
|---|---|
| `Connection refused` | container parado ou ainda compilando |
| `KeyError: 'WakeWord'` | a classe tem outro nome; troque `CLASSE` |
| `port is already allocated` | outro container na 1337: `docker ps` e `docker stop <id>` |
| tudo sai `ruido` | microfone errado ou mudo no Windows |
