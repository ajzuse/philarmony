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

/// Maps common flash/USB failures to suggested fixes (FR-011).
class FlashErrorHints {
  static String suggest(String error) {
    final e = error.toLowerCase();
    if (e.contains('could not open port') ||
        e.contains('permission denied') ||
        e.contains('access is denied')) {
      return 'Suggested fix: close other serial monitors, unplug/replug USB, '
          'and ensure your user can access the serial device '
          '(Linux: dialout/uucp group).';
    }
    if (e.contains('timed out waiting for packet header') ||
        e.contains('no serial data') ||
        e.contains('failed to connect')) {
      return 'Suggested fix: hold BOOT while resetting the board, try a lower '
          'baud (115200), and use a data-capable USB cable.';
    }
    if (e.contains('wrong boot mode') || e.contains('invalid head')) {
      return 'Suggested fix: put the ESP32 into download mode (BOOT+RESET), '
          'then Retry.';
    }
    if (e.contains('checksum') || e.contains('md5')) {
      return 'Suggested fix: re-run make sync-installer-firmware / '
          'make bundle-esptool and Retry with a fresh package.';
    }
    if (e.contains('firmware.bin missing') ||
        e.contains('bootloader.bin missing') ||
        e.contains('partitions.bin missing')) {
      return 'Suggested fix: run make sync-installer-firmware before flashing.';
    }
    if (e.contains('device not configured') ||
        e.contains('no such file') ||
        e.contains('enoent')) {
      return 'Suggested fix: USB unplugged mid-flash — device may be in a '
          'partial state. Reconnect, select the port, and Retry a full '
          'erase→write→verify. Do not power-cycle mid-Retry.';
    }
    if (e.contains('wifi password')) {
      return 'Suggested fix: re-enter the WiFi password on the WiFi step.';
    }
    return 'Suggested fix: Retry the full flash. Export the log and check '
        'USB drivers (CH340/CP210x/FTDI) if it keeps failing.';
  }

  static bool looksLikeUsbUnplug(String error) {
    final e = error.toLowerCase();
    return e.contains('device not configured') ||
        e.contains('no such file') ||
        e.contains('enoent') ||
        e.contains('input/output error') ||
        e.contains('broken pipe');
  }
}
