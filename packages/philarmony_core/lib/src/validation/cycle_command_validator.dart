/*
 * Philarmony Filament Dryer — Shared Core
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import '../models/ws_models.dart';

class CycleCommandValidator {
  static List<String> validateStart(StartCycleRequest request) {
    final errors = <String>[];
    final hasProfile = request.profileId != null && request.profileId!.isNotEmpty;
    final hasExplicit = request.targetTempC != null && request.maxDurationMin != null;

    if (!hasProfile && !hasExplicit) {
      errors.add('profile_id or explicit target_temp_c + max_duration_min required');
    }

    if (request.targetTempC != null) {
      if (request.targetTempC! < 30 || request.targetTempC! > 80) {
        errors.add('target_temp_c must be 30–80°C');
      }
    }
    if (request.maxDurationMin != null) {
      if (request.maxDurationMin! < 1 || request.maxDurationMin! > 1440) {
        errors.add('max_duration_min must be 1–1440');
      }
    }
    if (request.targetHumidityPct != null) {
      if (request.targetHumidityPct! < 5 || request.targetHumidityPct! > 50) {
        errors.add('target_humidity_pct must be 5–50');
      }
    }
    return errors;
  }
}
