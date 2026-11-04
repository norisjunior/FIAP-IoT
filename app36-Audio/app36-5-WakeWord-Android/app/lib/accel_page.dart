import 'dart:async';
import 'dart:collection';

import 'package:flutter/material.dart';
import 'package:sensors_plus/sensors_plus.dart';

const _axisColors = [Colors.redAccent, Colors.greenAccent, Colors.blueAccent];
const _axisNames = ['x', 'y', 'z'];

class AccelPage extends StatefulWidget {
  const AccelPage({
    super.key,
    required this.captureRequests,
    required this.onCaptureDone,
  });

  /// Cada evento dispara uma captura automatica de 5 s.
  final Stream<void> captureRequests;
  final VoidCallback onCaptureDone;

  @override
  State<AccelPage> createState() => _AccelPageState();
}

class _AccelPageState extends State<AccelPage> {
  static const _capacity = 200; // ~10 s a 50 ms
  static const _period = Duration(milliseconds: 50);

  final _history = [
    for (var i = 0; i < 3; i++) ListQueue<double>(_capacity),
  ];

  static const _autoSeconds = 5;

  StreamSubscription<AccelerometerEvent>? _sub;
  StreamSubscription<void>? _reqSub;
  Timer? _autoTimer;
  var _latest = const [0.0, 0.0, 0.0];
  var _running = false;
  var _remaining = 0;

  @override
  void initState() {
    super.initState();
    _reqSub = widget.captureRequests.listen((_) => _startAuto());
  }

  void _start() {
    if (_running) return;
    _sub = accelerometerEventStream(samplingPeriod: _period).listen((e) {
      final values = [e.x, e.y, e.z];
      for (var i = 0; i < 3; i++) {
        final q = _history[i];
        if (q.length == _capacity) q.removeFirst();
        q.addLast(values[i]);
      }
      setState(() => _latest = values);
    });
    setState(() => _running = true);
  }

  void _stop() {
    _sub?.cancel();
    _sub = null;
    setState(() => _running = false);
  }

  void _toggle() {
    // Um toque manual cancela a captura automatica em andamento.
    _autoTimer?.cancel();
    _autoTimer = null;
    if (_remaining != 0) setState(() => _remaining = 0);
    _running ? _stop() : _start();
  }

  /// Chamado quando o "Ei Fioti" e detectado.
  void _startAuto() {
    _autoTimer?.cancel();
    _clear();
    _start();
    setState(() => _remaining = _autoSeconds);
    _autoTimer = Timer.periodic(const Duration(seconds: 1), (t) {
      setState(() => _remaining--);
      if (_remaining > 0) return;
      t.cancel();
      _autoTimer = null;
      _stop();
      widget.onCaptureDone();
    });
  }

  void _clear() {
    for (final q in _history) {
      q.clear();
    }
    setState(() {});
  }

  @override
  void dispose() {
    _autoTimer?.cancel();
    _reqSub?.cancel();
    _sub?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) => Padding(
        padding: const EdgeInsets.all(20),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: [
                for (var i = 0; i < 3; i++)
                  Column(
                    children: [
                      Text(
                        _axisNames[i],
                        style: TextStyle(
                          color: _axisColors[i],
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                      Text(
                        _latest[i].toStringAsFixed(2),
                        style: Theme.of(context).textTheme.titleLarge,
                      ),
                    ],
                  ),
              ],
            ),
            const SizedBox(height: 8),
            if (_remaining > 0)
              Text(
                'capturando... $_remaining s',
                textAlign: TextAlign.center,
                style: Theme.of(context)
                    .textTheme
                    .titleMedium
                    ?.copyWith(color: Colors.greenAccent),
              ),
            Text(
              'm/s² — inclui a gravidade (~9.8 no eixo vertical)',
              textAlign: TextAlign.center,
              style: Theme.of(context).textTheme.bodySmall,
            ),
            const SizedBox(height: 16),
            Expanded(
              child: Card(
                clipBehavior: Clip.antiAlias,
                child: CustomPaint(
                  painter: _AccelPainter(_history),
                  size: Size.infinite,
                ),
              ),
            ),
            const SizedBox(height: 16),
            Row(
              children: [
                Expanded(
                  child: FilledButton.icon(
                    onPressed: _toggle,
                    icon: Icon(_running ? Icons.stop : Icons.play_arrow),
                    label: Text(_running ? 'Parar' : 'Ler'),
                    style: FilledButton.styleFrom(
                      padding: const EdgeInsets.symmetric(vertical: 18),
                    ),
                  ),
                ),
                const SizedBox(width: 12),
                OutlinedButton(
                  onPressed: _clear,
                  style: OutlinedButton.styleFrom(
                    padding: const EdgeInsets.symmetric(
                        vertical: 18, horizontal: 20),
                  ),
                  child: const Text('Limpar'),
                ),
              ],
            ),
          ],
        ),
      );
}

class _AccelPainter extends CustomPainter {
  _AccelPainter(this.history);

  final List<ListQueue<double>> history;

  // Escala fixa: o eixo nao pula de tamanho a cada quadro.
  static const _range = 20.0;

  @override
  void paint(Canvas canvas, Size size) {
    final grid = Paint()
      ..color = Colors.white24
      ..strokeWidth = 1;

    // Linhas em -20, -10, 0, 10, 20
    for (var v = -_range; v <= _range; v += 10) {
      final y = _toY(v, size);
      canvas.drawLine(Offset(0, y), Offset(size.width, y), grid);
    }

    for (var axis = 0; axis < 3; axis++) {
      final data = history[axis];
      if (data.length < 2) continue;

      final path = Path();
      final step = size.width / (_capacityOf(data) - 1);
      var i = 0;
      for (final v in data) {
        final x = i * step;
        final y = _toY(v, size);
        i == 0 ? path.moveTo(x, y) : path.lineTo(x, y);
        i++;
      }

      canvas.drawPath(
        path,
        Paint()
          ..color = _axisColors[axis]
          ..strokeWidth = 2
          ..style = PaintingStyle.stroke,
      );
    }
  }

  int _capacityOf(ListQueue<double> q) => q.length < 2 ? 2 : q.length;

  double _toY(double value, Size size) {
    final clamped = value.clamp(-_range, _range);
    return size.height * (1 - (clamped + _range) / (2 * _range));
  }

  @override
  bool shouldRepaint(_AccelPainter old) => true;
}
