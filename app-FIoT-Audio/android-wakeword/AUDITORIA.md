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

## Próximos passos — instruções

Ordem sugerida. Cada item é executável sem contexto adicional.

### 0. Preparar o ambiente (obrigatório antes de qualquer build)

O SDK do Edge Impulse não está no Git. No PowerShell, em
`C:\Projects\FIAP-IoT-eval\app40-WakeWord\app40-1-SmartphoneAndroid`:

```powershell
Expand-Archive -Path WakeWordEIFIOT\wakeword-eifiot-cpp-android-v1-impulse-1.zip -DestinationPath app\android\app\src\main\cpp -Force
cd app
flutter pub get
flutter build apk --debug
```

### 1. APK release assinado

Hoje só existe o debug (com faixa vermelha e sem otimização).

- Gerar a keystore, uma vez:
  ```powershell
  keytool -genkey -v -keystore $env:USERPROFILE\ei-fioti.jks -keyalg RSA -keysize 2048 -validity 10000 -alias eifioti
  ```
- Criar `app/android/key.properties` (já está no `.gitignore` do Flutter):
  ```
  storePassword=<senha>
  keyPassword=<senha>
  keyAlias=eifioti
  storeFile=C:/Users/<usuario>/ei-fioti.jks
  ```
- Em `app/android/app/build.gradle.kts`, trocar o `signingConfig` do bloco
  `release`, que hoje aponta para o do debug
- Build:
  ```powershell
  flutter build apk --release --target-platform android-arm64
  ```
- `abiFilters` já está limitado a `arm64-v8a`

**Atenção:** nunca commitar `key.properties` nem o `.jks`.

### 2. Retreinar com ruído real

Motivo: o limiar hoje precisa de 0.90 em sala e 0.79 em ambiente barulhento.
A causa é dado, não parâmetro.

- No Edge Impulse, projeto `WakeWord-EiFIOT`
- Gravar ~30 amostras de "Ei Fioti" **com ruído de fundo real** (corredor,
  bar, ventilador), label `WakeWord`, sample length 2000 ms
- Retreinar com Data augmentation ligada, 200 ciclos
- Conferir no **Model testing** se `desconhecido` → `WakeWord` caiu abaixo
  de 10%
- **Deployment** → **Android library (C++)** → **Unoptimized (float32)**
- Substituir o `.zip` em `WakeWordEIFIOT/` e repetir o passo 0
- O código Dart não muda: janela, taxa e labels são lidos do modelo

Meta: um limiar único (~0.85) servir nos dois ambientes.

### 3. Levar para o ESP32 + INMP441

O trilho irmão, em `app-FIoT-Audio/app-4-mic-FIOT`.

- Mesmo projeto do Edge Impulse, outro Deployment
- Escolher **Quantized (int8)** — no ESP32 a RAM importa
- Se não couber, baixar a janela de 1500 para 1000 ms e retreinar
- O `ww_classify` em `wakeword_ffi.cpp` serve de referência: a chamada ao
  `run_classifier` é idêntica, muda só o porting

## Armadilhas já pagas (não repetir)

- O MFCC do Edge Impulse espera escala **int16 crua**, não [-1,1].
  `int16_to_float` do SDK só faz cast — normalizar quebra o modelo em
  silêncio, sem erro
- O `record` entrega chunks com offset ímpar: copiar o buffer antes do
  `asInt16List`, senão lança exceção dentro do listener e o app fica
  "ouvindo" sem nunca classificar
- Buffer circular, nunca linear: deslocar 24.000 amostras por amostra
  são 384 M de operações por segundo
- `compileSdk = 37` fixo no Gradle — `permission_handler` exige, e o
  padrão do Flutter ainda está em 36

## Notas de build
- `compileSdk = 37` fixo: permission_handler exige, o padrao do Flutter esta em 36
- `platforms;android-37` e `cmake;3.22.1` instalados via sdkmanager

---

**Plano no `FIAP-IoT-develop`** (este repositório) ·
**Implementação no `FIAP-IoT-eval`**, em
`app40-WakeWord/app40-1-SmartphoneAndroid`
