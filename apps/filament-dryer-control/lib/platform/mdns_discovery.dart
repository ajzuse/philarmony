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
        await for (final ip in client.lookup<IPAddressResourceRecord>(
          ResourceRecordQuery.addressIPv4(srv.target),
        )) {
          found[ip.address.address] = DiscoveredDevice(
            name: ptr.domainName,
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
