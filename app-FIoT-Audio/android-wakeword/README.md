# Wake Word no Android — "Ei Fioti"

Trilho irmão do `app-FIoT-Audio` (ESP32 + INMP441), rodando no celular.
Mesmo modelo do Edge Impulse, mesmo MFCC, outro alvo.

> **Plano e documentação:** aqui, neste repositório (`FIAP-IoT-develop`).
> **Implementação:** `C:\Projects\FIAP-IoT-eval\app40-WakeWord\app40-1-SmartphoneAndroid`
> (repositório `FIAP-IoT-eval`, branch `fiapiot/eval`)

## Documentos

| Arquivo | O que é |
|---|---|
| [PLANO.md](PLANO.md) | As etapas do projeto |
| [AUDITORIA.md](AUDITORIA.md) | O que está feito e o que falta |
| [ETAPA1-COLETA.md](ETAPA1-COLETA.md) | Roteiro de gravação do dataset |
| [AULA-WakeWord.md](AULA-WakeWord.md) | Slides da aula + instruções de montagem no PowerPoint |

## Resumo do que foi construído

- Edge Impulse: MFCC + CNN, janela de 1,5 s a 16 kHz, 3 classes
- Exportado como **Android library (C++)**, float32 + EON
- App Flutter com ponte **Dart FFI** para o SDK nativo
- Inferência: **2 a 4 ms** no Galaxy A52
- Ao detectar, captura 5 s do acelerômetro e volta

Para compilar, veja o `README.md` do lado da implementação.
