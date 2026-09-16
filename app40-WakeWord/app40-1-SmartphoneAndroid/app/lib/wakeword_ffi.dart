import 'dart:ffi' as ffi;
import 'dart:typed_data';
import 'package:ffi/ffi.dart';

typedef _ClassifyC = ffi.Int32 Function(
    ffi.Pointer<ffi.Int16>, ffi.Int32, ffi.Pointer<ffi.Float>, ffi.Int32);
typedef _ClassifyDart = int Function(
    ffi.Pointer<ffi.Int16>, int, ffi.Pointer<ffi.Float>, int);

typedef _IntC = ffi.Int32 Function();
typedef _IntDart = int Function();

typedef _LabelC = ffi.Pointer<Utf8> Function(ffi.Int32);
typedef _LabelDart = ffi.Pointer<Utf8> Function(int);

/// Ponte para o classificador do Edge Impulse compilado em libwakeword.so.
class WakeWordFfi {
  WakeWordFfi._(ffi.DynamicLibrary lib, this.windowSamples, this.sampleRate,
      this.labels)
      : _classify = lib.lookupFunction<_ClassifyC, _ClassifyDart>('ww_classify'),
        _inBuf = calloc<ffi.Int16>(windowSamples),
        _outBuf = calloc<ffi.Float>(labels.length);

  final int windowSamples;
  final int sampleRate;
  final List<String> labels;

  final _ClassifyDart _classify;
  final ffi.Pointer<ffi.Int16> _inBuf;
  final ffi.Pointer<ffi.Float> _outBuf;

  static WakeWordFfi open() {
    final lib = ffi.DynamicLibrary.open('libwakeword.so');
    final windowSamples =
        lib.lookupFunction<_IntC, _IntDart>('ww_window_samples')();
    final sampleRate = lib.lookupFunction<_IntC, _IntDart>('ww_sample_rate')();
    final count = lib.lookupFunction<_IntC, _IntDart>('ww_label_count')();
    final labelFn = lib.lookupFunction<_LabelC, _LabelDart>('ww_label');
    final labels = [
      for (var i = 0; i < count; i++) labelFn(i).toDartString(),
    ];
    return WakeWordFfi._(lib, windowSamples, sampleRate, labels);
  }

  /// Classifica uma janela completa. Retorna uma probabilidade por label.
  List<double> classify(Int16List window) {
    if (window.length != windowSamples) {
      throw ArgumentError('esperado $windowSamples amostras, veio ${window.length}');
    }
    _inBuf.asTypedList(windowSamples).setAll(0, window);
    final rc = _classify(_inBuf, windowSamples, _outBuf, labels.length);
    if (rc != 0) throw StateError('ww_classify falhou: $rc');
    return List<double>.from(_outBuf.asTypedList(labels.length));
  }

  void dispose() {
    calloc.free(_inBuf);
    calloc.free(_outBuf);
  }
}
