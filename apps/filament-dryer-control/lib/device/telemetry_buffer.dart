/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:philarmony_core/philarmony_core.dart';

class TelemetryBuffer {
  TelemetryBuffer({this.maxSamples = 60});

  final int maxSamples;
  final List<double> _temps = [];
  final List<double> _humidity = [];

  List<double> get temps => List.unmodifiable(_temps);
  List<double> get humidity => List.unmodifiable(_humidity);

  void add(StatusSnapshot status) {
    final t = status.chamberTempC;
    if (t != null) {
      _temps.add(t);
      if (_temps.length > maxSamples) _temps.removeAt(0);
    }
    final h = status.humidityPct;
    if (h != null) {
      _humidity.add(h);
      if (_humidity.length > maxSamples) _humidity.removeAt(0);
    }
  }

  void clear() {
    _temps.clear();
    _humidity.clear();
  }
}
