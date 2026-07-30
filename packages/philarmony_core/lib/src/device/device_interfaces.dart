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

import '../models/device_profile.dart';
import '../models/flash_job.dart';
import '../models/firmware_package.dart';

class SerialPortInfo {
  SerialPortInfo({
    required this.path,
    this.description = '',
    this.vid,
    this.pid,
  });

  final String path;
  final String description;
  final int? vid;
  final int? pid;
}

abstract class DeviceDetector {
  Future<List<SerialPortInfo>> listPorts();
}

class FlashProgressEvent {
  FlashProgressEvent({
    required this.stage,
    required this.progressPct,
    this.message = '',
  });

  final FlashJobState stage;
  final int progressPct;
  final String message;
}

abstract class FirmwareFlasher {
  Stream<FlashProgressEvent> start({
    required String portPath,
    required DeviceProfile profile,
    required FirmwarePackage package,
  });
}

/// Soft post-flash network check — never overrides esptool success (R8).
abstract class PostFlashVerifier {
  Future<NetworkVerifyStatus> tryReach({
    String? host,
    Duration timeout = const Duration(seconds: 5),
  });
}
