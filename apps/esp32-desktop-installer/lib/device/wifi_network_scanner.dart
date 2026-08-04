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

import 'dart:convert';
import 'dart:io';

/// Optional nearby WiFi SSID scan via host OS tools (nmcli / airport / netsh).
/// Returns empty list when unsupported or permission denied — never throws.
class WifiNetworkScanner {
  Future<List<String>> scanNearbySsids({
    Duration timeout = const Duration(seconds: 12),
  }) async {
    try {
      if (Platform.isLinux) {
        return _unique(await _scanNmcli(timeout));
      }
      if (Platform.isMacOS) {
        return _unique(await _scanAirport(timeout));
      }
      if (Platform.isWindows) {
        return _unique(await _scanNetsh(timeout));
      }
    } catch (_) {}
    return const [];
  }

  List<String> _unique(List<String> ssids) {
    final seen = <String>{};
    final out = <String>[];
    for (final s in ssids) {
      final t = s.trim();
      if (t.isEmpty || t == '--' || t.toLowerCase() == 'ssid') continue;
      if (seen.add(t)) out.add(t);
    }
    return out;
  }

  Future<List<String>> _scanNmcli(Duration timeout) async {
    final r = await Process.run(
      'nmcli',
      ['-t', '-f', 'SSID', 'dev', 'wifi', 'list', '--rescan', 'yes'],
      runInShell: true,
    ).timeout(timeout);
    if (r.exitCode != 0) return const [];
    return const LineSplitter().convert('${r.stdout}');
  }

  Future<List<String>> _scanAirport(Duration timeout) async {
    const airport =
        '/System/Library/PrivateFrameworks/Apple80211.framework/Versions/Current/Resources/airport';
    final r = await Process.run(
      airport,
      ['-s'],
      runInShell: false,
    ).timeout(timeout);
    if (r.exitCode != 0) return const [];
    final lines = const LineSplitter().convert('${r.stdout}').skip(1);
    return lines
        .map((l) => l.trimLeft().split(RegExp(r'\s{2,}')).first)
        .toList();
  }

  Future<List<String>> _scanNetsh(Duration timeout) async {
    final r = await Process.run(
      'netsh',
      ['wlan', 'show', 'networks', 'mode=bssid'],
      runInShell: true,
    ).timeout(timeout);
    if (r.exitCode != 0) return const [];
    final ssids = <String>[];
    for (final line in const LineSplitter().convert('${r.stdout}')) {
      final m = RegExp(r'^\s*SSID\s+\d+\s*:\s*(.+)\s*$', caseSensitive: false)
          .firstMatch(line);
      if (m != null) ssids.add(m.group(1)!.trim());
    }
    return ssids;
  }
}
