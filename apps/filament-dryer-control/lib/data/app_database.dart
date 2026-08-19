/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:io';

import 'package:drift/drift.dart';
import 'package:drift/native.dart';
import 'package:path/path.dart' as p;
import 'package:path_provider/path_provider.dart';

part 'app_database.g.dart';

class KnownDevices extends Table {
  TextColumn get id => text()();
  TextColumn get nickname => text()();
  TextColumn get host => text()();
  IntColumn get port => integer().withDefault(const Constant(80))();
  TextColumn get path => text().withDefault(const Constant('/ws'))();
  TextColumn get deviceModel => text().nullable()();
  TextColumn get firmwareVersion => text().nullable()();
  DateTimeColumn get lastSeen => dateTime().nullable()();
  DateTimeColumn get lastConnected => dateTime().nullable()();
  BoolColumn get autoConnect => boolean().withDefault(const Constant(false))();
  TextColumn get notificationSettingsJson => text().withDefault(const Constant('{}'))();

  @override
  Set<Column> get primaryKey => {id};
}

class DryingCycles extends Table {
  TextColumn get id => text()();
  TextColumn get knownDeviceId => text()();
  TextColumn get profileId => text().nullable()();
  TextColumn get materialName => text()();
  RealColumn get targetTempC => real().nullable()();
  IntColumn get maxDurationMin => integer().nullable()();
  RealColumn get targetHumidityPct => real().nullable()();
  DateTimeColumn get startTime => dateTime()();
  DateTimeColumn get endTime => dateTime().nullable()();
  RealColumn get avgTempC => real().nullable()();
  RealColumn get maxTempC => real().nullable()();
  RealColumn get avgHumidityPct => real().nullable()();
  TextColumn get stopReason => text().nullable()();
  TextColumn get status => text()();
  TextColumn get notes => text().withDefault(const Constant(''))();
  BoolColumn get hasSamples => boolean().withDefault(const Constant(false))();

  @override
  Set<Column> get primaryKey => {id};
}

class CycleSamples extends Table {
  IntColumn get id => integer().autoIncrement()();
  TextColumn get cycleId => text()();
  IntColumn get tSec => integer()();
  RealColumn get tempC => real().nullable()();
  RealColumn get humidityPct => real().nullable()();
  RealColumn get heaterPct => real().withDefault(const Constant(0))();
  RealColumn get fanPct => real().withDefault(const Constant(0))();
}

class PendingCommands extends Table {
  TextColumn get id => text()();
  TextColumn get knownDeviceId => text()();
  TextColumn get topic => text()();
  TextColumn get payloadJson => text()();
  DateTimeColumn get createdAt => dateTime()();
  TextColumn get status => text()();
  TextColumn get lastError => text().nullable()();

  @override
  Set<Column> get primaryKey => {id};
}

@DriftDatabase(tables: [KnownDevices, DryingCycles, CycleSamples, PendingCommands])
class AppDatabase extends _$AppDatabase {
  AppDatabase([QueryExecutor? executor]) : super(executor ?? _openConnection());

  @override
  int get schemaVersion => 1;

  static LazyDatabase _openConnection() {
    return LazyDatabase(() async {
      final dir = await getApplicationDocumentsDirectory();
      final file = File(p.join(dir.path, 'philarmony_control.sqlite'));
      return NativeDatabase.createInBackground(file);
    });
  }

  /// In-memory DB for tests.
  factory AppDatabase.memory() => AppDatabase(NativeDatabase.memory());
}
