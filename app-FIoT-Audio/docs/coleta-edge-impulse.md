# Coletar o dataset de voz pelo celular (Edge Impulse)

O dataset das etapas 4 a 6 é gravado **pelo celular**, direto no Edge Impulse Studio. Nenhum
firmware de gravação, nenhum script Python, nenhum arquivo `.wav` na mão.

O ESP32 fica com o que a plataforma não dá: o sinal cru ao vivo (etapas 1 a 3) e a inferência
na borda (etapa 4).

---

## 1. O projeto

Studio → **Create new project** → tipo **Audio**.

Dois projetos, não um:

| Projeto | Classes | Quem usa |
|---|---|---|
| `fiot-wakeword` | `fiot` · `emergencia` · `ruido` · `desconhecido` | a turma (etapas 4 e 5) |
| `fiot-assistente` | + `alerta` · `localizacao` | demonstração (etapa 6) |

O aluno grava 2 palavras em vez de 4 e treina mais rápido. A versão estendida é "mais dados,
mesmo impulse" — ninguém precisa retreinar depois.

## 2. As classes

| Classe | O que gravar | Por que existe |
|---|---|---|
| `fiot` | a palavra FIOT | acorda o dispositivo |
| `emergencia` | a palavra EMERGÊNCIA | o comando |
| `ruido` | silêncio da sala, ventilador, cadeira, conversa ao fundo | sem isso o modelo classifica silêncio como palavra |
| `desconhecido` | outras palavras: "fiapo", "emergente", "alô", nomes, números | ensina o modelo a dizer **não** |

As duas últimas não são enchimento. Um modelo treinado só com as palavras-alvo **sempre**
responde uma delas — não tem alternativa para escolher.

## 3. Gravar

Studio → **Data acquisition** → **Show QR code** → aponte a câmera do celular. Abre uma página
web; nada para instalar.

Grave **uma faixa longa por classe**, não uma amostra por vez:

1. Escolha **Microphone**, comprimento **60 s**, label da classe.
2. Comece a gravar e **espere 2 s** antes da primeira palavra.
3. Repita a palavra ~25 vezes, com **1 s de pausa** entre elas, no mesmo ritmo da fala normal.
4. Mantenha o celular a **~30 cm** — a mesma distância em que o INMP441 vai ser usado.

> **Protocolo (Aula 14, slide 30).** Descartar a transição vale aqui também: os 2 s iniciais
> são você ajeitando o celular, e contaminam o dataset. E a label só é honesta se a condição
> valeu a gravação inteira — não grave `ruido` falando.

Para um modelo que funcione com a turma inteira, grave com **pelo menos 3 pessoas diferentes**,
e inclua vozes graves e agudas. Um modelo treinado só com a sua voz reconhece só a sua voz.

## 4. Split: 60 s viram 25 amostras

Na amostra gravada → menu **⋮** → **Split sample**.

| Campo | Valor |
|---|---|
| Segment length | **1500 ms** |
| Shift segments | deixe ligado |

O Studio detecta os picos de energia e coloca uma janela em torno de cada palavra. Confira as
janelas na tela e arraste as que ficaram fora do lugar antes de confirmar.

> **Por que 1500 ms e não os 1000 ms do padrão.** "EMERGÊNCIA" e "LOCALIZAÇÃO" têm 5 sílabas
> e, faladas em ritmo normal, passam de 1 segundo. Numa janela de 1000 ms a palavra entra
> cortada e o modelo aprende pedaço de palavra. O segmento aqui e a janela do impulse (§6)
> **precisam ser o mesmo número.**

## 5. Train / Test

**Data acquisition → Dashboard → Perform train/test split** (80/20). O Studio faz sozinho.

Meta por classe: **~50 amostras**. Menos que isso e a matriz de confusão vira ruído.

Aproveite para olhar a **forma de onda e o espectrograma** de cada amostra — o Studio mostra os
dois. É a mesma inspeção visual que a etapa 1 faz no Teleplot, agora com a frequência junto.

## 6. O impulse

**Create impulse:**

| Bloco | Configuração |
|---|---|
| Time series data | window size **1500 ms** · window increase **500 ms** · frequency **16000 Hz** |
| Processing | **Audio (MFCC)** |
| Learning | **Classification** |

Deixe os parâmetros do MFCC no padrão. **Generate features** e olhe o *Feature explorer*: se as
classes já aparecem separadas ali, o treino vai bem. Se estiverem embaralhadas, o problema é o
dataset — voltar e gravar mais, não mexer na rede.

**Training:** 100 ciclos, learning rate 0.005, e ligue **Data augmentation** (adiciona ruído e
deslocamento — ajuda justamente no descasamento de domínio da §8).

Se a acurácia decepcionar, rode o **EON Tuner** antes de mexer à mão.

## 7. Deploy

**Deployment → Arduino library → Build.** Baixa um `.zip`.

Descompacte dentro da pasta da etapa:

```text
app-4-mic-FIOT/
└── lib/
    └── fiot-wakeword_inferencing/     <- o conteúdo do .zip
        ├── src/
        └── library.properties
```

É a mesma convenção do `app33-MagicWand` no repositório principal. A biblioteca é grande e
**é versionada mesmo assim**.

O `platformio.ini` da etapa já traz `board_build.partitions = huge_app.csv` — sem isso o
binário estoura a partição padrão de 1,3 MB.

## 8. O risco que precisa ser dito em aula

**Você treinou com o microfone do celular e vai inferir com o INMP441.** Resposta em
frequência, ganho e distância são diferentes. Isso se chama **descasamento de domínio**, e é
exatamente a lição da Aula 14: *o dado de treino precisa parecer o dado de operação*.

Na prática, para uma wake word de sala de aula, costuma funcionar — o MFCC é razoavelmente
robusto e o augmentation ajuda. Mas é uma aposta consciente, não um detalhe.

Se a taxa de acerto no dispositivo decepcionar, em ordem de custo:

1. gravar com o celular à mesma distância de uso (~30 cm);
2. mais amostras de `ruido` gravadas **na sala onde a demo vai acontecer**;
3. ajustar o `GANHO_SHIFT` do INMP441 para o sinal ficar na mesma faixa do treino;
4. só então: gravar ~10 amostras com o próprio INMP441 e subir no Studio.
