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

class FilamentProfile {
  FilamentProfile({
    required this.id,
    required this.namePt,
    required this.nameEn,
    required this.targetTempC,
    required this.defaultDurationMin,
    required this.targetHumidityPct,
    this.isBuiltin = false,
  });

  final String id;
  String namePt;
  String nameEn;
  double targetTempC;
  int defaultDurationMin;
  double targetHumidityPct;
  final bool isBuiltin;

  static List<FilamentProfile> builtins() => [
        FilamentProfile(
          id: 'pla',
          namePt: 'PLA',
          nameEn: 'PLA',
          targetTempC: 50,
          defaultDurationMin: 240,
          targetHumidityPct: 15,
          isBuiltin: true,
        ),
        FilamentProfile(
          id: 'petg',
          namePt: 'PETG',
          nameEn: 'PETG',
          targetTempC: 65,
          defaultDurationMin: 240,
          targetHumidityPct: 15,
          isBuiltin: true,
        ),
        FilamentProfile(
          id: 'abs',
          namePt: 'ABS',
          nameEn: 'ABS',
          targetTempC: 80,
          defaultDurationMin: 120,
          targetHumidityPct: 10,
          isBuiltin: true,
        ),
        FilamentProfile(
          id: 'tpu',
          namePt: 'TPU',
          nameEn: 'TPU',
          targetTempC: 45,
          defaultDurationMin: 240,
          targetHumidityPct: 20,
          isBuiltin: true,
        ),
        FilamentProfile(
          id: 'nylon',
          namePt: 'Nylon',
          nameEn: 'Nylon',
          targetTempC: 70,
          defaultDurationMin: 360,
          targetHumidityPct: 10,
          isBuiltin: true,
        ),
      ];
}
