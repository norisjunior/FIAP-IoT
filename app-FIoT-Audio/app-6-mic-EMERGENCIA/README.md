# app-6-mic-EMERGENCIA — o comando EMERGÊNCIA (a fazer)

Etapa 6, **fora do roteiro da turma**. Pasta reservada: só `.gitignore` e `LICENSE` por
enquanto.

A trilha até a etapa 5 roda inteira com o modelo **"Ei Fioti"** que já está treinado — três
classes, nenhuma gravação nova. Esta etapa é o passo seguinte, para quando houver tempo de
voltar ao Studio: acrescentar a palavra de comando **EMERGÊNCIA**, que só vale com o
dispositivo já ATIVO.

## O que fazer quando chegar a hora

1. **Projeto EI novo** (não retreinar o `WakeWord-EiFIOT`): classes `WakeWord`, `emergencia`,
   `desconhecido`, `ruido`. O projeto atual continua servindo as etapas 3 a 5 sem mexer.
2. Gravar `emergencia` pelo celular, **sample length 2000 ms**, mesmo protocolo do
   [ETAPA1-COLETA.md](../android-wakeword/ETAPA1-COLETA.md) — a palavra tem 5 sílabas e passa
   de 1 s falada em ritmo normal, por isso a janela de **1500 ms** é obrigatória aqui.
3. Mesmo impulse das outras etapas: MFCC → Classification, janela 1500 ms, stride 500 ms,
   16 kHz. Deployment **Arduino library**, quantized (int8).
4. Firmware = o da [app-5-mic-ASSISTENTE](../app-5-mic-ASSISTENTE/) mais um ramo na máquina de
   estados: com o dispositivo ATIVO, `emergencia` acima do limiar dispara o buzzer (GPIO 19) e
   publica no MQTT.

## Por que depois do assistente, e não antes

Trocar de modelo custa uma coleta inteira. A etapa 3 existe para o aluno ver a wake word
funcionando **no mesmo dia**, com o modelo pronto; o comando é enriquecimento, não pré-requisito.
