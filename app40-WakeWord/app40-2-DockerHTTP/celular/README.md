# Wake word no celular: microfone do celular, modelo no computador

```text
celular ──HTTPS──► túnel ──► servidor.py ──POST ~24000 amostras──► container :1337
```

O celular só captura o áudio. O container é o modelo do Edge Impulse (MFCC + rede). O
`servidor.py` monta a janela, manda ao container e decide.

**Passo a passo:** [ROTEIRO.md](ROTEIRO.md). Notebook com microfone? A pasta
[`../notebook/`](../notebook/) é mais simples.

## Antes de começar

- Docker Desktop aberto e Python 3.12.
- Celular com internet; não precisa estar na mesma rede do computador.
- No Edge Impulse, a classe da palavra se chama exatamente **`WakeWord`**.
- A chave de API do comando `docker run` é **sua**: não publique.

## Por que o túnel

O celular só libera o microfone em página **HTTPS**. O túnel do Cloudflare dá ao
`servidor.py` (`localhost:8000`) um endereço HTTPS público, sem conta. O endereço muda
a cada execução, e qualquer um com o link abre a página: feche ao terminar.

## Conferir o modelo

Com o container rodando, abra <http://localhost:1337/api/info> e confira:

| Campo | Esperado | Onde |
|---|---|---|
| `frequency` | ~`16000` | `TAXA` no `celular.html` |
| `input_features_count` | ~`24000` | `JANELA` no `servidor.py` (lido sozinho) |
| `labels` | contém `WakeWord` | `CLASSE` no `servidor.py` |

## Como funciona

| Arquivo | O que faz |
|---|---|
| `celular.html` | pede o áudio em 16 kHz, converte para int16, manda pelo WebSocket e chama `wakeWord()` quando o servidor avisa |
| `servidor.py` | monta a janela de 1,5 s, chama o container a cada 0,5 s e devolve as probabilidades |

Taxa e escala têm de bater com o treino. Se divergir, o modelo não dá erro: só
responde errado.

## Limiar

`LIMIAR = 0.80` no `servidor.py`. Dispara sozinho → suba. Não reage → desça.

## Problemas comuns

| Sintoma | Causa |
|---|---|
| `ConnectionError` no servidor | container parado ou ainda compilando |
| `KeyError: 'WakeWord'` | a classe tem outro nome; troque `CLASSE` |
| **Ouvir** não faz nada | página aberta por `http://`; use o `https://` do túnel |
| `502 Bad Gateway` | `servidor.py` não está rodando na porta 8000 |
| `Conexão com o servidor fechada.` | servidor ou túnel caiu |
