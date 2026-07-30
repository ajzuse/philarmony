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

class FirmwarePackage {
  FirmwarePackage({
    required this.version,
    required this.appBinaryPath,
    this.bootloaderPath,
    this.partitionTablePath,
    List<String>? supportedChips,
    required this.checksumSha256,
  }) : supportedChips = supportedChips ?? const ['ESP32', 'ESP32-S2', 'ESP32-S3', 'ESP32-C3'];

  final String version;
  final String appBinaryPath;
  final String? bootloaderPath;
  final String? partitionTablePath;
  final List<String> supportedChips;
  final String checksumSha256;
}
