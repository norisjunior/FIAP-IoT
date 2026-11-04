import 'dart:async';

import 'package:flutter/material.dart';
import 'package:permission_handler/permission_handler.dart';

import 'detector.dart';
import 'wakeword_ffi.dart';

class WakeWordPage extends StatefulWidget {
  const WakeWordPage({super.key, this.onTrigger});

  /// Avisa o shell que a wake word foi detectada.
  final VoidCallback? onTrigger;

  @override
  State<WakeWordPage> createState() => _WakeWordPageState();
}

class _WakeWordPageState extends State<WakeWordPage> {
  WakeWordFfi? _ffi;
  WakeWordDetector? _detector;
  Detection? _last;
  DateTime? _triggeredAt;
  String? _error;
  bool _listening = false;
  Timer? _flashTimer;
  Timer? _statsTimer;

  @override
  void initState() {
    super.initState();
    try {
      final ffi = WakeWordFfi.open();
      _ffi = ffi;
      final detector = WakeWordDetector(ffi);
      detector.detections.listen((d) => setState(() => _last = d));
      detector.triggers.listen(_onTrigger);
      detector.errors.listen((e) => setState(() => _error = e));
      _detector = detector;
    } catch (e) {
      _error = '$e';
    }
  }

  void _onTrigger(DateTime at) {
    widget.onTrigger?.call();
    setState(() => _triggeredAt = at);
    _flashTimer?.cancel();
    _flashTimer = Timer(const Duration(milliseconds: 2500), () {
      if (mounted) setState(() => _triggeredAt = null);
    });
  }

  Future<void> _toggle() async {
    final detector = _detector;
    if (detector == null) return;

    if (detector.isRunning) {
      await detector.stop();
      _statsTimer?.cancel();
      _statsTimer = null;
      setState(() {
        _listening = false;
        _last = null;
      });
      return;
    }

    if (!await Permission.microphone.request().isGranted) {
      setState(() => _error = 'Permissao de microfone negada');
      return;
    }
    await detector.start();
    _statsTimer?.cancel();
    _statsTimer = Timer.periodic(
      const Duration(milliseconds: 500),
      (_) => setState(() {}),
    );
    setState(() {
      _listening = true;
      _error = null;
    });
  }

  @override
  void dispose() {
    _flashTimer?.cancel();
    _statsTimer?.cancel();
    _detector?.dispose();
    _ffi?.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final fired = _triggeredAt != null;
    return Scaffold(
      backgroundColor: fired ? Colors.green.shade700 : null,
      body: Padding(
        padding: const EdgeInsets.all(20),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            if (_error != null)
              Card(
                color: Colors.red.shade900,
                child: Padding(
                  padding: const EdgeInsets.all(12),
                  child: Text(_error!),
                ),
              ),
            Expanded(
              child: Center(
                child: Text(
                  fired ? 'Ei Fioti!' : (_listening ? 'Ouvindo...' : 'Parado'),
                  style: Theme.of(context).textTheme.headlineMedium,
                ),
              ),
            ),
            if (_last != null) ...[
              for (var i = 0; i < _last!.labels.length; i++)
                _ScoreBar(
                  label: _last!.labels[i],
                  score: _last!.scores[i],
                  highlight: _last!.labels[i] == WakeWordDetector.wakeLabel,
                ),
              const SizedBox(height: 12),
              Text(
                'inferencia: ${_last!.inferenceMs} ms',
                textAlign: TextAlign.center,
                style: Theme.of(context).textTheme.bodySmall,
              ),
            ],
            if (_detector != null) ...[
              _Tuner(
                label: 'limiar',
                value: _detector!.threshold,
                min: 0.50,
                max: 0.99,
                divisions: 49,
                display: _detector!.threshold.toStringAsFixed(2),
                onChanged: (v) => setState(() => _detector!.threshold = v),
              ),
              _Tuner(
                label: 'janelas',
                value: _detector!.hitsNeeded.toDouble(),
                min: 1,
                max: 3,
                divisions: 2,
                display: '${_detector!.hitsNeeded}',
                onChanged: (v) =>
                    setState(() => _detector!.hitsNeeded = v.round()),
              ),
            ],
            if (_listening && _detector != null)
              Text(
                'chunks ${_detector!.chunks} | amostras ${_detector!.samplesIn}'
                ' | inferencias ${_detector!.runs}',
                textAlign: TextAlign.center,
                style: Theme.of(context).textTheme.bodySmall,
              ),
            const SizedBox(height: 20),
            FilledButton.icon(
              onPressed: _detector == null ? null : _toggle,
              icon: Icon(_listening ? Icons.stop : Icons.mic),
              label: Text(_listening ? 'Parar' : 'Ouvir'),
              style: FilledButton.styleFrom(
                padding: const EdgeInsets.symmetric(vertical: 18),
              ),
            ),
          ],
        ),
      ),
    );
  }
}

class _Tuner extends StatelessWidget {
  const _Tuner({
    required this.label,
    required this.value,
    required this.min,
    required this.max,
    required this.divisions,
    required this.display,
    required this.onChanged,
  });

  final String label;
  final double value;
  final double min;
  final double max;
  final int divisions;
  final String display;
  final ValueChanged<double> onChanged;

  @override
  Widget build(BuildContext context) => Row(
        children: [
          SizedBox(width: 70, child: Text(label)),
          Expanded(
            child: Slider(
              value: value,
              min: min,
              max: max,
              divisions: divisions,
              onChanged: onChanged,
            ),
          ),
          SizedBox(
            width: 44,
            child: Text(display, textAlign: TextAlign.right),
          ),
        ],
      );
}

class _ScoreBar extends StatelessWidget {
  const _ScoreBar({
    required this.label,
    required this.score,
    required this.highlight,
  });

  final String label;
  final double score;
  final bool highlight;

  @override
  Widget build(BuildContext context) => Padding(
        padding: const EdgeInsets.symmetric(vertical: 4),
        child: Row(
          children: [
            SizedBox(
              width: 120,
              child: Text(
                label,
                style: TextStyle(
                  fontWeight: highlight ? FontWeight.bold : FontWeight.normal,
                ),
              ),
            ),
            Expanded(
              child: LinearProgressIndicator(
                value: score.clamp(0.0, 1.0),
                minHeight: 10,
              ),
            ),
            SizedBox(
              width: 56,
              child: Text(
                score.toStringAsFixed(2),
                textAlign: TextAlign.right,
              ),
            ),
          ],
        ),
      );
}
