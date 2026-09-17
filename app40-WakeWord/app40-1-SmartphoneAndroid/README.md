# Ei Fioti — Wake Word no Android

Detecção de wake word rodando no celular, sem internet.
Edge Impulse (MFCC + CNN) → C++ nativo → Flutter via FFI.

> **Implementação:** aqui (`FIAP-IoT-eval`).
> **Plano, auditoria e material de aula:** `FIAP-IoT-develop`, em
> `app-FIoT-Audio/android-wakeword/`

- Wake word: **"Ei Fioti"**
- Janela de 1,5 s a 16 kHz, avaliada a cada 500 ms
- Inferência: **2 a 4 ms** no Galaxy A52
- Ao detectar, o app captura 5 s do acelerômetro e volta

## ⚠️ Antes de compilar

O SDK do Edge Impulse (29 MB, ~1400 arquivos) **não está no Git**.
Sem ele o `flutter build` falha. Extraia primeiro.

No PowerShell, a partir desta pasta:

```powershell
Expand-Archive -Path WakeWordEIFIOT\wakeword-eifiot-cpp-android-v1-impulse-1.zip -DestinationPath app\android\app\src\main\cpp -Force
```

Deve criar três pastas dentro de `app/android/app/src/main/cpp/`:

```
edge-impulse-sdk/    model-parameters/    tflite-model/
```

Se elas já existirem, o comando sobrescreve sem erro.

## Compilar e instalar

```powershell
cd app
flutter pub get
flutter build apk --debug
adb install -r build\app\outputs\flutter-apk\app-debug.apk
```

## Requisitos

- Flutter 3.47+ · JDK 17 · Android SDK com `platforms;android-37` e `cmake;3.22.1`
- `compileSdk = 37` fixo no Gradle (exigência do `permission_handler`)
- Celular com depuração USB ligada

## Estrutura

| Caminho | O que é |
|---|---|
| `app/lib/detector.dart` | Microfone, buffer circular, decisão de disparo |
| `app/lib/wakeword_ffi.dart` | Ponte Dart → `libwakeword.so` |
| `app/lib/wakeword_page.dart` | Tela da wake word |
| `app/lib/accel_page.dart` | Acelerômetro e gráfico |
| `app/android/.../cpp/wakeword_ffi.cpp` | Wrapper C sobre o SDK |
| `app/android/.../cpp/CMakeLists.txt` | Compila o SDK para Android |
| `WakeWordEIFIOT/*.zip` | Saída do Edge Impulse (fonte da extração acima) |

## Documentos

Ficam no repositório `FIAP-IoT-develop`, em `app-FIoT-Audio/android-wakeword/`:

| Arquivo | O que é |
|---|---|
| `PLANO.md` | As etapas do projeto |
| `AUDITORIA.md` | O que está feito e o que falta |
| `ETAPA1-COLETA.md` | Roteiro de gravação do dataset |
| `AULA-WakeWord.md` | Slides da aula |

## Regerar o modelo

Ao retreinar no Edge Impulse:

- **Deployment** → **Android library (C++)** → **Unoptimized (float32)** → Build
- Substitua o `.zip` em `WakeWordEIFIOT/`
- Rode o `Expand-Archive` acima de novo
- `flutter build apk --debug`

O app lê janela, taxa e labels do próprio modelo — se esses valores mudarem,
o código Dart não precisa ser alterado.
