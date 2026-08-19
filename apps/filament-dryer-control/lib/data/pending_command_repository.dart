/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:convert';

import 'package:drift/drift.dart';
import 'package:uuid/uuid.dart';

import 'app_database.dart' as drift;

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
  PendingCommandRepository(this._db);

  final drift.AppDatabase _db;
  static const _uuid = Uuid();

  Future<List<PendingCommand>> list({String? deviceId}) async {
    final query = _db.select(_db.pendingCommands)
      ..orderBy([(t) => OrderingTerm.asc(t.createdAt)]);
    if (deviceId != null) {
      query.where((t) => t.knownDeviceId.equals(deviceId));
    }
    final rows = await query.get();
    return rows.map(_fromRow).toList();
  }

  Future<List<PendingCommand>> listQueued(String deviceId) async {
    final rows = await (_db.select(_db.pendingCommands)
          ..where(
            (t) =>
                t.knownDeviceId.equals(deviceId) & t.status.equals('queued'),
          )
          ..orderBy([(t) => OrderingTerm.asc(t.createdAt)]))
        .get();
    return rows.map(_fromRow).toList();
  }

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
    await _db.into(_db.pendingCommands).insert(
          drift.PendingCommandsCompanion.insert(
            id: cmd.id,
            knownDeviceId: cmd.knownDeviceId,
            topic: cmd.topic,
            payloadJson: cmd.payloadJson,
            createdAt: cmd.createdAt,
            status: cmd.status.name,
          ),
        );
    return cmd;
  }

  Future<void> markAcked(String id) async {
    await (_db.delete(_db.pendingCommands)..where((t) => t.id.equals(id))).go();
  }

  Future<void> markFailed(String id, String error) async {
    await (_db.update(_db.pendingCommands)..where((t) => t.id.equals(id))).write(
      drift.PendingCommandsCompanion(
        status: const Value('failed'),
        lastError: Value(error),
      ),
    );
  }

  Future<int> flush(
    String deviceId,
    Future<void> Function(PendingCommand command) send,
  ) async {
    var sent = 0;
    for (final cmd in await listQueued(deviceId)) {
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

  PendingCommand _fromRow(drift.PendingCommand row) => PendingCommand(
        id: row.id,
        knownDeviceId: row.knownDeviceId,
        topic: row.topic,
        payloadJson: row.payloadJson,
        createdAt: row.createdAt,
        status: PendingCommandStatus.values.byName(row.status),
        lastError: row.lastError,
      );
}
