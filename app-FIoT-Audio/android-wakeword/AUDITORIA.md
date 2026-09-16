# Auditoria

Legenda: `[ ]` a fazer · `[~]` em andamento · `[x]` feito

Atualizado em: 2026-09-16

## Etapa 0 — Ambiente
- [x] JDK 17 instalado
- [x] Flutter SDK em `C:\dev\flutter` (3.47.4 stable)
- [x] cmdline-tools em `C:\dev\android-sdk\cmdline-tools\latest`
- [x] `ANDROID_HOME` e `Path` configurados
- [x] `sdkmanager` — platform-tools, android-36, build-tools 36
- [x] `flutter doctor --android-licenses` aceito
- [x] `flutter doctor` com Android toolchain verde
- [x] Extensão Dart-Code.flutter no VS Code (3.142.0)
- [x] Galaxy A52 (SM-A525M, Android 14) com depuração USB ligada
- [x] A52 aparece em `flutter devices`
- [x] APK de exemplo compilado e instalado no A52

Ignorar: Visual Studio (só para apps Windows desktop).

## Etapa 1 — Coleta no Edge Impulse
- [x] Wake word definida: "Ei Fioti"
- [x] Projeto criado (WakeWord-EiFIOT)
- [x] Celular conectado por QR code
- [x] Amostras WakeWord (131 janelas)
- [x] Amostras desconhecido (144) e ruido (158)
- [x] Split treino / teste

## Etapa 2 — Treino no Edge Impulse
- [x] Impulse: MFCC, janela 1500ms, stride 500ms, 16kHz
- [x] Modelo treinado — 93.1% val, ROC 0.98, augmentation ON, 200 ciclos
- [x] Model testing — 86.73%, WakeWord F1 0.92
- [x] Live classification no A52 — otimo
- [x] Exportado como Android library (C++), float32 + EON

## Etapa 3 — App Flutter
- [x] Projeto Flutter criado em `app/` (br.com.fiap.wakeword_app)
- [x] Permissão de microfone (RECORD_AUDIO + permission_handler)
- [x] SDK nativo em `android/app/src/main/cpp` + ponte FFI (libwakeword.so)
- [x] Escuta contínua: buffer circular 24000, stride 500ms
- [x] Sliders de limiar e janelas para ajuste ao vivo
- [x] Detecção testada no A52 — reconhece bem
- [ ] `.apk` release assinado

## Extra — Acelerômetro
- [x] Leitura com `sensors_plus` (50 ms)
- [x] Gráfico ao vivo (CustomPainter, ±20 m/s², ~10 s)
- [x] Captura automática de 5 s ao detectar a wake word, com retorno à tela principal

## Aula
- [x] `AULA-WakeWord.md` — 4 slides de teoria + 4 de código + fechamento

## Bloqueios
- (nenhum)

## Ajuste medido no A52
- Sala fechada: limiar **0.90**, 1 janela
- Bar / multidão: limiar **0.79**, 1 janela
- Padrão fixado: **0.85**, 1 janela
- Causa da diferença: `WakeWord` foi gravado quase todo em ambiente controlado
- Inferência: **2–4 ms** (isolate separada dispensada)

## Melhoria pendente (opcional)
- [ ] Gravar ~30 amostras de "Ei Fioti" com ruído real e retreinar
- [ ] Objetivo: um limiar único servir nos dois ambientes

## Repositório
- [x] `.gitignore` — 8,5 MB versionados (era 2,2 GB)
- [x] `README.md` com o comando de extração do SDK
- SDK do Edge Impulse fica fora do Git; o `.zip` é a fonte

## Notas de build
- `compileSdk = 37` fixo: permission_handler exige, o padrao do Flutter esta em 36
- `platforms;android-37` e `cmake;3.22.1` instalados via sdkmanager

---

**Plano no `FIAP-IoT-develop`** (este repositório) ·
**Implementação no `FIAP-IoT-eval`**, em
`app40-WakeWord/app40-1-SmartphoneAndroid`
