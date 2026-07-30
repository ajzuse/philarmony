/*
 * Philarmony Filament Dryer — Desktop Installer / Shared Core
 * Copyright (C) 2026 Philarmony Contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

import '../models/filament_profile.dart';

class FilamentProfileValidator {
  static const maxCustom = 20;

  static List<String> validate(FilamentProfile p, {required int customCount}) {
    final errors = <String>[];
    if (!p.isBuiltin && customCount > maxCustom) {
      errors.add('Max $maxCustom custom profiles');
    }
    if (p.targetTempC < 30 || p.targetTempC > 80) {
      errors.add('Temperature must be 30–80°C');
    }
    if (p.defaultDurationMin < 1 || p.defaultDurationMin > 1440) {
      errors.add('Duration must be 1–1440 minutes');
    }
    if (p.targetHumidityPct < 5 || p.targetHumidityPct > 50) {
      errors.add('Humidity must be 5–50%');
    }
    if (p.namePt.trim().isEmpty || p.nameEn.trim().isEmpty) {
      errors.add('Names PT/EN required');
    }
    return errors;
  }
}
