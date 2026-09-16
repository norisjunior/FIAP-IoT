# Wake Word no Android — Plano

Coleta e treino: Edge Impulse (navegador, sem app).
App final: Flutter + Dart + VS Code, sem Android Studio.
Alvo: Galaxy A52 (SM-A525M, Android 14). Saída: `.apk`.

## Etapa 0 — Ambiente
- JDK 17
- Flutter SDK em `C:\dev\flutter`
- Android cmdline-tools em `C:\dev\android-sdk`
- Variáveis `ANDROID_HOME` + `Path`
- `flutter doctor` com Android toolchain verde
- Extensão Dart/Flutter no VS Code
- Celular em modo desenvolvedor + depuração USB

## Etapa 1 — Coleta no Edge Impulse
- Criar projeto
- Conectar o celular por QR code
- Gravar 50–100 amostras da wake word
- Gravar 50–100 de ruído e fala aleatória
- Dividir treino / teste

## Etapa 2 — Treino no Edge Impulse
- Impulse: MFCC + rede neural
- Treinar e avaliar acurácia
- Testar ao vivo pelo celular
- Exportar TensorFlow Lite

## Etapa 3 — App Flutter
- Projeto Flutter novo
- Permissão de microfone
- Embutir o modelo exportado
- Escuta contínua
- Ação visível ao detectar a wake word
- Gerar o `.apk`

## Extra — Acelerômetro
- Tela lendo `sensors_plus`
- Gráfico ao vivo

## Definições
- Wake word: **"Ei Fioti"** (label `WakeWord`)
- Sample length: 2000 ms · Window: 1500 ms

---

**Plano no `FIAP-IoT-develop`** (este repositório) ·
**Implementação no `FIAP-IoT-eval`**, em
`app40-WakeWord/app40-1-SmartphoneAndroid`
