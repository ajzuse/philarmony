/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:convert';

import 'package:philarmony_core/philarmony_core.dart';

import '../../data/app_preferences.dart';
import '../../data/pending_command_repository.dart';
import '../../device/device_interfaces.dart';

/// Collapse builtins hidden when a custom profile shadows the same id.
List<FilamentProfile> visibleProfiles(List<FilamentProfile> all) {
  final shadowed = all.where((f) => !f.isBuiltin).map((f) => f.id).toSet();
  return [
    for (final f in all)
      if (!(f.isBuiltin && shadowed.contains(f.id))) f,
  ];
}

class ProfileSyncService {
  ProfileSyncService({
    required LocalStore store,
    required PendingCommandRepository pending,
    required PhilarmonyWsClient Function() getClient,
    required String? Function() getActiveDeviceId,
    required DeviceConnectionState Function() getConnectionState,
  })  : _store = store,
        _pending = pending,
        _getClient = getClient,
        _getActiveDeviceId = getActiveDeviceId,
        _getConnectionState = getConnectionState;

  final LocalStore _store;
  final PendingCommandRepository _pending;
  final PhilarmonyWsClient Function() _getClient;
  final String? Function() _getActiveDeviceId;
  final DeviceConnectionState Function() _getConnectionState;

  bool get _isOnline =>
      _getConnectionState() == DeviceConnectionState.connected;

  String? get _deviceId => _getActiveDeviceId();

  Future<List<FilamentProfile>> listProfiles() async {
    final deviceId = _deviceId;
    if (deviceId == null) {
      return visibleProfiles(FilamentProfile.builtins());
    }

    if (_isOnline) {
      try {
        final remote = await _fetchFromDevice();
        await _saveCache(deviceId, remote);
        return visibleProfiles(remote);
      } catch (_) {
        // Fall back to cache when WS topics are unavailable.
      }
    }

    final cached = _loadCache(deviceId);
    if (cached.isEmpty) {
      return visibleProfiles(FilamentProfile.builtins());
    }
    return visibleProfiles(cached);
  }

  Future<void> refreshFromDevice() async {
    final deviceId = _deviceId;
    if (deviceId == null || !_isOnline) return;
    final remote = await _fetchFromDevice();
    await _saveCache(deviceId, remote);
  }

  Future<FilamentProfile> createProfile(FilamentProfile profile) async {
    final deviceId = _deviceId;
    if (deviceId == null) throw StateError('No active device');

    final payload = _profileToPayload(profile, includeId: false);
    if (_isOnline) {
      try {
        final response = await _getClient().request(
          WsEnvelope(topic: 'config/profiles/create', payload: payload),
        );
        final id = response?.payload['profile_id'] as String? ?? profile.id;
        final created = FilamentProfile(
          id: id,
          namePt: profile.namePt,
          nameEn: profile.nameEn,
          targetTempC: profile.targetTempC,
          defaultDurationMin: profile.defaultDurationMin,
          targetHumidityPct: profile.targetHumidityPct,
          isBuiltin: false,
        );
        await _upsertLocal(deviceId, created);
        return created;
      } catch (_) {
        // Queue when device does not support profile topics.
      }
    }

    await _pending.enqueue(
      deviceId: deviceId,
      topic: 'config/profiles/create',
      payload: payload,
    );
    await _upsertLocal(deviceId, profile);
    return profile;
  }

  Future<FilamentProfile> updateProfile(FilamentProfile profile) async {
    if (profile.isBuiltin) {
      throw ArgumentError('Built-in profiles are read-only');
    }
    final deviceId = _deviceId;
    if (deviceId == null) throw StateError('No active device');

    final payload = _profileToPayload(profile, includeId: true);
    if (_isOnline) {
      try {
        await _getClient().request(
          WsEnvelope(topic: 'config/profiles/update', payload: payload),
        );
        await _upsertLocal(deviceId, profile);
        return profile;
      } catch (_) {}
    }

    await _pending.enqueue(
      deviceId: deviceId,
      topic: 'config/profiles/update',
      payload: payload,
    );
    await _upsertLocal(deviceId, profile);
    return profile;
  }

  Future<FilamentProfile> getProfile(String profileId) async {
    final deviceId = _deviceId;
    if (deviceId == null) throw StateError('No active device');

    if (_isOnline) {
      try {
        final response = await _getClient().request(
          WsEnvelope(
            topic: 'config/profiles/get',
            payload: {'profile_id': profileId},
          ),
        );
        final profileJson = response?.payload['profile'];
        if (profileJson is Map) {
          final profile =
              _profileFromJson(Map<String, dynamic>.from(profileJson));
          await _upsertLocal(deviceId, profile);
          return profile;
        }
      } catch (_) {}
    }

    final cached = _loadCache(deviceId).where((p) => p.id == profileId).firstOrNull;
    if (cached != null) return cached;
    final builtin =
        FilamentProfile.builtins().where((p) => p.id == profileId).firstOrNull;
    if (builtin != null) return builtin;
    throw StateError('Profile $profileId not found');
  }

  Future<List<FilamentProfile>> resetDefaults() async {
    final deviceId = _deviceId;
    if (deviceId == null) throw StateError('No active device');

    if (_isOnline) {
      try {
        final response = await _getClient().request(
          const WsEnvelope(topic: 'config/profiles/reset_defaults', payload: {}),
        );
        final profiles = response?.payload['profiles'];
        if (profiles is List) {
          final parsed = profiles
              .map((e) => _profileFromJson(Map<String, dynamic>.from(e as Map)))
              .toList();
          await _saveCache(deviceId, parsed);
          return visibleProfiles(parsed);
        }
      } catch (_) {}
    }

    await _pending.enqueue(
      deviceId: deviceId,
      topic: 'config/profiles/reset_defaults',
      payload: const {},
    );
    final defaults = FilamentProfile.builtins();
    await _saveCache(deviceId, defaults);
    return visibleProfiles(defaults);
  }

  Future<void> deleteProfile(String profileId) async {
    final deviceId = _deviceId;
    if (deviceId == null) throw StateError('No active device');

    if (FilamentProfile.builtins().any((p) => p.id == profileId)) {
      throw ArgumentError('Built-in profiles are read-only');
    }

    final existing =
        _loadCache(deviceId).where((p) => p.id == profileId).firstOrNull;
    if (existing?.isBuiltin == true) {
      throw ArgumentError('Built-in profiles are read-only');
    }

    final payload = {'profile_id': profileId};
    if (_isOnline) {
      try {
        await _getClient().request(
          WsEnvelope(topic: 'config/profiles/delete', payload: payload),
        );
        await _removeLocal(deviceId, profileId);
        return;
      } catch (_) {}
    }

    await _pending.enqueue(
      deviceId: deviceId,
      topic: 'config/profiles/delete',
      payload: payload,
    );
    await _removeLocal(deviceId, profileId);
  }

  Future<void> flushPending() async {
    final deviceId = _deviceId;
    if (deviceId == null || !_isOnline) return;

    await _pending.flush(deviceId, (cmd) async {
      final payload = Map<String, dynamic>.from(
        jsonDecode(cmd.payloadJson) as Map,
      );
      await _getClient().request(
        WsEnvelope(topic: cmd.topic, payload: payload),
      );
    });
    await refreshFromDevice();
  }

  Future<List<FilamentProfile>> _fetchFromDevice() async {
    final response = await _getClient().request(
      const WsEnvelope(topic: 'config/profiles/list', payload: {}),
    );
    if (response == null) {
      throw StateError('No profile list response');
    }
    final profiles = response.payload['profiles'];
    if (profiles is! List) {
      throw FormatException('Invalid profiles list response');
    }
    return profiles
        .map((e) => _profileFromJson(Map<String, dynamic>.from(e as Map)))
        .toList();
  }

  List<FilamentProfile> _loadCache(String deviceId) {
    return _store
        .loadProfilesForDevice(deviceId)
        .map(_profileFromJson)
        .toList();
  }

  Future<void> _saveCache(String deviceId, List<FilamentProfile> profiles) {
    return _store.saveProfilesForDevice(
      deviceId,
      profiles.map(_profileToJson).toList(),
    );
  }

  Future<void> _upsertLocal(String deviceId, FilamentProfile profile) async {
    final all = _loadCache(deviceId);
    final builtins = FilamentProfile.builtins();
    final customs = [
      ...all.where((p) => !p.isBuiltin && p.id != profile.id),
      if (!profile.isBuiltin) profile,
    ];
    final base = builtins
        .where((b) => !customs.any((c) => c.id == b.id))
        .toList();
    await _saveCache(deviceId, [...base, ...customs]);
  }

  Future<void> _removeLocal(String deviceId, String profileId) async {
    final all = _loadCache(deviceId);
    final remaining = all.where((p) => p.id != profileId).toList();
    if (remaining.isEmpty) {
      await _saveCache(deviceId, FilamentProfile.builtins());
      return;
    }
    await _saveCache(deviceId, remaining);
  }

  static Map<String, dynamic> _profileToPayload(
    FilamentProfile profile, {
    required bool includeId,
  }) {
    final map = <String, dynamic>{
      'name_pt': profile.namePt,
      'name_en': profile.nameEn,
      'target_temp_c': profile.targetTempC,
      'default_duration_min': profile.defaultDurationMin,
      'target_humidity_pct': profile.targetHumidityPct,
    };
    if (includeId) map['profile_id'] = profile.id;
    return map;
  }

  static Map<String, dynamic> _profileToJson(FilamentProfile profile) => {
        'id': profile.id,
        'name_pt': profile.namePt,
        'name_en': profile.nameEn,
        'target_temp_c': profile.targetTempC,
        'default_duration_min': profile.defaultDurationMin,
        'target_humidity_pct': profile.targetHumidityPct,
        'is_builtin': profile.isBuiltin,
      };

  static FilamentProfile _profileFromJson(Map<String, dynamic> json) =>
      FilamentProfile(
        id: json['id'] as String,
        namePt: json['name_pt'] as String? ?? '',
        nameEn: json['name_en'] as String? ?? '',
        targetTempC: (json['target_temp_c'] as num?)?.toDouble() ?? 50,
        defaultDurationMin: json['default_duration_min'] as int? ?? 240,
        targetHumidityPct:
            (json['target_humidity_pct'] as num?)?.toDouble() ?? 15,
        isBuiltin: json['is_builtin'] as bool? ?? false,
      );
}
