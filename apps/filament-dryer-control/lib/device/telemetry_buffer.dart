/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:philarmony_core/philarmony_core.dart';

class TelemetryBuffer {
  TelemetryBuffer({this.maxSamples = 3600});

  int maxSamples;
  final List<double> _temps = [];
  final List<double> _humidity = [];
  final List<double> _heaterPower = [];

  List<double> get temps => _windowed(_temps);
  List<double> get humidity => _windowed(_humidity);
  List<double> get heaterPower => _windowed(_heaterPower);

  void setMaxSamples(int samples) {
    maxSamples = samples;
    _trimAll();
  }

  void add(StatusSnapshot status) {
    final t = status.chamberTempC;
    if (t != null && !t.isNaN) {
      _temps.add(t);
    }
    final h = status.humidityPct;
    if (h != null && !h.isNaN) {
      _humidity.add(h);
    }
    _heaterPower.add(status.heaterPowerPct);
    _trimAll();
  }

  void clear() {
    _temps.clear();
    _humidity.clear();
    _heaterPower.clear();
  }

  List<double> _windowed(List<double> series) =>
      List.unmodifiable(series.length <= maxSamples
          ? series
          : series.sublist(series.length - maxSamples));

  void _trimAll() {
    _trim(_temps);
    _trim(_humidity);
    _trim(_heaterPower);
  }

  void _trim(List<double> series) {
    while (series.length > maxSamples) {
      series.removeAt(0);
    }
  }
}
