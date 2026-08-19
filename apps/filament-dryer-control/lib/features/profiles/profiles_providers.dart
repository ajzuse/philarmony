/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:philarmony_core/philarmony_core.dart';

import '../../device/session_providers.dart';
import 'profile_sync_service.dart';

final profileSyncServiceProvider = FutureProvider<ProfileSyncService>((ref) async {
  final store = await ref.watch(localStoreProvider.future);
  final pending = await ref.watch(pendingCommandRepositoryProvider.future);
  return ProfileSyncService(
    store: store,
    pending: pending,
    getClient: () => ref.read(wsClientProvider),
    getActiveDeviceId: () => ref.read(deviceSessionProvider).activeDevice?.id,
    getConnectionState: () => ref.read(deviceSessionProvider).connectionState,
  );
});

final profilesListProvider = FutureProvider.autoDispose<List<FilamentProfile>>(
  (ref) async {
    final svc = await ref.watch(profileSyncServiceProvider.future);
    return svc.listProfiles();
  },
);
