# Fase 3 — Perguntar em português

A Fase 2 fez a máquina decidir. Esta fase deixa **uma pessoa perguntar**.

Um agente no n8n recebe a pergunta em linguagem natural, decide sozinho chamar
a API de predição e responde em português.

## Importar

Com o compose da Fase 2 rodando, importe
`Fluxos-n8n-LLM/LLM-VaccineSense-Chat.json` no n8n.

Configure a credencial do modelo de linguagem no nó **Modelo de linguagem**.

## Os cinco nós

| Nó | Papel |
|---|---|
| `chatTrigger` | a janela de conversa |
| `agent` | decide o que fazer com a pergunta |
| `lmChatOpenAi` | o modelo de linguagem |
| `memoryBufferWindow` | lembra das últimas mensagens |
| `toolHttpRequest` | **a ferramenta**: chama `/predict` |

A ferramenta é o ponto da fase. O agente não sabe classificar carga — ele sabe
**quando chamar quem sabe**.

## Perguntas para testar

```
A carga está bem com 7 graus e criticidade 75?
E se a criticidade fosse 20?
A caixa está a 5 graus mas a luz interna está em 3100. O que houve?
```

Na terceira, o agente deve responder `CARGA_EM_PERIGO` e explicar que a luz alta
indica tampa aberta — isso vem do texto de contexto do agente, não do modelo de
ML.

## O que discutir

**O agente não substitui o modelo.** Ele traduz a pergunta, chama a ferramenta
certa e traduz a resposta. Quem classifica continua sendo a árvore de decisão
treinada com os dados da caixa.

Vale perguntar à turma: *o que acontece se o agente responder sem chamar a
ferramenta?* Ele vai chutar — com confiança. É por isso que a descrição da
ferramenta importa tanto quanto o modelo.
