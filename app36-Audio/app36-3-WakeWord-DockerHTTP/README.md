# Wake word com Docker como motor de inferência

Você treinou a sua wake word no Edge Impulse. Aqui ela roda num container Docker no
seu computador. Escolha quem vai ouvir:

| Pasta | Microfone | Para quem |
|---|---|---|
| [`notebook/`](notebook/) | do computador | tem notebook com microfone |
| [`celular/`](celular/) | do celular, por uma página HTTPS | o computador não tem microfone (laboratório) |

Nas duas, o container é o **impulse inteiro**: o MFCC e a rede neural, com o mesmo
código do treino. O Python só recebe o áudio, monta a janela e decide o que fazer com
a resposta.

Cada pasta é independente: tem o seu `README.md`, o seu `ROTEIRO.md` (passo a passo
curto) e o seu `requirements.txt`.
