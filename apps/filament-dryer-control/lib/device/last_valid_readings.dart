/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:philarmony_core/philarmony_core.dart';

/// Retains the most recent non-null, non-NaN chamber temperature and humidity.
class LastValidReadings {
  double? chamberTempC;
  double? humidityPct;

  void update(StatusSnapshot status) {
    final t = status.chamberTempC;
    if (t != null && !t.isNaN) chamberTempC = t;
    final h = status.humidityPct;
    if (h != null && !h.isNaN) humidityPct = h;
  }

  void clear() {
    chamberTempC = null;
    humidityPct = null;
  }
}
