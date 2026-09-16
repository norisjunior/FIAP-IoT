# Etapa 1 — Coleta no Edge Impulse

Wake word: **"Ei Fioti"**
Sample length: **2000 ms** · Frequência: **16 kHz**

## Classes

Labels exatas no Edge Impulse: `WakeWord`, `desconhecido`, `ruido`

| Label | Amostras | O que gravar |
|---|---|---|
| `WakeWord` | 80 | A wake word |
| `desconhecido` | 80 | Outras palavras, incluindo "fiot", "fiap", "ei" sozinho |
| `ruido` | 60 | Sala vazia, ar-condicionado, passos, cadeira, digitação |

## Como variar as 80 do `WakeWord`

- 20x tom normal, 30 cm do celular
- 20x mais longe, ~1,5 m
- 15x falando rápido
- 15x falando devagar / arrastado
- 10x com ruído de fundo (música baixa, conversa)

Variar também:
- Entonação (afirmativa, perguntando, animado)
- Posição do celular (mesa, mão, bolso da camisa)
- Peça para 2–3 outras pessoas gravarem algumas

## `desconhecido` — o que não pode faltar

- "fiot", "fioti" sozinho, "fiap", "ei", "oi"
- Palavras da sua aula: "sensor", "dashboard", "microcontrolador", "projeto"
- Contar de 1 a 20
- Ler um parágrafo qualquer em voz alta

Isso é o que ensina o modelo a **não** disparar.

## Gravações longas

Para `desconhecido` e `ruido`, grave corrido e deixe o Edge Impulse fatiar.

- Sample length: `60000` ms (1 min) ou mais
- Depois: clique na amostra -> `...` -> **Split sample** -> `2000` ms
- A amostra nunca pode ser menor que a janela de 1500 ms

Ruido: nao use so um clipe de YouTube. Misture com sala real em silencio,
corredor, ar-condicionado de verdade. Senao o modelo aprende o clipe.

## Passos no Edge Impulse

- **Devices** → **Connect a new device** → **Use your mobile phone** → QR code
- **Data acquisition** → aba do celular
- Campo **Label**: escrever o nome da classe
- **Sample length**: `2000` ms
- **Start sampling** → falar → repetir
- Trocar o Label ao mudar de classe

## Fechamento

- **Data acquisition** → menu `⋮` → **Perform train/test split**
- Confirmar ~80/20
- Conferir o balanço das classes no gráfico de pizza

## Dicas

- Fale a frase no **meio** da janela de 2 s, não no comecinho
- Grave em **2–3 ambientes diferentes** (sala, corredor, casa)
- Não grave tudo de uma vez — modelo fica viciado no seu tom daquele dia

---

**Plano no `FIAP-IoT-develop`** (este repositório) ·
**Implementação no `FIAP-IoT-eval`**, em
`app40-WakeWord/app40-1-SmartphoneAndroid`
