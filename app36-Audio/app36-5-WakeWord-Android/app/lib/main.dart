import 'dart:async';

import 'package:flutter/material.dart';

import 'accel_page.dart';
import 'wakeword_page.dart';

void main() => runApp(const WakeWordApp());

class WakeWordApp extends StatelessWidget {
  const WakeWordApp({super.key});

  @override
  Widget build(BuildContext context) => MaterialApp(
        title: 'Ei Fioti',
        debugShowCheckedModeBanner: false,
        theme: ThemeData(
          colorSchemeSeed: Colors.indigo,
          brightness: Brightness.dark,
          useMaterial3: true,
        ),
        home: const Shell(),
      );
}

class Shell extends StatefulWidget {
  const Shell({super.key});

  @override
  State<Shell> createState() => _ShellState();
}

class _ShellState extends State<Shell> {
  int _index = 0;
  final _captureRequests = StreamController<void>.broadcast();

  static const _titles = ['Ei Fioti', 'Acelerometro'];

  /// Wake word detectada: vai para o grafico e pede a captura de 5 s.
  void _onWakeWord() {
    setState(() => _index = 1);
    _captureRequests.add(null);
  }

  void _onCaptureDone() {
    if (mounted) setState(() => _index = 0);
  }

  @override
  void dispose() {
    _captureRequests.close();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) => Scaffold(
        appBar: AppBar(title: Text(_titles[_index])),
        // IndexedStack mantem as duas telas vivas: sair do acelerometro
        // nao derruba a escuta do microfone.
        body: IndexedStack(
          index: _index,
          children: [
            WakeWordPage(onTrigger: _onWakeWord),
            AccelPage(
              captureRequests: _captureRequests.stream,
              onCaptureDone: _onCaptureDone,
            ),
          ],
        ),
        bottomNavigationBar: NavigationBar(
          selectedIndex: _index,
          onDestinationSelected: (i) => setState(() => _index = i),
          destinations: const [
            NavigationDestination(icon: Icon(Icons.mic), label: 'Wake word'),
            NavigationDestination(
                icon: Icon(Icons.show_chart), label: 'Acelerometro'),
          ],
        ),
      );
}
