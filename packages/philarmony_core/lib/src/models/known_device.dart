/*
 * Philarmony Filament Dryer — Shared Core
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/// Saved connection target for the control app (not installer [DeviceProfile]).
class KnownDevice {
  KnownDevice({
    required this.id,
    required this.nickname,
    required this.host,
    this.port = 80,
    this.path = '/ws',
    this.deviceModel,
    this.firmwareVersion,
    this.lastSeen,
    this.lastConnected,
    this.autoConnect = false,
    Map<String, bool>? notificationSettings,
  }) : notificationSettings = notificationSettings ?? {};

  final String id;
  String nickname;
  String host;
  int port;
  String path;
  String? deviceModel;
  String? firmwareVersion;
  DateTime? lastSeen;
  DateTime? lastConnected;
  bool autoConnect;
  Map<String, bool> notificationSettings;

  String get wsUri => 'ws://$host:$port$path';

  KnownDevice copyWith({
    String? nickname,
    String? host,
    int? port,
    String? path,
    String? deviceModel,
    String? firmwareVersion,
    DateTime? lastSeen,
    DateTime? lastConnected,
    bool? autoConnect,
    Map<String, bool>? notificationSettings,
  }) {
    return KnownDevice(
      id: id,
      nickname: nickname ?? this.nickname,
      host: host ?? this.host,
      port: port ?? this.port,
      path: path ?? this.path,
      deviceModel: deviceModel ?? this.deviceModel,
      firmwareVersion: firmwareVersion ?? this.firmwareVersion,
      lastSeen: lastSeen ?? this.lastSeen,
      lastConnected: lastConnected ?? this.lastConnected,
      autoConnect: autoConnect ?? this.autoConnect,
      notificationSettings: notificationSettings ?? this.notificationSettings,
    );
  }

  Map<String, dynamic> toJson() => {
        'id': id,
        'nickname': nickname,
        'host': host,
        'port': port,
        'path': path,
        'device_model': deviceModel,
        'firmware_version': firmwareVersion,
        'last_seen': lastSeen?.toUtc().toIso8601String(),
        'last_connected': lastConnected?.toUtc().toIso8601String(),
        'auto_connect': autoConnect,
        'notification_settings': notificationSettings,
      };

  factory KnownDevice.fromJson(Map<String, dynamic> json) => KnownDevice(
        id: json['id'] as String,
        nickname: json['nickname'] as String,
        host: json['host'] as String,
        port: (json['port'] as num?)?.toInt() ?? 80,
        path: json['path'] as String? ?? '/ws',
        deviceModel: json['device_model'] as String?,
        firmwareVersion: json['firmware_version'] as String?,
        lastSeen: json['last_seen'] != null
            ? DateTime.parse(json['last_seen'] as String)
            : null,
        lastConnected: json['last_connected'] != null
            ? DateTime.parse(json['last_connected'] as String)
            : null,
        autoConnect: json['auto_connect'] as bool? ?? false,
        notificationSettings: (json['notification_settings'] as Map?)
                ?.map((k, v) => MapEntry(k.toString(), v as bool)) ??
            {},
      );
}
