/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:async';

import 'package:multicast_dns/multicast_dns.dart';

import '../device/device_interfaces.dart';

class MdnsDiscovery implements DeviceDiscovery {
  @override
  Stream<List<DiscoveredDevice>> scan({
    Duration timeout = const Duration(seconds: 5),
  }) async* {
    final client = MDnsClient();
    await client.start();
    final found = <String, DiscoveredDevice>{};
    final sub = client.lookup<PtrResourceRecord>(
      ResourceRecordQuery.serverPointer('_philarmony._tcp.local'),
    ).listen((ptr) async {
      await for (final srv in client.lookup<SrvResourceRecord>(
        ResourceRecordQuery.service(ptr.domainName),
      )) {
        var displayName = _hostnameWithoutLocal(srv.target);
        await for (final txt in client.lookup<TxtResourceRecord>(
          ResourceRecordQuery.text(ptr.domainName),
        )) {
          final friendly = _friendlyNameFromTxt(txt.text);
          if (friendly != null && friendly.isNotEmpty) {
            displayName = friendly;
          }
        }
        await for (final ip in client.lookup<IPAddressResourceRecord>(
          ResourceRecordQuery.addressIPv4(srv.target),
        )) {
          found[ip.address.address] = DiscoveredDevice(
            name: displayName,
            host: ip.address.address,
            port: srv.port,
          );
        }
      }
    });
    await Future<void>.delayed(timeout);
    await sub.cancel();
    client.stop();
    yield found.values.toList();
  }
}

String _hostnameWithoutLocal(String hostname) {
  final trimmed = hostname.endsWith('.') ? hostname.substring(0, hostname.length - 1) : hostname;
  if (trimmed.toLowerCase().endsWith('.local')) {
    return trimmed.substring(0, trimmed.length - 6);
  }
  return trimmed;
}

String? _friendlyNameFromTxt(String raw) {
  final entries = <String, String>{};
  for (final line in raw.split('\n')) {
    final trimmed = line.trim();
    if (trimmed.isEmpty) continue;
    final eq = trimmed.indexOf('=');
    if (eq <= 0) {
      entries[trimmed] = '';
      continue;
    }
    final key = trimmed.substring(0, eq);
    final value = trimmed.substring(eq + 1);
    entries[key] = value;
  }
  for (final key in ['name', 'friendly_name', 'fn', 'nn']) {
    final value = entries[key];
    if (value != null && value.isNotEmpty) return value;
  }
  return null;
}
