import 'dart:async';
import 'dart:typed_data';

import 'package:record/record.dart';

import 'wakeword_ffi.dart';

class Detection {
  Detection(this.scores, this.labels, this.inferenceMs);
  final List<double> scores;
  final List<String> labels;
  final int inferenceMs;

  int get topIndex {
    var best = 0;
    for (var i = 1; i < scores.length; i++) {
      if (scores[i] > scores[best]) best = i;
    }
    return best;
  }

  String get topLabel => labels[topIndex];
  double get topScore => scores[topIndex];
  double scoreOf(String label) {
    final i = labels.indexOf(label);
    return i < 0 ? 0 : scores[i];
  }
}

/// Escuta o microfone e classifica uma janela deslizante.
class WakeWordDetector {
  WakeWordDetector(this._ffi);

  final WakeWordFfi _ffi;
  final _recorder = AudioRecorder();

  // Mitiga o falso positivo: so dispara com confianca alta em N janelas seguidas.
  // Ajustaveis em tempo de execucao pelos sliders da tela.
  static const wakeLabel = 'WakeWord';
  static const cooldown = Duration(milliseconds: 1500);

  // Medido no A52: 0.90 funciona em sala fechada, 0.79 em bar cheio.
  // 0.85 e o meio-termo; o slider da tela ajusta ao vivo.
  double threshold = 0.85;
  int hitsNeeded = 1;

  static const _strideSamples = 8000; // 500 ms a 16 kHz

  final _detections = StreamController<Detection>.broadcast();
  final _triggers = StreamController<DateTime>.broadcast();
  final _errors = StreamController<String>.broadcast();

  Stream<Detection> get detections => _detections.stream;
  Stream<DateTime> get triggers => _triggers.stream;
  Stream<String> get errors => _errors.stream;

  // Diagnostico: quanto audio chegou de fato do microfone.
  int chunks = 0;
  int samplesIn = 0;
  int runs = 0;

  late final Int16List _ring = Int16List(_ffi.windowSamples);
  late final Int16List _window = Int16List(_ffi.windowSamples);
  int _writeIdx = 0;
  int _filled = 0;
  int _sinceLastRun = 0;
  int _hits = 0;
  DateTime _lastTrigger = DateTime.fromMillisecondsSinceEpoch(0);

  StreamSubscription<Uint8List>? _sub;
  bool get isRunning => _sub != null;

  Future<void> start() async {
    if (_sub != null) return;
    final stream = await _recorder.startStream(RecordConfig(
      encoder: AudioEncoder.pcm16bits,
      sampleRate: _ffi.sampleRate,
      numChannels: 1,
      echoCancel: false,
      noiseSuppress: false,
      autoGain: false,
    ));
    _sub = stream.listen(
      _onChunk,
      onError: (Object e) => _errors.add('stream: $e'),
      cancelOnError: false,
    );
  }

  Future<void> stop() async {
    await _sub?.cancel();
    _sub = null;
    await _recorder.stop();
    _writeIdx = 0;
    _filled = 0;
    _sinceLastRun = 0;
    _hits = 0;
    chunks = 0;
    samplesIn = 0;
    runs = 0;
  }

  void _onChunk(Uint8List bytes) {
    try {
      _ingest(bytes);
    } catch (e) {
      _errors.add('chunk: $e');
    }
  }

  void _ingest(Uint8List bytes) {
    chunks++;
    // O offset pode ser impar; copiar evita o erro de alinhamento do asInt16List.
    final aligned = Uint8List.fromList(bytes);
    final usable = aligned.lengthInBytes - (aligned.lengthInBytes % 2);
    final samples = aligned.buffer.asInt16List(0, usable ~/ 2);
    samplesIn += samples.length;
    final len = _ring.length;
    for (final s in samples) {
      _ring[_writeIdx] = s;
      _writeIdx = _writeIdx + 1 == len ? 0 : _writeIdx + 1;
      if (_filled < len) _filled++;
      _sinceLastRun++;

      if (_filled == len && _sinceLastRun >= _strideSamples) {
        _sinceLastRun = 0;
        _run();
      }
    }
  }

  /// Desenrola o buffer circular: a amostra mais antiga primeiro.
  void _fillWindow() {
    final len = _ring.length;
    final tail = len - _writeIdx;
    _window.setRange(0, tail, _ring, _writeIdx);
    _window.setRange(tail, len, _ring, 0);
  }

  void _run() {
    runs++;
    final started = DateTime.now();
    _fillWindow();
    final scores = _ffi.classify(_window);
    final ms = DateTime.now().difference(started).inMilliseconds;
    final det = Detection(scores, _ffi.labels, ms);
    _detections.add(det);

    if (det.scoreOf(wakeLabel) >= threshold) {
      _hits++;
    } else {
      _hits = 0;
    }

    final now = DateTime.now();
    if (_hits >= hitsNeeded && now.difference(_lastTrigger) > cooldown) {
      _lastTrigger = now;
      _hits = 0;
      _triggers.add(now);
    }
  }

  Future<void> dispose() async {
    await stop();
    await _recorder.dispose();
    await _detections.close();
    await _triggers.close();
    await _errors.close();
  }
}
