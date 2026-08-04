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

enum FlashJobState {
  idle,
  preflight,
  erasing,
  writingApp,
  writingNvs,
  verifying,
  resetting,
  success,
  failed,
  networkVerify,
}

enum NetworkVerifyStatus {
  pending,
  ok,
  skipped,
  warn,
}

class FlashJob {
  FlashJob({
    this.state = FlashJobState.idle,
    this.progressPct = 0,
    List<String>? logLines,
    this.error,
    this.networkVerify = NetworkVerifyStatus.pending,
    this.startedAt,
    this.finishedAt,
  }) : logLines = logLines ?? [];

  FlashJobState state;
  int progressPct;
  List<String> logLines;
  String? error;
  NetworkVerifyStatus networkVerify;
  DateTime? startedAt;
  DateTime? finishedAt;

  bool get isSuccess => state == FlashJobState.success;
  bool get isFailed => state == FlashJobState.failed;
  bool get isActive =>
      state != FlashJobState.idle &&
      state != FlashJobState.success &&
      state != FlashJobState.failed;
}
