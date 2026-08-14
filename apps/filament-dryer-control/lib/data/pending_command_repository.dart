/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:convert';

import 'package:philarmony_core/philarmony_core.dart';
import 'package:uuid/uuid.dart';

import 'app_preferences.dart';

enum PendingCommandStatus { queued, sending, acked, failed }

class PendingCommand {
  PendingCommand({
    required this.id,
    required this.knownDeviceId,
    required this.topic,
    required this.payloadJson,
    required this.createdAt,
    this.status = PendingCommandStatus.queued,
    this.lastError,
  });

  final String id;
  final String knownDeviceId;
  final String topic;
  final String payloadJson;
  final DateTime createdAt;
  final PendingCommandStatus status;
  final String? lastError;

  Map<String, dynamic> toJson() => {
        'id': id,
        'known_device_id': knownDeviceId,
        'topic': topic,
        'payload_json': payloadJson,
        'created_at': createdAt.toUtc().toIso8601String(),
        'status': status.name,
        if (lastError != null) 'last_error': lastError,
      };

  factory PendingCommand.fromJson(Map<String, dynamic> json) => PendingCommand(
        id: json['id'] as String,
        knownDeviceId: json['known_device_id'] as String,
        topic: json['topic'] as String,
        payloadJson: json['payload_json'] as String,
        createdAt: DateTime.parse(json['created_at'] as String),
        status: PendingCommandStatus.values.byName(
          json['status'] as String? ?? 'queued',
        ),
        lastError: json['last_error'] as String?,
      );

  PendingCommand copyWith({
    PendingCommandStatus? status,
    String? lastError,
  }) {
    return PendingCommand(
      id: id,
      knownDeviceId: knownDeviceId,
      topic: topic,
      payloadJson: payloadJson,
      createdAt: createdAt,
      status: status ?? this.status,
      lastError: lastError ?? this.lastError,
    );
  }
}

class PendingCommandRepository {
  PendingCommandRepository(this._store);

  final LocalStore _store;
  static const _uuid = Uuid();

  List<PendingCommand> list({String? deviceId}) {
    final all = _store
        .loadPendingCommands()
        .map(PendingCommand.fromJson)
        .toList();
    if (deviceId == null) return all;
    return all.where((c) => c.knownDeviceId == deviceId).toList();
  }

  List<PendingCommand> listQueued(String deviceId) => list(deviceId: deviceId)
      .where((c) => c.status == PendingCommandStatus.queued)
      .toList();

  Future<PendingCommand> enqueue({
    required String deviceId,
    required String topic,
    required Map<String, dynamic> payload,
  }) async {
    final cmd = PendingCommand(
      id: _uuid.v4(),
      knownDeviceId: deviceId,
      topic: topic,
      payloadJson: jsonEncode(payload),
      createdAt: DateTime.now().toUtc(),
    );
    final all = list();
    all.add(cmd);
    await _save(all);
    return cmd;
  }

  Future<void> markAcked(String id) async {
    final all = list();
    final idx = all.indexWhere((c) => c.id == id);
    if (idx < 0) return;
    all.removeAt(idx);
    await _save(all);
  }

  Future<void> markFailed(String id, String error) async {
    final all = list();
    final idx = all.indexWhere((c) => c.id == id);
    if (idx < 0) return;
    all[idx] = all[idx].copyWith(
      status: PendingCommandStatus.failed,
      lastError: error,
    );
    await _save(all);
  }

  Future<int> flush(
    String deviceId,
    Future<void> Function(PendingCommand command) send,
  ) async {
    var sent = 0;
    for (final cmd in listQueued(deviceId)) {
      try {
        await send(cmd);
        await markAcked(cmd.id);
        sent++;
      } catch (e) {
        await markFailed(cmd.id, '$e');
      }
    }
    return sent;
  }

  Future<void> _save(List<PendingCommand> commands) async {
    await _store.savePendingCommands(commands.map((c) => c.toJson()).toList());
  }
}
