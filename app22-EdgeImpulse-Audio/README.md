# Edge Impulse e áudio: do som à palavra que acorda

O microfone digital entrega 16.000 números por segundo. As features da janela (média, desvio,
pico a pico e cruzamentos por zero) dizem **quanto** som houve e **quão agudo** ele era. Elas
não dizem que palavra foi dita: as contas não guardam a ordem dos sons. Por isso o MFCC
transforma a janela numa **imagem** (o tempo no eixo horizontal, as frequências no vertical), e
uma rede neural pequena aprende a reconhecer essa imagem.

A aula tem duas partes:

- **a teoria, com o microfone do professor** (INMP441 na ESP32-S3): a onda no Teleplot e as 4
  features ao vivo;
- **a prática, toda no Edge Impulse, com o celular como microfone**: cada grupo grava, treina
  e testa a própria palavra, e a vê acordar no navegador do celular.

| Pasta | O que é | Quem roda |
|---|---|---|
| [`mic-onda/`](mic-onda/) | a onda do INMP441 no Teleplot | o professor, na teoria |
| [`mic-features/`](mic-features/) | a janela de 256 ms vira 4 números: `calcMean`, `calcStd`, `calcPtP`, `calcZCR` | o professor, na teoria |
| [`wakeword-notebook/`](wakeword-notebook/) | a palavra treinada num container Docker, com o microfone do notebook | opcional: quem terminar antes, ou em casa |
| [`wakeword-celular/`](wakeword-celular/) | o mesmo container, com o microfone do celular por uma página HTTPS | opcional: o computador sem microfone |

Os dois programas do microfone usam a S3 no `platformio.ini`. Para a DevKit v1, use o arquivo
ao lado (`platformio.ini.devkit1` na onda, `platformio.ini.esp32devkit` nas features). O
[`WAKEWORD-DOCKER.md`](WAKEWORD-DOCKER.md) ajuda a escolher entre as duas pastas do Docker.

---

## A prática: a palavra do grupo (75 min)

**Chega com:** um celular com internet por grupo. A rede `NorisIoT` não tem internet: use a
rede da faculdade ou o 4G.

### 1. A conta e o projeto (10 min)

1. Um colega cria a conta gratuita em [studio.edgeimpulse.com](https://studio.edgeimpulse.com)
   e um projeto com o nome da palavra do grupo.
2. **Devices → Connect a new device → Use your mobile phone.** Cada colega lê o QR com o
   celular.

Pronto quando os celulares aparecem em **Devices**.

> O QR leva a chave do projeto. Não publique o QR nem a chave (nada de print no grupo da
> turma, nada de chave no GitHub).

### 2. Gravar as 3 classes (25 min)

No celular, em **Data collection**: **Label**, **Length**, **Category** e **Start recording**.

| Label | O que gravar | Como | Category |
|---|---|---|---|
| `WakeWord` | a palavra do grupo | cada colega grava 60 s, dizendo a palavra a cada 2 ou 3 s. No Studio, em **Data acquisition**, no ⋮ da gravação: **Split sample**, amostras de 2000 ms (uns 20 por gravação) | a voz de dois colegas em **Training**; a do terceiro em **Test** |
| `desconhecido` | outras palavras: conversa, palavras parecidas com a de vocês, a palavra dos outros grupos | 2 gravações de 30 s, sem dividir | uma em **Training**, uma em **Test** |
| `ruido` | a sala sem ninguém falando com o celular: ventilador, corredor, cadeiras, teclado | 2 gravações de 30 s, sem dividir | uma em **Training**, uma em **Test** |

- O teste honesto é uma **voz que o modelo não ouviu**. Grupo de 2: cada colega põe uma das
  gravações da palavra no teste.
- A amostra nunca é menor que a janela: 2 s de amostra, 1,5 s de janela.
- Sem as outras duas classes, o modelo responde sempre a palavra.

Pronto quando a palavra tem uns 60 amostras de 2 s, e `desconhecido` e `ruido` têm 2
gravações de 30 s cada.

### 3. O impulse e as features (10 min)

1. **Impulse design → Create impulse:** Window size **1500 ms**, Window increase **500 ms**.
2. **Add a processing block:** Audio (MFCC). **Add a learning block:** Classification.
   **Save Impulse.**
3. **MFCC:** Save parameters (o padrão serve) → **Generate features**.

Pronto quando o **Feature explorer** aparece: cada ponto é uma janela; as cores são as
classes.

### 4. Treinar (10 min)

**Classifier:** Number of training cycles **200**, **Data augmentation** ligado →
**Save & train**.

Pronto quando aparecem a acurácia e a matriz de confusão. Anote as duas.

### 5. Testar: a voz nova e o celular (15 min)

1. **Model testing → Classify all:** o acerto com a voz que o modelo não ouviu.
2. **Deployment → Run this model:** leia o QR com o celular (ou **Launch in browser**). O
   navegador baixa o modelo e roda ali mesmo.

Anote: o acerto no teste; quantas de 10 vezes a palavra acordou; quantos alarmes falsos em
1 minuto de conversa.

### Se der errado

| Sintoma | O que fazer |
|---|---|
| O celular sumiu de **Devices** | recarregue a página no celular, ou leia o QR de novo |
| O Studio reclama de amostra mais curta que a janela | divida em 2000 ms, nunca menos que 1500 ms |
| A acurácia é alta, mas no celular a palavra não acorda | fale a palavra no meio de cada amostra; grave mais vozes |
| Acorda sozinho | grave mais `desconhecido` e mais `ruido` |
| O navegador não libera o microfone | permita o microfone para o site; o endereço tem que ser `https://` |

**Terminou antes:** a palavra no notebook, com o Docker ([`wakeword-notebook/`](wakeword-notebook/)).

**Na próxima aula:** a palavra na placa.
