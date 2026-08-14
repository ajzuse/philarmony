// GENERATED CODE - DO NOT MODIFY BY HAND

part of 'app_database.dart';

// ignore_for_file: type=lint
class $KnownDevicesTable extends KnownDevices
    with TableInfo<$KnownDevicesTable, KnownDevice> {
  @override
  final GeneratedDatabase attachedDatabase;
  final String? _alias;
  $KnownDevicesTable(this.attachedDatabase, [this._alias]);
  static const VerificationMeta _idMeta = const VerificationMeta('id');
  @override
  late final GeneratedColumn<String> id = GeneratedColumn<String>(
    'id',
    aliasedName,
    false,
    type: DriftSqlType.string,
    requiredDuringInsert: true,
  );
  static const VerificationMeta _nicknameMeta = const VerificationMeta(
    'nickname',
  );
  @override
  late final GeneratedColumn<String> nickname = GeneratedColumn<String>(
    'nickname',
    aliasedName,
    false,
    type: DriftSqlType.string,
    requiredDuringInsert: true,
  );
  static const VerificationMeta _hostMeta = const VerificationMeta('host');
  @override
  late final GeneratedColumn<String> host = GeneratedColumn<String>(
    'host',
    aliasedName,
    false,
    type: DriftSqlType.string,
    requiredDuringInsert: true,
  );
  static const VerificationMeta _portMeta = const VerificationMeta('port');
  @override
  late final GeneratedColumn<int> port = GeneratedColumn<int>(
    'port',
    aliasedName,
    false,
    type: DriftSqlType.int,
    requiredDuringInsert: false,
    defaultValue: const Constant(80),
  );
  static const VerificationMeta _pathMeta = const VerificationMeta('path');
  @override
  late final GeneratedColumn<String> path = GeneratedColumn<String>(
    'path',
    aliasedName,
    false,
    type: DriftSqlType.string,
    requiredDuringInsert: false,
    defaultValue: const Constant('/ws'),
  );
  static const VerificationMeta _deviceModelMeta = const VerificationMeta(
    'deviceModel',
  );
  @override
  late final GeneratedColumn<String> deviceModel = GeneratedColumn<String>(
    'device_model',
    aliasedName,
    true,
    type: DriftSqlType.string,
    requiredDuringInsert: false,
  );
  static const VerificationMeta _firmwareVersionMeta = const VerificationMeta(
    'firmwareVersion',
  );
  @override
  late final GeneratedColumn<String> firmwareVersion = GeneratedColumn<String>(
    'firmware_version',
    aliasedName,
    true,
    type: DriftSqlType.string,
    requiredDuringInsert: false,
  );
  static const VerificationMeta _lastSeenMeta = const VerificationMeta(
    'lastSeen',
  );
  @override
  late final GeneratedColumn<DateTime> lastSeen = GeneratedColumn<DateTime>(
    'last_seen',
    aliasedName,
    true,
    type: DriftSqlType.dateTime,
    requiredDuringInsert: false,
  );
  static const VerificationMeta _lastConnectedMeta = const VerificationMeta(
    'lastConnected',
  );
  @override
  late final GeneratedColumn<DateTime> lastConnected =
      GeneratedColumn<DateTime>(
        'last_connected',
        aliasedName,
        true,
        type: DriftSqlType.dateTime,
        requiredDuringInsert: false,
      );
  static const VerificationMeta _autoConnectMeta = const VerificationMeta(
    'autoConnect',
  );
  @override
  late final GeneratedColumn<bool> autoConnect = GeneratedColumn<bool>(
    'auto_connect',
    aliasedName,
    false,
    type: DriftSqlType.bool,
    requiredDuringInsert: false,
    defaultConstraints: GeneratedColumn.constraintIsAlways(
      'CHECK ("auto_connect" IN (0, 1))',
    ),
    defaultValue: const Constant(false),
  );
  static const VerificationMeta _notificationSettingsJsonMeta =
      const VerificationMeta('notificationSettingsJson');
  @override
  late final GeneratedColumn<String> notificationSettingsJson =
      GeneratedColumn<String>(
        'notification_settings_json',
        aliasedName,
        false,
        type: DriftSqlType.string,
        requiredDuringInsert: false,
        defaultValue: const Constant('{}'),
      );
  @override
  List<GeneratedColumn> get $columns => [
    id,
    nickname,
    host,
    port,
    path,
    deviceModel,
    firmwareVersion,
    lastSeen,
    lastConnected,
    autoConnect,
    notificationSettingsJson,
  ];
  @override
  String get aliasedName => _alias ?? actualTableName;
  @override
  String get actualTableName => $name;
  static const String $name = 'known_devices';
  @override
  VerificationContext validateIntegrity(
    Insertable<KnownDevice> instance, {
    bool isInserting = false,
  }) {
    final context = VerificationContext();
    final data = instance.toColumns(true);
    if (data.containsKey('id')) {
      context.handle(_idMeta, id.isAcceptableOrUnknown(data['id']!, _idMeta));
    } else if (isInserting) {
      context.missing(_idMeta);
    }
    if (data.containsKey('nickname')) {
      context.handle(
        _nicknameMeta,
        nickname.isAcceptableOrUnknown(data['nickname']!, _nicknameMeta),
      );
    } else if (isInserting) {
      context.missing(_nicknameMeta);
    }
    if (data.containsKey('host')) {
      context.handle(
        _hostMeta,
        host.isAcceptableOrUnknown(data['host']!, _hostMeta),
      );
    } else if (isInserting) {
      context.missing(_hostMeta);
    }
    if (data.containsKey('port')) {
      context.handle(
        _portMeta,
        port.isAcceptableOrUnknown(data['port']!, _portMeta),
      );
    }
    if (data.containsKey('path')) {
      context.handle(
        _pathMeta,
        path.isAcceptableOrUnknown(data['path']!, _pathMeta),
      );
    }
    if (data.containsKey('device_model')) {
      context.handle(
        _deviceModelMeta,
        deviceModel.isAcceptableOrUnknown(
          data['device_model']!,
          _deviceModelMeta,
        ),
      );
    }
    if (data.containsKey('firmware_version')) {
      context.handle(
        _firmwareVersionMeta,
        firmwareVersion.isAcceptableOrUnknown(
          data['firmware_version']!,
          _firmwareVersionMeta,
        ),
      );
    }
    if (data.containsKey('last_seen')) {
      context.handle(
        _lastSeenMeta,
        lastSeen.isAcceptableOrUnknown(data['last_seen']!, _lastSeenMeta),
      );
    }
    if (data.containsKey('last_connected')) {
      context.handle(
        _lastConnectedMeta,
        lastConnected.isAcceptableOrUnknown(
          data['last_connected']!,
          _lastConnectedMeta,
        ),
      );
    }
    if (data.containsKey('auto_connect')) {
      context.handle(
        _autoConnectMeta,
        autoConnect.isAcceptableOrUnknown(
          data['auto_connect']!,
          _autoConnectMeta,
        ),
      );
    }
    if (data.containsKey('notification_settings_json')) {
      context.handle(
        _notificationSettingsJsonMeta,
        notificationSettingsJson.isAcceptableOrUnknown(
          data['notification_settings_json']!,
          _notificationSettingsJsonMeta,
        ),
      );
    }
    return context;
  }

  @override
  Set<GeneratedColumn> get $primaryKey => {id};
  @override
  KnownDevice map(Map<String, dynamic> data, {String? tablePrefix}) {
    final effectivePrefix = tablePrefix != null ? '$tablePrefix.' : '';
    return KnownDevice(
      id: attachedDatabase.typeMapping.read(
        DriftSqlType.string,
        data['${effectivePrefix}id'],
      )!,
      nickname: attachedDatabase.typeMapping.read(
        DriftSqlType.string,
        data['${effectivePrefix}nickname'],
      )!,
      host: attachedDatabase.typeMapping.read(
        DriftSqlType.string,
        data['${effectivePrefix}host'],
      )!,
      port: attachedDatabase.typeMapping.read(
        DriftSqlType.int,
        data['${effectivePrefix}port'],
      )!,
      path: attachedDatabase.typeMapping.read(
        DriftSqlType.string,
        data['${effectivePrefix}path'],
      )!,
      deviceModel: attachedDatabase.typeMapping.read(
        DriftSqlType.string,
        data['${effectivePrefix}device_model'],
      ),
      firmwareVersion: attachedDatabase.typeMapping.read(
        DriftSqlType.string,
        data['${effectivePrefix}firmware_version'],
      ),
      lastSeen: attachedDatabase.typeMapping.read(
        DriftSqlType.dateTime,
        data['${effectivePrefix}last_seen'],
      ),
      lastConnected: attachedDatabase.typeMapping.read(
        DriftSqlType.dateTime,
        data['${effectivePrefix}last_connected'],
      ),
      autoConnect: attachedDatabase.typeMapping.read(
        DriftSqlType.bool,
        data['${effectivePrefix}auto_connect'],
      )!,
      notificationSettingsJson: attachedDatabase.typeMapping.read(
        DriftSqlType.string,
        data['${effectivePrefix}notification_settings_json'],
      )!,
    );
  }

  @override
  $KnownDevicesTable createAlias(String alias) {
    return $KnownDevicesTable(attachedDatabase, alias);
  }
}

class KnownDevice extends DataClass implements Insertable<KnownDevice> {
  final String id;
  final String nickname;
  final String host;
  final int port;
  final String path;
  final String? deviceModel;
  final String? firmwareVersion;
  final DateTime? lastSeen;
  final DateTime? lastConnected;
  final bool autoConnect;
  final String notificationSettingsJson;
  const KnownDevice({
    required this.id,
    required this.nickname,
    required this.host,
    required this.port,
    required this.path,
    this.deviceModel,
    this.firmwareVersion,
    this.lastSeen,
    this.lastConnected,
    required this.autoConnect,
    required this.notificationSettingsJson,
  });
  @override
  Map<String, Expression> toColumns(bool nullToAbsent) {
    final map = <String, Expression>{};
    map['id'] = Variable<String>(id);
    map['nickname'] = Variable<String>(nickname);
    map['host'] = Variable<String>(host);
    map['port'] = Variable<int>(port);
    map['path'] = Variable<String>(path);
    if (!nullToAbsent || deviceModel != null) {
      map['device_model'] = Variable<String>(deviceModel);
    }
    if (!nullToAbsent || firmwareVersion != null) {
      map['firmware_version'] = Variable<String>(firmwareVersion);
    }
    if (!nullToAbsent || lastSeen != null) {
      map['last_seen'] = Variable<DateTime>(lastSeen);
    }
    if (!nullToAbsent || lastConnected != null) {
      map['last_connected'] = Variable<DateTime>(lastConnected);
    }
    map['auto_connect'] = Variable<bool>(autoConnect);
    map['notification_settings_json'] = Variable<String>(
      notificationSettingsJson,
    );
    return map;
  }

  KnownDevicesCompanion toCompanion(bool nullToAbsent) {
    return KnownDevicesCompanion(
      id: Value(id),
      nickname: Value(nickname),
      host: Value(host),
      port: Value(port),
      path: Value(path),
      deviceModel: deviceModel == null && nullToAbsent
          ? const Value.absent()
          : Value(deviceModel),
      firmwareVersion: firmwareVersion == null && nullToAbsent
          ? const Value.absent()
          : Value(firmwareVersion),
      lastSeen: lastSeen == null && nullToAbsent
          ? const Value.absent()
          : Value(lastSeen),
      lastConnected: lastConnected == null && nullToAbsent
          ? const Value.absent()
          : Value(lastConnected),
      autoConnect: Value(autoConnect),
      notificationSettingsJson: Value(notificationSettingsJson),
    );
  }

  factory KnownDevice.fromJson(
    Map<String, dynamic> json, {
    ValueSerializer? serializer,
  }) {
    serializer ??= driftRuntimeOptions.defaultSerializer;
    return KnownDevice(
      id: serializer.fromJson<String>(json['id']),
      nickname: serializer.fromJson<String>(json['nickname']),
      host: serializer.fromJson<String>(json['host']),
      port: serializer.fromJson<int>(json['port']),
      path: serializer.fromJson<String>(json['path']),
      deviceModel: serializer.fromJson<String?>(json['deviceModel']),
      firmwareVersion: serializer.fromJson<String?>(json['firmwareVersion']),
      lastSeen: serializer.fromJson<DateTime?>(json['lastSeen']),
      lastConnected: serializer.fromJson<DateTime?>(json['lastConnected']),
      autoConnect: serializer.fromJson<bool>(json['autoConnect']),
      notificationSettingsJson: serializer.fromJson<String>(
        json['notificationSettingsJson'],
      ),
    );
  }
  @override
  Map<String, dynamic> toJson({ValueSerializer? serializer}) {
    serializer ??= driftRuntimeOptions.defaultSerializer;
    return <String, dynamic>{
      'id': serializer.toJson<String>(id),
      'nickname': serializer.toJson<String>(nickname),
      'host': serializer.toJson<String>(host),
      'port': serializer.toJson<int>(port),
      'path': serializer.toJson<String>(path),
      'deviceModel': serializer.toJson<String?>(deviceModel),
      'firmwareVersion': serializer.toJson<String?>(firmwareVersion),
      'lastSeen': serializer.toJson<DateTime?>(lastSeen),
      'lastConnected': serializer.toJson<DateTime?>(lastConnected),
      'autoConnect': serializer.toJson<bool>(autoConnect),
      'notificationSettingsJson': serializer.toJson<String>(
        notificationSettingsJson,
      ),
    };
  }

  KnownDevice copyWith({
    String? id,
    String? nickname,
    String? host,
    int? port,
    String? path,
    Value<String?> deviceModel = const Value.absent(),
    Value<String?> firmwareVersion = const Value.absent(),
    Value<DateTime?> lastSeen = const Value.absent(),
    Value<DateTime?> lastConnected = const Value.absent(),
    bool? autoConnect,
    String? notificationSettingsJson,
  }) => KnownDevice(
    id: id ?? this.id,
    nickname: nickname ?? this.nickname,
    host: host ?? this.host,
    port: port ?? this.port,
    path: path ?? this.path,
    deviceModel: deviceModel.present ? deviceModel.value : this.deviceModel,
    firmwareVersion: firmwareVersion.present
        ? firmwareVersion.value
        : this.firmwareVersion,
    lastSeen: lastSeen.present ? lastSeen.value : this.lastSeen,
    lastConnected: lastConnected.present
        ? lastConnected.value
        : this.lastConnected,
    autoConnect: autoConnect ?? this.autoConnect,
    notificationSettingsJson:
        notificationSettingsJson ?? this.notificationSettingsJson,
  );
  KnownDevice copyWithCompanion(KnownDevicesCompanion data) {
    return KnownDevice(
      id: data.id.present ? data.id.value : this.id,
      nickname: data.nickname.present ? data.nickname.value : this.nickname,
      host: data.host.present ? data.host.value : this.host,
      port: data.port.present ? data.port.value : this.port,
      path: data.path.present ? data.path.value : this.path,
      deviceModel: data.deviceModel.present
          ? data.deviceModel.value
          : this.deviceModel,
      firmwareVersion: data.firmwareVersion.present
          ? data.firmwareVersion.value
          : this.firmwareVersion,
      lastSeen: data.lastSeen.present ? data.lastSeen.value : this.lastSeen,
      lastConnected: data.lastConnected.present
          ? data.lastConnected.value
          : this.lastConnected,
      autoConnect: data.autoConnect.present
          ? data.autoConnect.value
          : this.autoConnect,
      notificationSettingsJson: data.notificationSettingsJson.present
          ? data.notificationSettingsJson.value
          : this.notificationSettingsJson,
    );
  }

  @override
  String toString() {
    return (StringBuffer('KnownDevice(')
          ..write('id: $id, ')
          ..write('nickname: $nickname, ')
          ..write('host: $host, ')
          ..write('port: $port, ')
          ..write('path: $path, ')
          ..write('deviceModel: $deviceModel, ')
          ..write('firmwareVersion: $firmwareVersion, ')
          ..write('lastSeen: $lastSeen, ')
          ..write('lastConnected: $lastConnected, ')
          ..write('autoConnect: $autoConnect, ')
          ..write('notificationSettingsJson: $notificationSettingsJson')
          ..write(')'))
        .toString();
  }

  @override
  int get hashCode => Object.hash(
    id,
    nickname,
    host,
    port,
    path,
    deviceModel,
    firmwareVersion,
    lastSeen,
    lastConnected,
    autoConnect,
    notificationSettingsJson,
  );
  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      (other is KnownDevice &&
          other.id == this.id &&
          other.nickname == this.nickname &&
          other.host == this.host &&
          other.port == this.port &&
          other.path == this.path &&
          other.deviceModel == this.deviceModel &&
          other.firmwareVersion == this.firmwareVersion &&
          other.lastSeen == this.lastSeen &&
          other.lastConnected == this.lastConnected &&
          other.autoConnect == this.autoConnect &&
          other.notificationSettingsJson == this.notificationSettingsJson);
}

class KnownDevicesCompanion extends UpdateCompanion<KnownDevice> {
  final Value<String> id;
  final Value<String> nickname;
  final Value<String> host;
  final Value<int> port;
  final Value<String> path;
  final Value<String?> deviceModel;
  final Value<String?> firmwareVersion;
  final Value<DateTime?> lastSeen;
  final Value<DateTime?> lastConnected;
  final Value<bool> autoConnect;
  final Value<String> notificationSettingsJson;
  final Value<int> rowid;
  const KnownDevicesCompanion({
    this.id = const Value.absent(),
    this.nickname = const Value.absent(),
    this.host = const Value.absent(),
    this.port = const Value.absent(),
    this.path = const Value.absent(),
    this.deviceModel = const Value.absent(),
    this.firmwareVersion = const Value.absent(),
    this.lastSeen = const Value.absent(),
    this.lastConnected = const Value.absent(),
    this.autoConnect = const Value.absent(),
    this.notificationSettingsJson = const Value.absent(),
    this.rowid = const Value.absent(),
  });
  KnownDevicesCompanion.insert({
    required String id,
    required String nickname,
    required String host,
    this.port = const Value.absent(),
    this.path = const Value.absent(),
    this.deviceModel = const Value.absent(),
    this.firmwareVersion = const Value.absent(),
    this.lastSeen = const Value.absent(),
    this.lastConnected = const Value.absent(),
    this.autoConnect = const Value.absent(),
    this.notificationSettingsJson = const Value.absent(),
    this.rowid = const Value.absent(),
  }) : id = Value(id),
       nickname = Value(nickname),
       host = Value(host);
  static Insertable<KnownDevice> custom({
    Expression<String>? id,
    Expression<String>? nickname,
    Expression<String>? host,
    Expression<int>? port,
    Expression<String>? path,
    Expression<String>? deviceModel,
    Expression<String>? firmwareVersion,
    Expression<DateTime>? lastSeen,
    Expression<DateTime>? lastConnected,
    Expression<bool>? autoConnect,
    Expression<String>? notificationSettingsJson,
    Expression<int>? rowid,
  }) {
    return RawValuesInsertable({
      if (id != null) 'id': id,
      if (nickname != null) 'nickname': nickname,
      if (host != null) 'host': host,
      if (port != null) 'port': port,
      if (path != null) 'path': path,
      if (deviceModel != null) 'device_model': deviceModel,
      if (firmwareVersion != null) 'firmware_version': firmwareVersion,
      if (lastSeen != null) 'last_seen': lastSeen,
      if (lastConnected != null) 'last_connected': lastConnected,
      if (autoConnect != null) 'auto_connect': autoConnect,
      if (notificationSettingsJson != null)
        'notification_settings_json': notificationSettingsJson,
      if (rowid != null) 'rowid': rowid,
    });
  }

  KnownDevicesCompanion copyWith({
    Value<String>? id,
    Value<String>? nickname,
    Value<String>? host,
    Value<int>? port,
    Value<String>? path,
    Value<String?>? deviceModel,
    Value<String?>? firmwareVersion,
    Value<DateTime?>? lastSeen,
    Value<DateTime?>? lastConnected,
    Value<bool>? autoConnect,
    Value<String>? notificationSettingsJson,
    Value<int>? rowid,
  }) {
    return KnownDevicesCompanion(
      id: id ?? this.id,
      nickname: nickname ?? this.nickname,
      host: host ?? this.host,
      port: port ?? this.port,
      path: path ?? this.path,
      deviceModel: deviceModel ?? this.deviceModel,
      firmwareVersion: firmwareVersion ?? this.firmwareVersion,
      lastSeen: lastSeen ?? this.lastSeen,
      lastConnected: lastConnected ?? this.lastConnected,
      autoConnect: autoConnect ?? this.autoConnect,
      notificationSettingsJson:
          notificationSettingsJson ?? this.notificationSettingsJson,
      rowid: rowid ?? this.rowid,
    );
  }

  @override
  Map<String, Expression> toColumns(bool nullToAbsent) {
    final map = <String, Expression>{};
    if (id.present) {
      map['id'] = Variable<String>(id.value);
    }
    if (nickname.present) {
      map['nickname'] = Variable<String>(nickname.value);
    }
    if (host.present) {
      map['host'] = Variable<String>(host.value);
    }
    if (port.present) {
      map['port'] = Variable<int>(port.value);
    }
    if (path.present) {
      map['path'] = Variable<String>(path.value);
    }
    if (deviceModel.present) {
      map['device_model'] = Variable<String>(deviceModel.value);
    }
    if (firmwareVersion.present) {
      map['firmware_version'] = Variable<String>(firmwareVersion.value);
    }
    if (lastSeen.present) {
      map['last_seen'] = Variable<DateTime>(lastSeen.value);
    }
    if (lastConnected.present) {
      map['last_connected'] = Variable<DateTime>(lastConnected.value);
    }
    if (autoConnect.present) {
      map['auto_connect'] = Variable<bool>(autoConnect.value);
    }
    if (notificationSettingsJson.present) {
      map['notification_settings_json'] = Variable<String>(
        notificationSettingsJson.value,
      );
    }
    if (rowid.present) {
      map['rowid'] = Variable<int>(rowid.value);
    }
    return map;
  }

  @override
  String toString() {
    return (StringBuffer('KnownDevicesCompanion(')
          ..write('id: $id, ')
          ..write('nickname: $nickname, ')
          ..write('host: $host, ')
          ..write('port: $port, ')
          ..write('path: $path, ')
          ..write('deviceModel: $deviceModel, ')
          ..write('firmwareVersion: $firmwareVersion, ')
          ..write('lastSeen: $lastSeen, ')
          ..write('lastConnected: $lastConnected, ')
          ..write('autoConnect: $autoConnect, ')
          ..write('notificationSettingsJson: $notificationSettingsJson, ')
          ..write('rowid: $rowid')
          ..write(')'))
        .toString();
  }
}

class $DryingCyclesTable extends DryingCycles
    with TableInfo<$DryingCyclesTable, DryingCycle> {
  @override
  final GeneratedDatabase attachedDatabase;
  final String? _alias;
  $DryingCyclesTable(this.attachedDatabase, [this._alias]);
  static const VerificationMeta _idMeta = const VerificationMeta('id');
  @override
  late final GeneratedColumn<String> id = GeneratedColumn<String>(
    'id',
    aliasedName,
    false,
    type: DriftSqlType.string,
    requiredDuringInsert: true,
  );
  static const VerificationMeta _knownDeviceIdMeta = const VerificationMeta(
    'knownDeviceId',
  );
  @override
  late final GeneratedColumn<String> knownDeviceId = GeneratedColumn<String>(
    'known_device_id',
    aliasedName,
    false,
    type: DriftSqlType.string,
    requiredDuringInsert: true,
  );
  static const VerificationMeta _profileIdMeta = const VerificationMeta(
    'profileId',
  );
  @override
  late final GeneratedColumn<String> profileId = GeneratedColumn<String>(
    'profile_id',
    aliasedName,
    true,
    type: DriftSqlType.string,
    requiredDuringInsert: false,
  );
  static const VerificationMeta _materialNameMeta = const VerificationMeta(
    'materialName',
  );
  @override
  late final GeneratedColumn<String> materialName = GeneratedColumn<String>(
    'material_name',
    aliasedName,
    false,
    type: DriftSqlType.string,
    requiredDuringInsert: true,
  );
  static const VerificationMeta _targetTempCMeta = const VerificationMeta(
    'targetTempC',
  );
  @override
  late final GeneratedColumn<double> targetTempC = GeneratedColumn<double>(
    'target_temp_c',
    aliasedName,
    true,
    type: DriftSqlType.double,
    requiredDuringInsert: false,
  );
  static const VerificationMeta _maxDurationMinMeta = const VerificationMeta(
    'maxDurationMin',
  );
  @override
  late final GeneratedColumn<int> maxDurationMin = GeneratedColumn<int>(
    'max_duration_min',
    aliasedName,
    true,
    type: DriftSqlType.int,
    requiredDuringInsert: false,
  );
  static const VerificationMeta _targetHumidityPctMeta = const VerificationMeta(
    'targetHumidityPct',
  );
  @override
  late final GeneratedColumn<double> targetHumidityPct =
      GeneratedColumn<double>(
        'target_humidity_pct',
        aliasedName,
        true,
        type: DriftSqlType.double,
        requiredDuringInsert: false,
      );
  static const VerificationMeta _startTimeMeta = const VerificationMeta(
    'startTime',
  );
  @override
  late final GeneratedColumn<DateTime> startTime = GeneratedColumn<DateTime>(
    'start_time',
    aliasedName,
    false,
    type: DriftSqlType.dateTime,
    requiredDuringInsert: true,
  );
  static const VerificationMeta _endTimeMeta = const VerificationMeta(
    'endTime',
  );
  @override
  late final GeneratedColumn<DateTime> endTime = GeneratedColumn<DateTime>(
    'end_time',
    aliasedName,
    true,
    type: DriftSqlType.dateTime,
    requiredDuringInsert: false,
  );
  static const VerificationMeta _avgTempCMeta = const VerificationMeta(
    'avgTempC',
  );
  @override
  late final GeneratedColumn<double> avgTempC = GeneratedColumn<double>(
    'avg_temp_c',
    aliasedName,
    true,
    type: DriftSqlType.double,
    requiredDuringInsert: false,
  );
  static const VerificationMeta _maxTempCMeta = const VerificationMeta(
    'maxTempC',
  );
  @override
  late final GeneratedColumn<double> maxTempC = GeneratedColumn<double>(
    'max_temp_c',
    aliasedName,
    true,
    type: DriftSqlType.double,
    requiredDuringInsert: false,
  );
  static const VerificationMeta _avgHumidityPctMeta = const VerificationMeta(
    'avgHumidityPct',
  );
  @override
  late final GeneratedColumn<double> avgHumidityPct = GeneratedColumn<double>(
    'avg_humidity_pct',
    aliasedName,
    true,
    type: DriftSqlType.double,
    requiredDuringInsert: false,
  );
  static const VerificationMeta _stopReasonMeta = const VerificationMeta(
    'stopReason',
  );
  @override
  late final GeneratedColumn<String> stopReason = GeneratedColumn<String>(
    'stop_reason',
    aliasedName,
    true,
    type: DriftSqlType.string,
    requiredDuringInsert: false,
  );
  static const VerificationMeta _statusMeta = const VerificationMeta('status');
  @override
  late final GeneratedColumn<String> status = GeneratedColumn<String>(
    'status',
    aliasedName,
    false,
    type: DriftSqlType.string,
    requiredDuringInsert: true,
  );
  static const VerificationMeta _notesMeta = const VerificationMeta('notes');
  @override
  late final GeneratedColumn<String> notes = GeneratedColumn<String>(
    'notes',
    aliasedName,
    false,
    type: DriftSqlType.string,
    requiredDuringInsert: false,
    defaultValue: const Constant(''),
  );
  static const VerificationMeta _hasSamplesMeta = const VerificationMeta(
    'hasSamples',
  );
  @override
  late final GeneratedColumn<bool> hasSamples = GeneratedColumn<bool>(
    'has_samples',
    aliasedName,
    false,
    type: DriftSqlType.bool,
    requiredDuringInsert: false,
    defaultConstraints: GeneratedColumn.constraintIsAlways(
      'CHECK ("has_samples" IN (0, 1))',
    ),
    defaultValue: const Constant(false),
  );
  @override
  List<GeneratedColumn> get $columns => [
    id,
    knownDeviceId,
    profileId,
    materialName,
    targetTempC,
    maxDurationMin,
    targetHumidityPct,
    startTime,
    endTime,
    avgTempC,
    maxTempC,
    avgHumidityPct,
    stopReason,
    status,
    notes,
    hasSamples,
  ];
  @override
  String get aliasedName => _alias ?? actualTableName;
  @override
  String get actualTableName => $name;
  static const String $name = 'drying_cycles';
  @override
  VerificationContext validateIntegrity(
    Insertable<DryingCycle> instance, {
    bool isInserting = false,
  }) {
    final context = VerificationContext();
    final data = instance.toColumns(true);
    if (data.containsKey('id')) {
      context.handle(_idMeta, id.isAcceptableOrUnknown(data['id']!, _idMeta));
    } else if (isInserting) {
      context.missing(_idMeta);
    }
    if (data.containsKey('known_device_id')) {
      context.handle(
        _knownDeviceIdMeta,
        knownDeviceId.isAcceptableOrUnknown(
          data['known_device_id']!,
          _knownDeviceIdMeta,
        ),
      );
    } else if (isInserting) {
      context.missing(_knownDeviceIdMeta);
    }
    if (data.containsKey('profile_id')) {
      context.handle(
        _profileIdMeta,
        profileId.isAcceptableOrUnknown(data['profile_id']!, _profileIdMeta),
      );
    }
    if (data.containsKey('material_name')) {
      context.handle(
        _materialNameMeta,
        materialName.isAcceptableOrUnknown(
          data['material_name']!,
          _materialNameMeta,
        ),
      );
    } else if (isInserting) {
      context.missing(_materialNameMeta);
    }
    if (data.containsKey('target_temp_c')) {
      context.handle(
        _targetTempCMeta,
        targetTempC.isAcceptableOrUnknown(
          data['target_temp_c']!,
          _targetTempCMeta,
        ),
      );
    }
    if (data.containsKey('max_duration_min')) {
      context.handle(
        _maxDurationMinMeta,
        maxDurationMin.isAcceptableOrUnknown(
          data['max_duration_min']!,
          _maxDurationMinMeta,
        ),
      );
    }
    if (data.containsKey('target_humidity_pct')) {
      context.handle(
        _targetHumidityPctMeta,
        targetHumidityPct.isAcceptableOrUnknown(
          data['target_humidity_pct']!,
          _targetHumidityPctMeta,
        ),
      );
    }
    if (data.containsKey('start_time')) {
      context.handle(
        _startTimeMeta,
        startTime.isAcceptableOrUnknown(data['start_time']!, _startTimeMeta),
      );
    } else if (isInserting) {
      context.missing(_startTimeMeta);
    }
    if (data.containsKey('end_time')) {
      context.handle(
        _endTimeMeta,
        endTime.isAcceptableOrUnknown(data['end_time']!, _endTimeMeta),
      );
    }
    if (data.containsKey('avg_temp_c')) {
      context.handle(
        _avgTempCMeta,
        avgTempC.isAcceptableOrUnknown(data['avg_temp_c']!, _avgTempCMeta),
      );
    }
    if (data.containsKey('max_temp_c')) {
      context.handle(
        _maxTempCMeta,
        maxTempC.isAcceptableOrUnknown(data['max_temp_c']!, _maxTempCMeta),
      );
    }
    if (data.containsKey('avg_humidity_pct')) {
      context.handle(
        _avgHumidityPctMeta,
        avgHumidityPct.isAcceptableOrUnknown(
          data['avg_humidity_pct']!,
          _avgHumidityPctMeta,
        ),
      );
    }
    if (data.containsKey('stop_reason')) {
      context.handle(
        _stopReasonMeta,
        stopReason.isAcceptableOrUnknown(data['stop_reason']!, _stopReasonMeta),
      );
    }
    if (data.containsKey('status')) {
      context.handle(
        _statusMeta,
        status.isAcceptableOrUnknown(data['status']!, _statusMeta),
      );
    } else if (isInserting) {
      context.missing(_statusMeta);
    }
    if (data.containsKey('notes')) {
      context.handle(
        _notesMeta,
        notes.isAcceptableOrUnknown(data['notes']!, _notesMeta),
      );
    }
    if (data.containsKey('has_samples')) {
      context.handle(
        _hasSamplesMeta,
        hasSamples.isAcceptableOrUnknown(data['has_samples']!, _hasSamplesMeta),
      );
    }
    return context;
  }

  @override
  Set<GeneratedColumn> get $primaryKey => {id};
  @override
  DryingCycle map(Map<String, dynamic> data, {String? tablePrefix}) {
    final effectivePrefix = tablePrefix != null ? '$tablePrefix.' : '';
    return DryingCycle(
      id: attachedDatabase.typeMapping.read(
        DriftSqlType.string,
        data['${effectivePrefix}id'],
      )!,
      knownDeviceId: attachedDatabase.typeMapping.read(
        DriftSqlType.string,
        data['${effectivePrefix}known_device_id'],
      )!,
      profileId: attachedDatabase.typeMapping.read(
        DriftSqlType.string,
        data['${effectivePrefix}profile_id'],
      ),
      materialName: attachedDatabase.typeMapping.read(
        DriftSqlType.string,
        data['${effectivePrefix}material_name'],
      )!,
      targetTempC: attachedDatabase.typeMapping.read(
        DriftSqlType.double,
        data['${effectivePrefix}target_temp_c'],
      ),
      maxDurationMin: attachedDatabase.typeMapping.read(
        DriftSqlType.int,
        data['${effectivePrefix}max_duration_min'],
      ),
      targetHumidityPct: attachedDatabase.typeMapping.read(
        DriftSqlType.double,
        data['${effectivePrefix}target_humidity_pct'],
      ),
      startTime: attachedDatabase.typeMapping.read(
        DriftSqlType.dateTime,
        data['${effectivePrefix}start_time'],
      )!,
      endTime: attachedDatabase.typeMapping.read(
        DriftSqlType.dateTime,
        data['${effectivePrefix}end_time'],
      ),
      avgTempC: attachedDatabase.typeMapping.read(
        DriftSqlType.double,
        data['${effectivePrefix}avg_temp_c'],
      ),
      maxTempC: attachedDatabase.typeMapping.read(
        DriftSqlType.double,
        data['${effectivePrefix}max_temp_c'],
      ),
      avgHumidityPct: attachedDatabase.typeMapping.read(
        DriftSqlType.double,
        data['${effectivePrefix}avg_humidity_pct'],
      ),
      stopReason: attachedDatabase.typeMapping.read(
        DriftSqlType.string,
        data['${effectivePrefix}stop_reason'],
      ),
      status: attachedDatabase.typeMapping.read(
        DriftSqlType.string,
        data['${effectivePrefix}status'],
      )!,
      notes: attachedDatabase.typeMapping.read(
        DriftSqlType.string,
        data['${effectivePrefix}notes'],
      )!,
      hasSamples: attachedDatabase.typeMapping.read(
        DriftSqlType.bool,
        data['${effectivePrefix}has_samples'],
      )!,
    );
  }

  @override
  $DryingCyclesTable createAlias(String alias) {
    return $DryingCyclesTable(attachedDatabase, alias);
  }
}

class DryingCycle extends DataClass implements Insertable<DryingCycle> {
  final String id;
  final String knownDeviceId;
  final String? profileId;
  final String materialName;
  final double? targetTempC;
  final int? maxDurationMin;
  final double? targetHumidityPct;
  final DateTime startTime;
  final DateTime? endTime;
  final double? avgTempC;
  final double? maxTempC;
  final double? avgHumidityPct;
  final String? stopReason;
  final String status;
  final String notes;
  final bool hasSamples;
  const DryingCycle({
    required this.id,
    required this.knownDeviceId,
    this.profileId,
    required this.materialName,
    this.targetTempC,
    this.maxDurationMin,
    this.targetHumidityPct,
    required this.startTime,
    this.endTime,
    this.avgTempC,
    this.maxTempC,
    this.avgHumidityPct,
    this.stopReason,
    required this.status,
    required this.notes,
    required this.hasSamples,
  });
  @override
  Map<String, Expression> toColumns(bool nullToAbsent) {
    final map = <String, Expression>{};
    map['id'] = Variable<String>(id);
    map['known_device_id'] = Variable<String>(knownDeviceId);
    if (!nullToAbsent || profileId != null) {
      map['profile_id'] = Variable<String>(profileId);
    }
    map['material_name'] = Variable<String>(materialName);
    if (!nullToAbsent || targetTempC != null) {
      map['target_temp_c'] = Variable<double>(targetTempC);
    }
    if (!nullToAbsent || maxDurationMin != null) {
      map['max_duration_min'] = Variable<int>(maxDurationMin);
    }
    if (!nullToAbsent || targetHumidityPct != null) {
      map['target_humidity_pct'] = Variable<double>(targetHumidityPct);
    }
    map['start_time'] = Variable<DateTime>(startTime);
    if (!nullToAbsent || endTime != null) {
      map['end_time'] = Variable<DateTime>(endTime);
    }
    if (!nullToAbsent || avgTempC != null) {
      map['avg_temp_c'] = Variable<double>(avgTempC);
    }
    if (!nullToAbsent || maxTempC != null) {
      map['max_temp_c'] = Variable<double>(maxTempC);
    }
    if (!nullToAbsent || avgHumidityPct != null) {
      map['avg_humidity_pct'] = Variable<double>(avgHumidityPct);
    }
    if (!nullToAbsent || stopReason != null) {
      map['stop_reason'] = Variable<String>(stopReason);
    }
    map['status'] = Variable<String>(status);
    map['notes'] = Variable<String>(notes);
    map['has_samples'] = Variable<bool>(hasSamples);
    return map;
  }

  DryingCyclesCompanion toCompanion(bool nullToAbsent) {
    return DryingCyclesCompanion(
      id: Value(id),
      knownDeviceId: Value(knownDeviceId),
      profileId: profileId == null && nullToAbsent
          ? const Value.absent()
          : Value(profileId),
      materialName: Value(materialName),
      targetTempC: targetTempC == null && nullToAbsent
          ? const Value.absent()
          : Value(targetTempC),
      maxDurationMin: maxDurationMin == null && nullToAbsent
          ? const Value.absent()
          : Value(maxDurationMin),
      targetHumidityPct: targetHumidityPct == null && nullToAbsent
          ? const Value.absent()
          : Value(targetHumidityPct),
      startTime: Value(startTime),
      endTime: endTime == null && nullToAbsent
          ? const Value.absent()
          : Value(endTime),
      avgTempC: avgTempC == null && nullToAbsent
          ? const Value.absent()
          : Value(avgTempC),
      maxTempC: maxTempC == null && nullToAbsent
          ? const Value.absent()
          : Value(maxTempC),
      avgHumidityPct: avgHumidityPct == null && nullToAbsent
          ? const Value.absent()
          : Value(avgHumidityPct),
      stopReason: stopReason == null && nullToAbsent
          ? const Value.absent()
          : Value(stopReason),
      status: Value(status),
      notes: Value(notes),
      hasSamples: Value(hasSamples),
    );
  }

  factory DryingCycle.fromJson(
    Map<String, dynamic> json, {
    ValueSerializer? serializer,
  }) {
    serializer ??= driftRuntimeOptions.defaultSerializer;
    return DryingCycle(
      id: serializer.fromJson<String>(json['id']),
      knownDeviceId: serializer.fromJson<String>(json['knownDeviceId']),
      profileId: serializer.fromJson<String?>(json['profileId']),
      materialName: serializer.fromJson<String>(json['materialName']),
      targetTempC: serializer.fromJson<double?>(json['targetTempC']),
      maxDurationMin: serializer.fromJson<int?>(json['maxDurationMin']),
      targetHumidityPct: serializer.fromJson<double?>(
        json['targetHumidityPct'],
      ),
      startTime: serializer.fromJson<DateTime>(json['startTime']),
      endTime: serializer.fromJson<DateTime?>(json['endTime']),
      avgTempC: serializer.fromJson<double?>(json['avgTempC']),
      maxTempC: serializer.fromJson<double?>(json['maxTempC']),
      avgHumidityPct: serializer.fromJson<double?>(json['avgHumidityPct']),
      stopReason: serializer.fromJson<String?>(json['stopReason']),
      status: serializer.fromJson<String>(json['status']),
      notes: serializer.fromJson<String>(json['notes']),
      hasSamples: serializer.fromJson<bool>(json['hasSamples']),
    );
  }
  @override
  Map<String, dynamic> toJson({ValueSerializer? serializer}) {
    serializer ??= driftRuntimeOptions.defaultSerializer;
    return <String, dynamic>{
      'id': serializer.toJson<String>(id),
      'knownDeviceId': serializer.toJson<String>(knownDeviceId),
      'profileId': serializer.toJson<String?>(profileId),
      'materialName': serializer.toJson<String>(materialName),
      'targetTempC': serializer.toJson<double?>(targetTempC),
      'maxDurationMin': serializer.toJson<int?>(maxDurationMin),
      'targetHumidityPct': serializer.toJson<double?>(targetHumidityPct),
      'startTime': serializer.toJson<DateTime>(startTime),
      'endTime': serializer.toJson<DateTime?>(endTime),
      'avgTempC': serializer.toJson<double?>(avgTempC),
      'maxTempC': serializer.toJson<double?>(maxTempC),
      'avgHumidityPct': serializer.toJson<double?>(avgHumidityPct),
      'stopReason': serializer.toJson<String?>(stopReason),
      'status': serializer.toJson<String>(status),
      'notes': serializer.toJson<String>(notes),
      'hasSamples': serializer.toJson<bool>(hasSamples),
    };
  }

  DryingCycle copyWith({
    String? id,
    String? knownDeviceId,
    Value<String?> profileId = const Value.absent(),
    String? materialName,
    Value<double?> targetTempC = const Value.absent(),
    Value<int?> maxDurationMin = const Value.absent(),
    Value<double?> targetHumidityPct = const Value.absent(),
    DateTime? startTime,
    Value<DateTime?> endTime = const Value.absent(),
    Value<double?> avgTempC = const Value.absent(),
    Value<double?> maxTempC = const Value.absent(),
    Value<double?> avgHumidityPct = const Value.absent(),
    Value<String?> stopReason = const Value.absent(),
    String? status,
    String? notes,
    bool? hasSamples,
  }) => DryingCycle(
    id: id ?? this.id,
    knownDeviceId: knownDeviceId ?? this.knownDeviceId,
    profileId: profileId.present ? profileId.value : this.profileId,
    materialName: materialName ?? this.materialName,
    targetTempC: targetTempC.present ? targetTempC.value : this.targetTempC,
    maxDurationMin: maxDurationMin.present
        ? maxDurationMin.value
        : this.maxDurationMin,
    targetHumidityPct: targetHumidityPct.present
        ? targetHumidityPct.value
        : this.targetHumidityPct,
    startTime: startTime ?? this.startTime,
    endTime: endTime.present ? endTime.value : this.endTime,
    avgTempC: avgTempC.present ? avgTempC.value : this.avgTempC,
    maxTempC: maxTempC.present ? maxTempC.value : this.maxTempC,
    avgHumidityPct: avgHumidityPct.present
        ? avgHumidityPct.value
        : this.avgHumidityPct,
    stopReason: stopReason.present ? stopReason.value : this.stopReason,
    status: status ?? this.status,
    notes: notes ?? this.notes,
    hasSamples: hasSamples ?? this.hasSamples,
  );
  DryingCycle copyWithCompanion(DryingCyclesCompanion data) {
    return DryingCycle(
      id: data.id.present ? data.id.value : this.id,
      knownDeviceId: data.knownDeviceId.present
          ? data.knownDeviceId.value
          : this.knownDeviceId,
      profileId: data.profileId.present ? data.profileId.value : this.profileId,
      materialName: data.materialName.present
          ? data.materialName.value
          : this.materialName,
      targetTempC: data.targetTempC.present
          ? data.targetTempC.value
          : this.targetTempC,
      maxDurationMin: data.maxDurationMin.present
          ? data.maxDurationMin.value
          : this.maxDurationMin,
      targetHumidityPct: data.targetHumidityPct.present
          ? data.targetHumidityPct.value
          : this.targetHumidityPct,
      startTime: data.startTime.present ? data.startTime.value : this.startTime,
      endTime: data.endTime.present ? data.endTime.value : this.endTime,
      avgTempC: data.avgTempC.present ? data.avgTempC.value : this.avgTempC,
      maxTempC: data.maxTempC.present ? data.maxTempC.value : this.maxTempC,
      avgHumidityPct: data.avgHumidityPct.present
          ? data.avgHumidityPct.value
          : this.avgHumidityPct,
      stopReason: data.stopReason.present
          ? data.stopReason.value
          : this.stopReason,
      status: data.status.present ? data.status.value : this.status,
      notes: data.notes.present ? data.notes.value : this.notes,
      hasSamples: data.hasSamples.present
          ? data.hasSamples.value
          : this.hasSamples,
    );
  }

  @override
  String toString() {
    return (StringBuffer('DryingCycle(')
          ..write('id: $id, ')
          ..write('knownDeviceId: $knownDeviceId, ')
          ..write('profileId: $profileId, ')
          ..write('materialName: $materialName, ')
          ..write('targetTempC: $targetTempC, ')
          ..write('maxDurationMin: $maxDurationMin, ')
          ..write('targetHumidityPct: $targetHumidityPct, ')
          ..write('startTime: $startTime, ')
          ..write('endTime: $endTime, ')
          ..write('avgTempC: $avgTempC, ')
          ..write('maxTempC: $maxTempC, ')
          ..write('avgHumidityPct: $avgHumidityPct, ')
          ..write('stopReason: $stopReason, ')
          ..write('status: $status, ')
          ..write('notes: $notes, ')
          ..write('hasSamples: $hasSamples')
          ..write(')'))
        .toString();
  }

  @override
  int get hashCode => Object.hash(
    id,
    knownDeviceId,
    profileId,
    materialName,
    targetTempC,
    maxDurationMin,
    targetHumidityPct,
    startTime,
    endTime,
    avgTempC,
    maxTempC,
    avgHumidityPct,
    stopReason,
    status,
    notes,
    hasSamples,
  );
  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      (other is DryingCycle &&
          other.id == this.id &&
          other.knownDeviceId == this.knownDeviceId &&
          other.profileId == this.profileId &&
          other.materialName == this.materialName &&
          other.targetTempC == this.targetTempC &&
          other.maxDurationMin == this.maxDurationMin &&
          other.targetHumidityPct == this.targetHumidityPct &&
          other.startTime == this.startTime &&
          other.endTime == this.endTime &&
          other.avgTempC == this.avgTempC &&
          other.maxTempC == this.maxTempC &&
          other.avgHumidityPct == this.avgHumidityPct &&
          other.stopReason == this.stopReason &&
          other.status == this.status &&
          other.notes == this.notes &&
          other.hasSamples == this.hasSamples);
}

class DryingCyclesCompanion extends UpdateCompanion<DryingCycle> {
  final Value<String> id;
  final Value<String> knownDeviceId;
  final Value<String?> profileId;
  final Value<String> materialName;
  final Value<double?> targetTempC;
  final Value<int?> maxDurationMin;
  final Value<double?> targetHumidityPct;
  final Value<DateTime> startTime;
  final Value<DateTime?> endTime;
  final Value<double?> avgTempC;
  final Value<double?> maxTempC;
  final Value<double?> avgHumidityPct;
  final Value<String?> stopReason;
  final Value<String> status;
  final Value<String> notes;
  final Value<bool> hasSamples;
  final Value<int> rowid;
  const DryingCyclesCompanion({
    this.id = const Value.absent(),
    this.knownDeviceId = const Value.absent(),
    this.profileId = const Value.absent(),
    this.materialName = const Value.absent(),
    this.targetTempC = const Value.absent(),
    this.maxDurationMin = const Value.absent(),
    this.targetHumidityPct = const Value.absent(),
    this.startTime = const Value.absent(),
    this.endTime = const Value.absent(),
    this.avgTempC = const Value.absent(),
    this.maxTempC = const Value.absent(),
    this.avgHumidityPct = const Value.absent(),
    this.stopReason = const Value.absent(),
    this.status = const Value.absent(),
    this.notes = const Value.absent(),
    this.hasSamples = const Value.absent(),
    this.rowid = const Value.absent(),
  });
  DryingCyclesCompanion.insert({
    required String id,
    required String knownDeviceId,
    this.profileId = const Value.absent(),
    required String materialName,
    this.targetTempC = const Value.absent(),
    this.maxDurationMin = const Value.absent(),
    this.targetHumidityPct = const Value.absent(),
    required DateTime startTime,
    this.endTime = const Value.absent(),
    this.avgTempC = const Value.absent(),
    this.maxTempC = const Value.absent(),
    this.avgHumidityPct = const Value.absent(),
    this.stopReason = const Value.absent(),
    required String status,
    this.notes = const Value.absent(),
    this.hasSamples = const Value.absent(),
    this.rowid = const Value.absent(),
  }) : id = Value(id),
       knownDeviceId = Value(knownDeviceId),
       materialName = Value(materialName),
       startTime = Value(startTime),
       status = Value(status);
  static Insertable<DryingCycle> custom({
    Expression<String>? id,
    Expression<String>? knownDeviceId,
    Expression<String>? profileId,
    Expression<String>? materialName,
    Expression<double>? targetTempC,
    Expression<int>? maxDurationMin,
    Expression<double>? targetHumidityPct,
    Expression<DateTime>? startTime,
    Expression<DateTime>? endTime,
    Expression<double>? avgTempC,
    Expression<double>? maxTempC,
    Expression<double>? avgHumidityPct,
    Expression<String>? stopReason,
    Expression<String>? status,
    Expression<String>? notes,
    Expression<bool>? hasSamples,
    Expression<int>? rowid,
  }) {
    return RawValuesInsertable({
      if (id != null) 'id': id,
      if (knownDeviceId != null) 'known_device_id': knownDeviceId,
      if (profileId != null) 'profile_id': profileId,
      if (materialName != null) 'material_name': materialName,
      if (targetTempC != null) 'target_temp_c': targetTempC,
      if (maxDurationMin != null) 'max_duration_min': maxDurationMin,
      if (targetHumidityPct != null) 'target_humidity_pct': targetHumidityPct,
      if (startTime != null) 'start_time': startTime,
      if (endTime != null) 'end_time': endTime,
      if (avgTempC != null) 'avg_temp_c': avgTempC,
      if (maxTempC != null) 'max_temp_c': maxTempC,
      if (avgHumidityPct != null) 'avg_humidity_pct': avgHumidityPct,
      if (stopReason != null) 'stop_reason': stopReason,
      if (status != null) 'status': status,
      if (notes != null) 'notes': notes,
      if (hasSamples != null) 'has_samples': hasSamples,
      if (rowid != null) 'rowid': rowid,
    });
  }

  DryingCyclesCompanion copyWith({
    Value<String>? id,
    Value<String>? knownDeviceId,
    Value<String?>? profileId,
    Value<String>? materialName,
    Value<double?>? targetTempC,
    Value<int?>? maxDurationMin,
    Value<double?>? targetHumidityPct,
    Value<DateTime>? startTime,
    Value<DateTime?>? endTime,
    Value<double?>? avgTempC,
    Value<double?>? maxTempC,
    Value<double?>? avgHumidityPct,
    Value<String?>? stopReason,
    Value<String>? status,
    Value<String>? notes,
    Value<bool>? hasSamples,
    Value<int>? rowid,
  }) {
    return DryingCyclesCompanion(
      id: id ?? this.id,
      knownDeviceId: knownDeviceId ?? this.knownDeviceId,
      profileId: profileId ?? this.profileId,
      materialName: materialName ?? this.materialName,
      targetTempC: targetTempC ?? this.targetTempC,
      maxDurationMin: maxDurationMin ?? this.maxDurationMin,
      targetHumidityPct: targetHumidityPct ?? this.targetHumidityPct,
      startTime: startTime ?? this.startTime,
      endTime: endTime ?? this.endTime,
      avgTempC: avgTempC ?? this.avgTempC,
      maxTempC: maxTempC ?? this.maxTempC,
      avgHumidityPct: avgHumidityPct ?? this.avgHumidityPct,
      stopReason: stopReason ?? this.stopReason,
      status: status ?? this.status,
      notes: notes ?? this.notes,
      hasSamples: hasSamples ?? this.hasSamples,
      rowid: rowid ?? this.rowid,
    );
  }

  @override
  Map<String, Expression> toColumns(bool nullToAbsent) {
    final map = <String, Expression>{};
    if (id.present) {
      map['id'] = Variable<String>(id.value);
    }
    if (knownDeviceId.present) {
      map['known_device_id'] = Variable<String>(knownDeviceId.value);
    }
    if (profileId.present) {
      map['profile_id'] = Variable<String>(profileId.value);
    }
    if (materialName.present) {
      map['material_name'] = Variable<String>(materialName.value);
    }
    if (targetTempC.present) {
      map['target_temp_c'] = Variable<double>(targetTempC.value);
    }
    if (maxDurationMin.present) {
      map['max_duration_min'] = Variable<int>(maxDurationMin.value);
    }
    if (targetHumidityPct.present) {
      map['target_humidity_pct'] = Variable<double>(targetHumidityPct.value);
    }
    if (startTime.present) {
      map['start_time'] = Variable<DateTime>(startTime.value);
    }
    if (endTime.present) {
      map['end_time'] = Variable<DateTime>(endTime.value);
    }
    if (avgTempC.present) {
      map['avg_temp_c'] = Variable<double>(avgTempC.value);
    }
    if (maxTempC.present) {
      map['max_temp_c'] = Variable<double>(maxTempC.value);
    }
    if (avgHumidityPct.present) {
      map['avg_humidity_pct'] = Variable<double>(avgHumidityPct.value);
    }
    if (stopReason.present) {
      map['stop_reason'] = Variable<String>(stopReason.value);
    }
    if (status.present) {
      map['status'] = Variable<String>(status.value);
    }
    if (notes.present) {
      map['notes'] = Variable<String>(notes.value);
    }
    if (hasSamples.present) {
      map['has_samples'] = Variable<bool>(hasSamples.value);
    }
    if (rowid.present) {
      map['rowid'] = Variable<int>(rowid.value);
    }
    return map;
  }

  @override
  String toString() {
    return (StringBuffer('DryingCyclesCompanion(')
          ..write('id: $id, ')
          ..write('knownDeviceId: $knownDeviceId, ')
          ..write('profileId: $profileId, ')
          ..write('materialName: $materialName, ')
          ..write('targetTempC: $targetTempC, ')
          ..write('maxDurationMin: $maxDurationMin, ')
          ..write('targetHumidityPct: $targetHumidityPct, ')
          ..write('startTime: $startTime, ')
          ..write('endTime: $endTime, ')
          ..write('avgTempC: $avgTempC, ')
          ..write('maxTempC: $maxTempC, ')
          ..write('avgHumidityPct: $avgHumidityPct, ')
          ..write('stopReason: $stopReason, ')
          ..write('status: $status, ')
          ..write('notes: $notes, ')
          ..write('hasSamples: $hasSamples, ')
          ..write('rowid: $rowid')
          ..write(')'))
        .toString();
  }
}

class $CycleSamplesTable extends CycleSamples
    with TableInfo<$CycleSamplesTable, CycleSample> {
  @override
  final GeneratedDatabase attachedDatabase;
  final String? _alias;
  $CycleSamplesTable(this.attachedDatabase, [this._alias]);
  static const VerificationMeta _idMeta = const VerificationMeta('id');
  @override
  late final GeneratedColumn<int> id = GeneratedColumn<int>(
    'id',
    aliasedName,
    false,
    hasAutoIncrement: true,
    type: DriftSqlType.int,
    requiredDuringInsert: false,
    defaultConstraints: GeneratedColumn.constraintIsAlways(
      'PRIMARY KEY AUTOINCREMENT',
    ),
  );
  static const VerificationMeta _cycleIdMeta = const VerificationMeta(
    'cycleId',
  );
  @override
  late final GeneratedColumn<String> cycleId = GeneratedColumn<String>(
    'cycle_id',
    aliasedName,
    false,
    type: DriftSqlType.string,
    requiredDuringInsert: true,
  );
  static const VerificationMeta _tSecMeta = const VerificationMeta('tSec');
  @override
  late final GeneratedColumn<int> tSec = GeneratedColumn<int>(
    't_sec',
    aliasedName,
    false,
    type: DriftSqlType.int,
    requiredDuringInsert: true,
  );
  static const VerificationMeta _tempCMeta = const VerificationMeta('tempC');
  @override
  late final GeneratedColumn<double> tempC = GeneratedColumn<double>(
    'temp_c',
    aliasedName,
    true,
    type: DriftSqlType.double,
    requiredDuringInsert: false,
  );
  static const VerificationMeta _humidityPctMeta = const VerificationMeta(
    'humidityPct',
  );
  @override
  late final GeneratedColumn<double> humidityPct = GeneratedColumn<double>(
    'humidity_pct',
    aliasedName,
    true,
    type: DriftSqlType.double,
    requiredDuringInsert: false,
  );
  static const VerificationMeta _heaterPctMeta = const VerificationMeta(
    'heaterPct',
  );
  @override
  late final GeneratedColumn<double> heaterPct = GeneratedColumn<double>(
    'heater_pct',
    aliasedName,
    false,
    type: DriftSqlType.double,
    requiredDuringInsert: false,
    defaultValue: const Constant(0),
  );
  static const VerificationMeta _fanPctMeta = const VerificationMeta('fanPct');
  @override
  late final GeneratedColumn<double> fanPct = GeneratedColumn<double>(
    'fan_pct',
    aliasedName,
    false,
    type: DriftSqlType.double,
    requiredDuringInsert: false,
    defaultValue: const Constant(0),
  );
  @override
  List<GeneratedColumn> get $columns => [
    id,
    cycleId,
    tSec,
    tempC,
    humidityPct,
    heaterPct,
    fanPct,
  ];
  @override
  String get aliasedName => _alias ?? actualTableName;
  @override
  String get actualTableName => $name;
  static const String $name = 'cycle_samples';
  @override
  VerificationContext validateIntegrity(
    Insertable<CycleSample> instance, {
    bool isInserting = false,
  }) {
    final context = VerificationContext();
    final data = instance.toColumns(true);
    if (data.containsKey('id')) {
      context.handle(_idMeta, id.isAcceptableOrUnknown(data['id']!, _idMeta));
    }
    if (data.containsKey('cycle_id')) {
      context.handle(
        _cycleIdMeta,
        cycleId.isAcceptableOrUnknown(data['cycle_id']!, _cycleIdMeta),
      );
    } else if (isInserting) {
      context.missing(_cycleIdMeta);
    }
    if (data.containsKey('t_sec')) {
      context.handle(
        _tSecMeta,
        tSec.isAcceptableOrUnknown(data['t_sec']!, _tSecMeta),
      );
    } else if (isInserting) {
      context.missing(_tSecMeta);
    }
    if (data.containsKey('temp_c')) {
      context.handle(
        _tempCMeta,
        tempC.isAcceptableOrUnknown(data['temp_c']!, _tempCMeta),
      );
    }
    if (data.containsKey('humidity_pct')) {
      context.handle(
        _humidityPctMeta,
        humidityPct.isAcceptableOrUnknown(
          data['humidity_pct']!,
          _humidityPctMeta,
        ),
      );
    }
    if (data.containsKey('heater_pct')) {
      context.handle(
        _heaterPctMeta,
        heaterPct.isAcceptableOrUnknown(data['heater_pct']!, _heaterPctMeta),
      );
    }
    if (data.containsKey('fan_pct')) {
      context.handle(
        _fanPctMeta,
        fanPct.isAcceptableOrUnknown(data['fan_pct']!, _fanPctMeta),
      );
    }
    return context;
  }

  @override
  Set<GeneratedColumn> get $primaryKey => {id};
  @override
  CycleSample map(Map<String, dynamic> data, {String? tablePrefix}) {
    final effectivePrefix = tablePrefix != null ? '$tablePrefix.' : '';
    return CycleSample(
      id: attachedDatabase.typeMapping.read(
        DriftSqlType.int,
        data['${effectivePrefix}id'],
      )!,
      cycleId: attachedDatabase.typeMapping.read(
        DriftSqlType.string,
        data['${effectivePrefix}cycle_id'],
      )!,
      tSec: attachedDatabase.typeMapping.read(
        DriftSqlType.int,
        data['${effectivePrefix}t_sec'],
      )!,
      tempC: attachedDatabase.typeMapping.read(
        DriftSqlType.double,
        data['${effectivePrefix}temp_c'],
      ),
      humidityPct: attachedDatabase.typeMapping.read(
        DriftSqlType.double,
        data['${effectivePrefix}humidity_pct'],
      ),
      heaterPct: attachedDatabase.typeMapping.read(
        DriftSqlType.double,
        data['${effectivePrefix}heater_pct'],
      )!,
      fanPct: attachedDatabase.typeMapping.read(
        DriftSqlType.double,
        data['${effectivePrefix}fan_pct'],
      )!,
    );
  }

  @override
  $CycleSamplesTable createAlias(String alias) {
    return $CycleSamplesTable(attachedDatabase, alias);
  }
}

class CycleSample extends DataClass implements Insertable<CycleSample> {
  final int id;
  final String cycleId;
  final int tSec;
  final double? tempC;
  final double? humidityPct;
  final double heaterPct;
  final double fanPct;
  const CycleSample({
    required this.id,
    required this.cycleId,
    required this.tSec,
    this.tempC,
    this.humidityPct,
    required this.heaterPct,
    required this.fanPct,
  });
  @override
  Map<String, Expression> toColumns(bool nullToAbsent) {
    final map = <String, Expression>{};
    map['id'] = Variable<int>(id);
    map['cycle_id'] = Variable<String>(cycleId);
    map['t_sec'] = Variable<int>(tSec);
    if (!nullToAbsent || tempC != null) {
      map['temp_c'] = Variable<double>(tempC);
    }
    if (!nullToAbsent || humidityPct != null) {
      map['humidity_pct'] = Variable<double>(humidityPct);
    }
    map['heater_pct'] = Variable<double>(heaterPct);
    map['fan_pct'] = Variable<double>(fanPct);
    return map;
  }

  CycleSamplesCompanion toCompanion(bool nullToAbsent) {
    return CycleSamplesCompanion(
      id: Value(id),
      cycleId: Value(cycleId),
      tSec: Value(tSec),
      tempC: tempC == null && nullToAbsent
          ? const Value.absent()
          : Value(tempC),
      humidityPct: humidityPct == null && nullToAbsent
          ? const Value.absent()
          : Value(humidityPct),
      heaterPct: Value(heaterPct),
      fanPct: Value(fanPct),
    );
  }

  factory CycleSample.fromJson(
    Map<String, dynamic> json, {
    ValueSerializer? serializer,
  }) {
    serializer ??= driftRuntimeOptions.defaultSerializer;
    return CycleSample(
      id: serializer.fromJson<int>(json['id']),
      cycleId: serializer.fromJson<String>(json['cycleId']),
      tSec: serializer.fromJson<int>(json['tSec']),
      tempC: serializer.fromJson<double?>(json['tempC']),
      humidityPct: serializer.fromJson<double?>(json['humidityPct']),
      heaterPct: serializer.fromJson<double>(json['heaterPct']),
      fanPct: serializer.fromJson<double>(json['fanPct']),
    );
  }
  @override
  Map<String, dynamic> toJson({ValueSerializer? serializer}) {
    serializer ??= driftRuntimeOptions.defaultSerializer;
    return <String, dynamic>{
      'id': serializer.toJson<int>(id),
      'cycleId': serializer.toJson<String>(cycleId),
      'tSec': serializer.toJson<int>(tSec),
      'tempC': serializer.toJson<double?>(tempC),
      'humidityPct': serializer.toJson<double?>(humidityPct),
      'heaterPct': serializer.toJson<double>(heaterPct),
      'fanPct': serializer.toJson<double>(fanPct),
    };
  }

  CycleSample copyWith({
    int? id,
    String? cycleId,
    int? tSec,
    Value<double?> tempC = const Value.absent(),
    Value<double?> humidityPct = const Value.absent(),
    double? heaterPct,
    double? fanPct,
  }) => CycleSample(
    id: id ?? this.id,
    cycleId: cycleId ?? this.cycleId,
    tSec: tSec ?? this.tSec,
    tempC: tempC.present ? tempC.value : this.tempC,
    humidityPct: humidityPct.present ? humidityPct.value : this.humidityPct,
    heaterPct: heaterPct ?? this.heaterPct,
    fanPct: fanPct ?? this.fanPct,
  );
  CycleSample copyWithCompanion(CycleSamplesCompanion data) {
    return CycleSample(
      id: data.id.present ? data.id.value : this.id,
      cycleId: data.cycleId.present ? data.cycleId.value : this.cycleId,
      tSec: data.tSec.present ? data.tSec.value : this.tSec,
      tempC: data.tempC.present ? data.tempC.value : this.tempC,
      humidityPct: data.humidityPct.present
          ? data.humidityPct.value
          : this.humidityPct,
      heaterPct: data.heaterPct.present ? data.heaterPct.value : this.heaterPct,
      fanPct: data.fanPct.present ? data.fanPct.value : this.fanPct,
    );
  }

  @override
  String toString() {
    return (StringBuffer('CycleSample(')
          ..write('id: $id, ')
          ..write('cycleId: $cycleId, ')
          ..write('tSec: $tSec, ')
          ..write('tempC: $tempC, ')
          ..write('humidityPct: $humidityPct, ')
          ..write('heaterPct: $heaterPct, ')
          ..write('fanPct: $fanPct')
          ..write(')'))
        .toString();
  }

  @override
  int get hashCode =>
      Object.hash(id, cycleId, tSec, tempC, humidityPct, heaterPct, fanPct);
  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      (other is CycleSample &&
          other.id == this.id &&
          other.cycleId == this.cycleId &&
          other.tSec == this.tSec &&
          other.tempC == this.tempC &&
          other.humidityPct == this.humidityPct &&
          other.heaterPct == this.heaterPct &&
          other.fanPct == this.fanPct);
}

class CycleSamplesCompanion extends UpdateCompanion<CycleSample> {
  final Value<int> id;
  final Value<String> cycleId;
  final Value<int> tSec;
  final Value<double?> tempC;
  final Value<double?> humidityPct;
  final Value<double> heaterPct;
  final Value<double> fanPct;
  const CycleSamplesCompanion({
    this.id = const Value.absent(),
    this.cycleId = const Value.absent(),
    this.tSec = const Value.absent(),
    this.tempC = const Value.absent(),
    this.humidityPct = const Value.absent(),
    this.heaterPct = const Value.absent(),
    this.fanPct = const Value.absent(),
  });
  CycleSamplesCompanion.insert({
    this.id = const Value.absent(),
    required String cycleId,
    required int tSec,
    this.tempC = const Value.absent(),
    this.humidityPct = const Value.absent(),
    this.heaterPct = const Value.absent(),
    this.fanPct = const Value.absent(),
  }) : cycleId = Value(cycleId),
       tSec = Value(tSec);
  static Insertable<CycleSample> custom({
    Expression<int>? id,
    Expression<String>? cycleId,
    Expression<int>? tSec,
    Expression<double>? tempC,
    Expression<double>? humidityPct,
    Expression<double>? heaterPct,
    Expression<double>? fanPct,
  }) {
    return RawValuesInsertable({
      if (id != null) 'id': id,
      if (cycleId != null) 'cycle_id': cycleId,
      if (tSec != null) 't_sec': tSec,
      if (tempC != null) 'temp_c': tempC,
      if (humidityPct != null) 'humidity_pct': humidityPct,
      if (heaterPct != null) 'heater_pct': heaterPct,
      if (fanPct != null) 'fan_pct': fanPct,
    });
  }

  CycleSamplesCompanion copyWith({
    Value<int>? id,
    Value<String>? cycleId,
    Value<int>? tSec,
    Value<double?>? tempC,
    Value<double?>? humidityPct,
    Value<double>? heaterPct,
    Value<double>? fanPct,
  }) {
    return CycleSamplesCompanion(
      id: id ?? this.id,
      cycleId: cycleId ?? this.cycleId,
      tSec: tSec ?? this.tSec,
      tempC: tempC ?? this.tempC,
      humidityPct: humidityPct ?? this.humidityPct,
      heaterPct: heaterPct ?? this.heaterPct,
      fanPct: fanPct ?? this.fanPct,
    );
  }

  @override
  Map<String, Expression> toColumns(bool nullToAbsent) {
    final map = <String, Expression>{};
    if (id.present) {
      map['id'] = Variable<int>(id.value);
    }
    if (cycleId.present) {
      map['cycle_id'] = Variable<String>(cycleId.value);
    }
    if (tSec.present) {
      map['t_sec'] = Variable<int>(tSec.value);
    }
    if (tempC.present) {
      map['temp_c'] = Variable<double>(tempC.value);
    }
    if (humidityPct.present) {
      map['humidity_pct'] = Variable<double>(humidityPct.value);
    }
    if (heaterPct.present) {
      map['heater_pct'] = Variable<double>(heaterPct.value);
    }
    if (fanPct.present) {
      map['fan_pct'] = Variable<double>(fanPct.value);
    }
    return map;
  }

  @override
  String toString() {
    return (StringBuffer('CycleSamplesCompanion(')
          ..write('id: $id, ')
          ..write('cycleId: $cycleId, ')
          ..write('tSec: $tSec, ')
          ..write('tempC: $tempC, ')
          ..write('humidityPct: $humidityPct, ')
          ..write('heaterPct: $heaterPct, ')
          ..write('fanPct: $fanPct')
          ..write(')'))
        .toString();
  }
}

class $PendingCommandsTable extends PendingCommands
    with TableInfo<$PendingCommandsTable, PendingCommand> {
  @override
  final GeneratedDatabase attachedDatabase;
  final String? _alias;
  $PendingCommandsTable(this.attachedDatabase, [this._alias]);
  static const VerificationMeta _idMeta = const VerificationMeta('id');
  @override
  late final GeneratedColumn<String> id = GeneratedColumn<String>(
    'id',
    aliasedName,
    false,
    type: DriftSqlType.string,
    requiredDuringInsert: true,
  );
  static const VerificationMeta _knownDeviceIdMeta = const VerificationMeta(
    'knownDeviceId',
  );
  @override
  late final GeneratedColumn<String> knownDeviceId = GeneratedColumn<String>(
    'known_device_id',
    aliasedName,
    false,
    type: DriftSqlType.string,
    requiredDuringInsert: true,
  );
  static const VerificationMeta _topicMeta = const VerificationMeta('topic');
  @override
  late final GeneratedColumn<String> topic = GeneratedColumn<String>(
    'topic',
    aliasedName,
    false,
    type: DriftSqlType.string,
    requiredDuringInsert: true,
  );
  static const VerificationMeta _payloadJsonMeta = const VerificationMeta(
    'payloadJson',
  );
  @override
  late final GeneratedColumn<String> payloadJson = GeneratedColumn<String>(
    'payload_json',
    aliasedName,
    false,
    type: DriftSqlType.string,
    requiredDuringInsert: true,
  );
  static const VerificationMeta _createdAtMeta = const VerificationMeta(
    'createdAt',
  );
  @override
  late final GeneratedColumn<DateTime> createdAt = GeneratedColumn<DateTime>(
    'created_at',
    aliasedName,
    false,
    type: DriftSqlType.dateTime,
    requiredDuringInsert: true,
  );
  static const VerificationMeta _statusMeta = const VerificationMeta('status');
  @override
  late final GeneratedColumn<String> status = GeneratedColumn<String>(
    'status',
    aliasedName,
    false,
    type: DriftSqlType.string,
    requiredDuringInsert: true,
  );
  static const VerificationMeta _lastErrorMeta = const VerificationMeta(
    'lastError',
  );
  @override
  late final GeneratedColumn<String> lastError = GeneratedColumn<String>(
    'last_error',
    aliasedName,
    true,
    type: DriftSqlType.string,
    requiredDuringInsert: false,
  );
  @override
  List<GeneratedColumn> get $columns => [
    id,
    knownDeviceId,
    topic,
    payloadJson,
    createdAt,
    status,
    lastError,
  ];
  @override
  String get aliasedName => _alias ?? actualTableName;
  @override
  String get actualTableName => $name;
  static const String $name = 'pending_commands';
  @override
  VerificationContext validateIntegrity(
    Insertable<PendingCommand> instance, {
    bool isInserting = false,
  }) {
    final context = VerificationContext();
    final data = instance.toColumns(true);
    if (data.containsKey('id')) {
      context.handle(_idMeta, id.isAcceptableOrUnknown(data['id']!, _idMeta));
    } else if (isInserting) {
      context.missing(_idMeta);
    }
    if (data.containsKey('known_device_id')) {
      context.handle(
        _knownDeviceIdMeta,
        knownDeviceId.isAcceptableOrUnknown(
          data['known_device_id']!,
          _knownDeviceIdMeta,
        ),
      );
    } else if (isInserting) {
      context.missing(_knownDeviceIdMeta);
    }
    if (data.containsKey('topic')) {
      context.handle(
        _topicMeta,
        topic.isAcceptableOrUnknown(data['topic']!, _topicMeta),
      );
    } else if (isInserting) {
      context.missing(_topicMeta);
    }
    if (data.containsKey('payload_json')) {
      context.handle(
        _payloadJsonMeta,
        payloadJson.isAcceptableOrUnknown(
          data['payload_json']!,
          _payloadJsonMeta,
        ),
      );
    } else if (isInserting) {
      context.missing(_payloadJsonMeta);
    }
    if (data.containsKey('created_at')) {
      context.handle(
        _createdAtMeta,
        createdAt.isAcceptableOrUnknown(data['created_at']!, _createdAtMeta),
      );
    } else if (isInserting) {
      context.missing(_createdAtMeta);
    }
    if (data.containsKey('status')) {
      context.handle(
        _statusMeta,
        status.isAcceptableOrUnknown(data['status']!, _statusMeta),
      );
    } else if (isInserting) {
      context.missing(_statusMeta);
    }
    if (data.containsKey('last_error')) {
      context.handle(
        _lastErrorMeta,
        lastError.isAcceptableOrUnknown(data['last_error']!, _lastErrorMeta),
      );
    }
    return context;
  }

  @override
  Set<GeneratedColumn> get $primaryKey => {id};
  @override
  PendingCommand map(Map<String, dynamic> data, {String? tablePrefix}) {
    final effectivePrefix = tablePrefix != null ? '$tablePrefix.' : '';
    return PendingCommand(
      id: attachedDatabase.typeMapping.read(
        DriftSqlType.string,
        data['${effectivePrefix}id'],
      )!,
      knownDeviceId: attachedDatabase.typeMapping.read(
        DriftSqlType.string,
        data['${effectivePrefix}known_device_id'],
      )!,
      topic: attachedDatabase.typeMapping.read(
        DriftSqlType.string,
        data['${effectivePrefix}topic'],
      )!,
      payloadJson: attachedDatabase.typeMapping.read(
        DriftSqlType.string,
        data['${effectivePrefix}payload_json'],
      )!,
      createdAt: attachedDatabase.typeMapping.read(
        DriftSqlType.dateTime,
        data['${effectivePrefix}created_at'],
      )!,
      status: attachedDatabase.typeMapping.read(
        DriftSqlType.string,
        data['${effectivePrefix}status'],
      )!,
      lastError: attachedDatabase.typeMapping.read(
        DriftSqlType.string,
        data['${effectivePrefix}last_error'],
      ),
    );
  }

  @override
  $PendingCommandsTable createAlias(String alias) {
    return $PendingCommandsTable(attachedDatabase, alias);
  }
}

class PendingCommand extends DataClass implements Insertable<PendingCommand> {
  final String id;
  final String knownDeviceId;
  final String topic;
  final String payloadJson;
  final DateTime createdAt;
  final String status;
  final String? lastError;
  const PendingCommand({
    required this.id,
    required this.knownDeviceId,
    required this.topic,
    required this.payloadJson,
    required this.createdAt,
    required this.status,
    this.lastError,
  });
  @override
  Map<String, Expression> toColumns(bool nullToAbsent) {
    final map = <String, Expression>{};
    map['id'] = Variable<String>(id);
    map['known_device_id'] = Variable<String>(knownDeviceId);
    map['topic'] = Variable<String>(topic);
    map['payload_json'] = Variable<String>(payloadJson);
    map['created_at'] = Variable<DateTime>(createdAt);
    map['status'] = Variable<String>(status);
    if (!nullToAbsent || lastError != null) {
      map['last_error'] = Variable<String>(lastError);
    }
    return map;
  }

  PendingCommandsCompanion toCompanion(bool nullToAbsent) {
    return PendingCommandsCompanion(
      id: Value(id),
      knownDeviceId: Value(knownDeviceId),
      topic: Value(topic),
      payloadJson: Value(payloadJson),
      createdAt: Value(createdAt),
      status: Value(status),
      lastError: lastError == null && nullToAbsent
          ? const Value.absent()
          : Value(lastError),
    );
  }

  factory PendingCommand.fromJson(
    Map<String, dynamic> json, {
    ValueSerializer? serializer,
  }) {
    serializer ??= driftRuntimeOptions.defaultSerializer;
    return PendingCommand(
      id: serializer.fromJson<String>(json['id']),
      knownDeviceId: serializer.fromJson<String>(json['knownDeviceId']),
      topic: serializer.fromJson<String>(json['topic']),
      payloadJson: serializer.fromJson<String>(json['payloadJson']),
      createdAt: serializer.fromJson<DateTime>(json['createdAt']),
      status: serializer.fromJson<String>(json['status']),
      lastError: serializer.fromJson<String?>(json['lastError']),
    );
  }
  @override
  Map<String, dynamic> toJson({ValueSerializer? serializer}) {
    serializer ??= driftRuntimeOptions.defaultSerializer;
    return <String, dynamic>{
      'id': serializer.toJson<String>(id),
      'knownDeviceId': serializer.toJson<String>(knownDeviceId),
      'topic': serializer.toJson<String>(topic),
      'payloadJson': serializer.toJson<String>(payloadJson),
      'createdAt': serializer.toJson<DateTime>(createdAt),
      'status': serializer.toJson<String>(status),
      'lastError': serializer.toJson<String?>(lastError),
    };
  }

  PendingCommand copyWith({
    String? id,
    String? knownDeviceId,
    String? topic,
    String? payloadJson,
    DateTime? createdAt,
    String? status,
    Value<String?> lastError = const Value.absent(),
  }) => PendingCommand(
    id: id ?? this.id,
    knownDeviceId: knownDeviceId ?? this.knownDeviceId,
    topic: topic ?? this.topic,
    payloadJson: payloadJson ?? this.payloadJson,
    createdAt: createdAt ?? this.createdAt,
    status: status ?? this.status,
    lastError: lastError.present ? lastError.value : this.lastError,
  );
  PendingCommand copyWithCompanion(PendingCommandsCompanion data) {
    return PendingCommand(
      id: data.id.present ? data.id.value : this.id,
      knownDeviceId: data.knownDeviceId.present
          ? data.knownDeviceId.value
          : this.knownDeviceId,
      topic: data.topic.present ? data.topic.value : this.topic,
      payloadJson: data.payloadJson.present
          ? data.payloadJson.value
          : this.payloadJson,
      createdAt: data.createdAt.present ? data.createdAt.value : this.createdAt,
      status: data.status.present ? data.status.value : this.status,
      lastError: data.lastError.present ? data.lastError.value : this.lastError,
    );
  }

  @override
  String toString() {
    return (StringBuffer('PendingCommand(')
          ..write('id: $id, ')
          ..write('knownDeviceId: $knownDeviceId, ')
          ..write('topic: $topic, ')
          ..write('payloadJson: $payloadJson, ')
          ..write('createdAt: $createdAt, ')
          ..write('status: $status, ')
          ..write('lastError: $lastError')
          ..write(')'))
        .toString();
  }

  @override
  int get hashCode => Object.hash(
    id,
    knownDeviceId,
    topic,
    payloadJson,
    createdAt,
    status,
    lastError,
  );
  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      (other is PendingCommand &&
          other.id == this.id &&
          other.knownDeviceId == this.knownDeviceId &&
          other.topic == this.topic &&
          other.payloadJson == this.payloadJson &&
          other.createdAt == this.createdAt &&
          other.status == this.status &&
          other.lastError == this.lastError);
}

class PendingCommandsCompanion extends UpdateCompanion<PendingCommand> {
  final Value<String> id;
  final Value<String> knownDeviceId;
  final Value<String> topic;
  final Value<String> payloadJson;
  final Value<DateTime> createdAt;
  final Value<String> status;
  final Value<String?> lastError;
  final Value<int> rowid;
  const PendingCommandsCompanion({
    this.id = const Value.absent(),
    this.knownDeviceId = const Value.absent(),
    this.topic = const Value.absent(),
    this.payloadJson = const Value.absent(),
    this.createdAt = const Value.absent(),
    this.status = const Value.absent(),
    this.lastError = const Value.absent(),
    this.rowid = const Value.absent(),
  });
  PendingCommandsCompanion.insert({
    required String id,
    required String knownDeviceId,
    required String topic,
    required String payloadJson,
    required DateTime createdAt,
    required String status,
    this.lastError = const Value.absent(),
    this.rowid = const Value.absent(),
  }) : id = Value(id),
       knownDeviceId = Value(knownDeviceId),
       topic = Value(topic),
       payloadJson = Value(payloadJson),
       createdAt = Value(createdAt),
       status = Value(status);
  static Insertable<PendingCommand> custom({
    Expression<String>? id,
    Expression<String>? knownDeviceId,
    Expression<String>? topic,
    Expression<String>? payloadJson,
    Expression<DateTime>? createdAt,
    Expression<String>? status,
    Expression<String>? lastError,
    Expression<int>? rowid,
  }) {
    return RawValuesInsertable({
      if (id != null) 'id': id,
      if (knownDeviceId != null) 'known_device_id': knownDeviceId,
      if (topic != null) 'topic': topic,
      if (payloadJson != null) 'payload_json': payloadJson,
      if (createdAt != null) 'created_at': createdAt,
      if (status != null) 'status': status,
      if (lastError != null) 'last_error': lastError,
      if (rowid != null) 'rowid': rowid,
    });
  }

  PendingCommandsCompanion copyWith({
    Value<String>? id,
    Value<String>? knownDeviceId,
    Value<String>? topic,
    Value<String>? payloadJson,
    Value<DateTime>? createdAt,
    Value<String>? status,
    Value<String?>? lastError,
    Value<int>? rowid,
  }) {
    return PendingCommandsCompanion(
      id: id ?? this.id,
      knownDeviceId: knownDeviceId ?? this.knownDeviceId,
      topic: topic ?? this.topic,
      payloadJson: payloadJson ?? this.payloadJson,
      createdAt: createdAt ?? this.createdAt,
      status: status ?? this.status,
      lastError: lastError ?? this.lastError,
      rowid: rowid ?? this.rowid,
    );
  }

  @override
  Map<String, Expression> toColumns(bool nullToAbsent) {
    final map = <String, Expression>{};
    if (id.present) {
      map['id'] = Variable<String>(id.value);
    }
    if (knownDeviceId.present) {
      map['known_device_id'] = Variable<String>(knownDeviceId.value);
    }
    if (topic.present) {
      map['topic'] = Variable<String>(topic.value);
    }
    if (payloadJson.present) {
      map['payload_json'] = Variable<String>(payloadJson.value);
    }
    if (createdAt.present) {
      map['created_at'] = Variable<DateTime>(createdAt.value);
    }
    if (status.present) {
      map['status'] = Variable<String>(status.value);
    }
    if (lastError.present) {
      map['last_error'] = Variable<String>(lastError.value);
    }
    if (rowid.present) {
      map['rowid'] = Variable<int>(rowid.value);
    }
    return map;
  }

  @override
  String toString() {
    return (StringBuffer('PendingCommandsCompanion(')
          ..write('id: $id, ')
          ..write('knownDeviceId: $knownDeviceId, ')
          ..write('topic: $topic, ')
          ..write('payloadJson: $payloadJson, ')
          ..write('createdAt: $createdAt, ')
          ..write('status: $status, ')
          ..write('lastError: $lastError, ')
          ..write('rowid: $rowid')
          ..write(')'))
        .toString();
  }
}

abstract class _$AppDatabase extends GeneratedDatabase {
  _$AppDatabase(QueryExecutor e) : super(e);
  $AppDatabaseManager get managers => $AppDatabaseManager(this);
  late final $KnownDevicesTable knownDevices = $KnownDevicesTable(this);
  late final $DryingCyclesTable dryingCycles = $DryingCyclesTable(this);
  late final $CycleSamplesTable cycleSamples = $CycleSamplesTable(this);
  late final $PendingCommandsTable pendingCommands = $PendingCommandsTable(
    this,
  );
  @override
  Iterable<TableInfo<Table, Object?>> get allTables =>
      allSchemaEntities.whereType<TableInfo<Table, Object?>>();
  @override
  List<DatabaseSchemaEntity> get allSchemaEntities => [
    knownDevices,
    dryingCycles,
    cycleSamples,
    pendingCommands,
  ];
}

typedef $$KnownDevicesTableCreateCompanionBuilder =
    KnownDevicesCompanion Function({
      required String id,
      required String nickname,
      required String host,
      Value<int> port,
      Value<String> path,
      Value<String?> deviceModel,
      Value<String?> firmwareVersion,
      Value<DateTime?> lastSeen,
      Value<DateTime?> lastConnected,
      Value<bool> autoConnect,
      Value<String> notificationSettingsJson,
      Value<int> rowid,
    });
typedef $$KnownDevicesTableUpdateCompanionBuilder =
    KnownDevicesCompanion Function({
      Value<String> id,
      Value<String> nickname,
      Value<String> host,
      Value<int> port,
      Value<String> path,
      Value<String?> deviceModel,
      Value<String?> firmwareVersion,
      Value<DateTime?> lastSeen,
      Value<DateTime?> lastConnected,
      Value<bool> autoConnect,
      Value<String> notificationSettingsJson,
      Value<int> rowid,
    });

class $$KnownDevicesTableFilterComposer
    extends Composer<_$AppDatabase, $KnownDevicesTable> {
  $$KnownDevicesTableFilterComposer({
    required super.$db,
    required super.$table,
    super.joinBuilder,
    super.$addJoinBuilderToRootComposer,
    super.$removeJoinBuilderFromRootComposer,
  });
  ColumnFilters<String> get id => $composableBuilder(
    column: $table.id,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<String> get nickname => $composableBuilder(
    column: $table.nickname,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<String> get host => $composableBuilder(
    column: $table.host,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<int> get port => $composableBuilder(
    column: $table.port,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<String> get path => $composableBuilder(
    column: $table.path,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<String> get deviceModel => $composableBuilder(
    column: $table.deviceModel,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<String> get firmwareVersion => $composableBuilder(
    column: $table.firmwareVersion,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<DateTime> get lastSeen => $composableBuilder(
    column: $table.lastSeen,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<DateTime> get lastConnected => $composableBuilder(
    column: $table.lastConnected,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<bool> get autoConnect => $composableBuilder(
    column: $table.autoConnect,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<String> get notificationSettingsJson => $composableBuilder(
    column: $table.notificationSettingsJson,
    builder: (column) => ColumnFilters(column),
  );
}

class $$KnownDevicesTableOrderingComposer
    extends Composer<_$AppDatabase, $KnownDevicesTable> {
  $$KnownDevicesTableOrderingComposer({
    required super.$db,
    required super.$table,
    super.joinBuilder,
    super.$addJoinBuilderToRootComposer,
    super.$removeJoinBuilderFromRootComposer,
  });
  ColumnOrderings<String> get id => $composableBuilder(
    column: $table.id,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<String> get nickname => $composableBuilder(
    column: $table.nickname,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<String> get host => $composableBuilder(
    column: $table.host,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<int> get port => $composableBuilder(
    column: $table.port,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<String> get path => $composableBuilder(
    column: $table.path,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<String> get deviceModel => $composableBuilder(
    column: $table.deviceModel,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<String> get firmwareVersion => $composableBuilder(
    column: $table.firmwareVersion,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<DateTime> get lastSeen => $composableBuilder(
    column: $table.lastSeen,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<DateTime> get lastConnected => $composableBuilder(
    column: $table.lastConnected,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<bool> get autoConnect => $composableBuilder(
    column: $table.autoConnect,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<String> get notificationSettingsJson => $composableBuilder(
    column: $table.notificationSettingsJson,
    builder: (column) => ColumnOrderings(column),
  );
}

class $$KnownDevicesTableAnnotationComposer
    extends Composer<_$AppDatabase, $KnownDevicesTable> {
  $$KnownDevicesTableAnnotationComposer({
    required super.$db,
    required super.$table,
    super.joinBuilder,
    super.$addJoinBuilderToRootComposer,
    super.$removeJoinBuilderFromRootComposer,
  });
  GeneratedColumn<String> get id =>
      $composableBuilder(column: $table.id, builder: (column) => column);

  GeneratedColumn<String> get nickname =>
      $composableBuilder(column: $table.nickname, builder: (column) => column);

  GeneratedColumn<String> get host =>
      $composableBuilder(column: $table.host, builder: (column) => column);

  GeneratedColumn<int> get port =>
      $composableBuilder(column: $table.port, builder: (column) => column);

  GeneratedColumn<String> get path =>
      $composableBuilder(column: $table.path, builder: (column) => column);

  GeneratedColumn<String> get deviceModel => $composableBuilder(
    column: $table.deviceModel,
    builder: (column) => column,
  );

  GeneratedColumn<String> get firmwareVersion => $composableBuilder(
    column: $table.firmwareVersion,
    builder: (column) => column,
  );

  GeneratedColumn<DateTime> get lastSeen =>
      $composableBuilder(column: $table.lastSeen, builder: (column) => column);

  GeneratedColumn<DateTime> get lastConnected => $composableBuilder(
    column: $table.lastConnected,
    builder: (column) => column,
  );

  GeneratedColumn<bool> get autoConnect => $composableBuilder(
    column: $table.autoConnect,
    builder: (column) => column,
  );

  GeneratedColumn<String> get notificationSettingsJson => $composableBuilder(
    column: $table.notificationSettingsJson,
    builder: (column) => column,
  );
}

class $$KnownDevicesTableTableManager
    extends
        RootTableManager<
          _$AppDatabase,
          $KnownDevicesTable,
          KnownDevice,
          $$KnownDevicesTableFilterComposer,
          $$KnownDevicesTableOrderingComposer,
          $$KnownDevicesTableAnnotationComposer,
          $$KnownDevicesTableCreateCompanionBuilder,
          $$KnownDevicesTableUpdateCompanionBuilder,
          (
            KnownDevice,
            BaseReferences<_$AppDatabase, $KnownDevicesTable, KnownDevice>,
          ),
          KnownDevice,
          PrefetchHooks Function()
        > {
  $$KnownDevicesTableTableManager(_$AppDatabase db, $KnownDevicesTable table)
    : super(
        TableManagerState(
          db: db,
          table: table,
          createFilteringComposer: () =>
              $$KnownDevicesTableFilterComposer($db: db, $table: table),
          createOrderingComposer: () =>
              $$KnownDevicesTableOrderingComposer($db: db, $table: table),
          createComputedFieldComposer: () =>
              $$KnownDevicesTableAnnotationComposer($db: db, $table: table),
          updateCompanionCallback:
              ({
                Value<String> id = const Value.absent(),
                Value<String> nickname = const Value.absent(),
                Value<String> host = const Value.absent(),
                Value<int> port = const Value.absent(),
                Value<String> path = const Value.absent(),
                Value<String?> deviceModel = const Value.absent(),
                Value<String?> firmwareVersion = const Value.absent(),
                Value<DateTime?> lastSeen = const Value.absent(),
                Value<DateTime?> lastConnected = const Value.absent(),
                Value<bool> autoConnect = const Value.absent(),
                Value<String> notificationSettingsJson = const Value.absent(),
                Value<int> rowid = const Value.absent(),
              }) => KnownDevicesCompanion(
                id: id,
                nickname: nickname,
                host: host,
                port: port,
                path: path,
                deviceModel: deviceModel,
                firmwareVersion: firmwareVersion,
                lastSeen: lastSeen,
                lastConnected: lastConnected,
                autoConnect: autoConnect,
                notificationSettingsJson: notificationSettingsJson,
                rowid: rowid,
              ),
          createCompanionCallback:
              ({
                required String id,
                required String nickname,
                required String host,
                Value<int> port = const Value.absent(),
                Value<String> path = const Value.absent(),
                Value<String?> deviceModel = const Value.absent(),
                Value<String?> firmwareVersion = const Value.absent(),
                Value<DateTime?> lastSeen = const Value.absent(),
                Value<DateTime?> lastConnected = const Value.absent(),
                Value<bool> autoConnect = const Value.absent(),
                Value<String> notificationSettingsJson = const Value.absent(),
                Value<int> rowid = const Value.absent(),
              }) => KnownDevicesCompanion.insert(
                id: id,
                nickname: nickname,
                host: host,
                port: port,
                path: path,
                deviceModel: deviceModel,
                firmwareVersion: firmwareVersion,
                lastSeen: lastSeen,
                lastConnected: lastConnected,
                autoConnect: autoConnect,
                notificationSettingsJson: notificationSettingsJson,
                rowid: rowid,
              ),
          withReferenceMapper: (p0) => p0
              .map((e) => (e.readTable(table), BaseReferences(db, table, e)))
              .toList(),
          prefetchHooksCallback: null,
        ),
      );
}

typedef $$KnownDevicesTableProcessedTableManager =
    ProcessedTableManager<
      _$AppDatabase,
      $KnownDevicesTable,
      KnownDevice,
      $$KnownDevicesTableFilterComposer,
      $$KnownDevicesTableOrderingComposer,
      $$KnownDevicesTableAnnotationComposer,
      $$KnownDevicesTableCreateCompanionBuilder,
      $$KnownDevicesTableUpdateCompanionBuilder,
      (
        KnownDevice,
        BaseReferences<_$AppDatabase, $KnownDevicesTable, KnownDevice>,
      ),
      KnownDevice,
      PrefetchHooks Function()
    >;
typedef $$DryingCyclesTableCreateCompanionBuilder =
    DryingCyclesCompanion Function({
      required String id,
      required String knownDeviceId,
      Value<String?> profileId,
      required String materialName,
      Value<double?> targetTempC,
      Value<int?> maxDurationMin,
      Value<double?> targetHumidityPct,
      required DateTime startTime,
      Value<DateTime?> endTime,
      Value<double?> avgTempC,
      Value<double?> maxTempC,
      Value<double?> avgHumidityPct,
      Value<String?> stopReason,
      required String status,
      Value<String> notes,
      Value<bool> hasSamples,
      Value<int> rowid,
    });
typedef $$DryingCyclesTableUpdateCompanionBuilder =
    DryingCyclesCompanion Function({
      Value<String> id,
      Value<String> knownDeviceId,
      Value<String?> profileId,
      Value<String> materialName,
      Value<double?> targetTempC,
      Value<int?> maxDurationMin,
      Value<double?> targetHumidityPct,
      Value<DateTime> startTime,
      Value<DateTime?> endTime,
      Value<double?> avgTempC,
      Value<double?> maxTempC,
      Value<double?> avgHumidityPct,
      Value<String?> stopReason,
      Value<String> status,
      Value<String> notes,
      Value<bool> hasSamples,
      Value<int> rowid,
    });

class $$DryingCyclesTableFilterComposer
    extends Composer<_$AppDatabase, $DryingCyclesTable> {
  $$DryingCyclesTableFilterComposer({
    required super.$db,
    required super.$table,
    super.joinBuilder,
    super.$addJoinBuilderToRootComposer,
    super.$removeJoinBuilderFromRootComposer,
  });
  ColumnFilters<String> get id => $composableBuilder(
    column: $table.id,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<String> get knownDeviceId => $composableBuilder(
    column: $table.knownDeviceId,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<String> get profileId => $composableBuilder(
    column: $table.profileId,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<String> get materialName => $composableBuilder(
    column: $table.materialName,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<double> get targetTempC => $composableBuilder(
    column: $table.targetTempC,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<int> get maxDurationMin => $composableBuilder(
    column: $table.maxDurationMin,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<double> get targetHumidityPct => $composableBuilder(
    column: $table.targetHumidityPct,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<DateTime> get startTime => $composableBuilder(
    column: $table.startTime,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<DateTime> get endTime => $composableBuilder(
    column: $table.endTime,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<double> get avgTempC => $composableBuilder(
    column: $table.avgTempC,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<double> get maxTempC => $composableBuilder(
    column: $table.maxTempC,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<double> get avgHumidityPct => $composableBuilder(
    column: $table.avgHumidityPct,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<String> get stopReason => $composableBuilder(
    column: $table.stopReason,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<String> get status => $composableBuilder(
    column: $table.status,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<String> get notes => $composableBuilder(
    column: $table.notes,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<bool> get hasSamples => $composableBuilder(
    column: $table.hasSamples,
    builder: (column) => ColumnFilters(column),
  );
}

class $$DryingCyclesTableOrderingComposer
    extends Composer<_$AppDatabase, $DryingCyclesTable> {
  $$DryingCyclesTableOrderingComposer({
    required super.$db,
    required super.$table,
    super.joinBuilder,
    super.$addJoinBuilderToRootComposer,
    super.$removeJoinBuilderFromRootComposer,
  });
  ColumnOrderings<String> get id => $composableBuilder(
    column: $table.id,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<String> get knownDeviceId => $composableBuilder(
    column: $table.knownDeviceId,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<String> get profileId => $composableBuilder(
    column: $table.profileId,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<String> get materialName => $composableBuilder(
    column: $table.materialName,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<double> get targetTempC => $composableBuilder(
    column: $table.targetTempC,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<int> get maxDurationMin => $composableBuilder(
    column: $table.maxDurationMin,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<double> get targetHumidityPct => $composableBuilder(
    column: $table.targetHumidityPct,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<DateTime> get startTime => $composableBuilder(
    column: $table.startTime,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<DateTime> get endTime => $composableBuilder(
    column: $table.endTime,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<double> get avgTempC => $composableBuilder(
    column: $table.avgTempC,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<double> get maxTempC => $composableBuilder(
    column: $table.maxTempC,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<double> get avgHumidityPct => $composableBuilder(
    column: $table.avgHumidityPct,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<String> get stopReason => $composableBuilder(
    column: $table.stopReason,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<String> get status => $composableBuilder(
    column: $table.status,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<String> get notes => $composableBuilder(
    column: $table.notes,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<bool> get hasSamples => $composableBuilder(
    column: $table.hasSamples,
    builder: (column) => ColumnOrderings(column),
  );
}

class $$DryingCyclesTableAnnotationComposer
    extends Composer<_$AppDatabase, $DryingCyclesTable> {
  $$DryingCyclesTableAnnotationComposer({
    required super.$db,
    required super.$table,
    super.joinBuilder,
    super.$addJoinBuilderToRootComposer,
    super.$removeJoinBuilderFromRootComposer,
  });
  GeneratedColumn<String> get id =>
      $composableBuilder(column: $table.id, builder: (column) => column);

  GeneratedColumn<String> get knownDeviceId => $composableBuilder(
    column: $table.knownDeviceId,
    builder: (column) => column,
  );

  GeneratedColumn<String> get profileId =>
      $composableBuilder(column: $table.profileId, builder: (column) => column);

  GeneratedColumn<String> get materialName => $composableBuilder(
    column: $table.materialName,
    builder: (column) => column,
  );

  GeneratedColumn<double> get targetTempC => $composableBuilder(
    column: $table.targetTempC,
    builder: (column) => column,
  );

  GeneratedColumn<int> get maxDurationMin => $composableBuilder(
    column: $table.maxDurationMin,
    builder: (column) => column,
  );

  GeneratedColumn<double> get targetHumidityPct => $composableBuilder(
    column: $table.targetHumidityPct,
    builder: (column) => column,
  );

  GeneratedColumn<DateTime> get startTime =>
      $composableBuilder(column: $table.startTime, builder: (column) => column);

  GeneratedColumn<DateTime> get endTime =>
      $composableBuilder(column: $table.endTime, builder: (column) => column);

  GeneratedColumn<double> get avgTempC =>
      $composableBuilder(column: $table.avgTempC, builder: (column) => column);

  GeneratedColumn<double> get maxTempC =>
      $composableBuilder(column: $table.maxTempC, builder: (column) => column);

  GeneratedColumn<double> get avgHumidityPct => $composableBuilder(
    column: $table.avgHumidityPct,
    builder: (column) => column,
  );

  GeneratedColumn<String> get stopReason => $composableBuilder(
    column: $table.stopReason,
    builder: (column) => column,
  );

  GeneratedColumn<String> get status =>
      $composableBuilder(column: $table.status, builder: (column) => column);

  GeneratedColumn<String> get notes =>
      $composableBuilder(column: $table.notes, builder: (column) => column);

  GeneratedColumn<bool> get hasSamples => $composableBuilder(
    column: $table.hasSamples,
    builder: (column) => column,
  );
}

class $$DryingCyclesTableTableManager
    extends
        RootTableManager<
          _$AppDatabase,
          $DryingCyclesTable,
          DryingCycle,
          $$DryingCyclesTableFilterComposer,
          $$DryingCyclesTableOrderingComposer,
          $$DryingCyclesTableAnnotationComposer,
          $$DryingCyclesTableCreateCompanionBuilder,
          $$DryingCyclesTableUpdateCompanionBuilder,
          (
            DryingCycle,
            BaseReferences<_$AppDatabase, $DryingCyclesTable, DryingCycle>,
          ),
          DryingCycle,
          PrefetchHooks Function()
        > {
  $$DryingCyclesTableTableManager(_$AppDatabase db, $DryingCyclesTable table)
    : super(
        TableManagerState(
          db: db,
          table: table,
          createFilteringComposer: () =>
              $$DryingCyclesTableFilterComposer($db: db, $table: table),
          createOrderingComposer: () =>
              $$DryingCyclesTableOrderingComposer($db: db, $table: table),
          createComputedFieldComposer: () =>
              $$DryingCyclesTableAnnotationComposer($db: db, $table: table),
          updateCompanionCallback:
              ({
                Value<String> id = const Value.absent(),
                Value<String> knownDeviceId = const Value.absent(),
                Value<String?> profileId = const Value.absent(),
                Value<String> materialName = const Value.absent(),
                Value<double?> targetTempC = const Value.absent(),
                Value<int?> maxDurationMin = const Value.absent(),
                Value<double?> targetHumidityPct = const Value.absent(),
                Value<DateTime> startTime = const Value.absent(),
                Value<DateTime?> endTime = const Value.absent(),
                Value<double?> avgTempC = const Value.absent(),
                Value<double?> maxTempC = const Value.absent(),
                Value<double?> avgHumidityPct = const Value.absent(),
                Value<String?> stopReason = const Value.absent(),
                Value<String> status = const Value.absent(),
                Value<String> notes = const Value.absent(),
                Value<bool> hasSamples = const Value.absent(),
                Value<int> rowid = const Value.absent(),
              }) => DryingCyclesCompanion(
                id: id,
                knownDeviceId: knownDeviceId,
                profileId: profileId,
                materialName: materialName,
                targetTempC: targetTempC,
                maxDurationMin: maxDurationMin,
                targetHumidityPct: targetHumidityPct,
                startTime: startTime,
                endTime: endTime,
                avgTempC: avgTempC,
                maxTempC: maxTempC,
                avgHumidityPct: avgHumidityPct,
                stopReason: stopReason,
                status: status,
                notes: notes,
                hasSamples: hasSamples,
                rowid: rowid,
              ),
          createCompanionCallback:
              ({
                required String id,
                required String knownDeviceId,
                Value<String?> profileId = const Value.absent(),
                required String materialName,
                Value<double?> targetTempC = const Value.absent(),
                Value<int?> maxDurationMin = const Value.absent(),
                Value<double?> targetHumidityPct = const Value.absent(),
                required DateTime startTime,
                Value<DateTime?> endTime = const Value.absent(),
                Value<double?> avgTempC = const Value.absent(),
                Value<double?> maxTempC = const Value.absent(),
                Value<double?> avgHumidityPct = const Value.absent(),
                Value<String?> stopReason = const Value.absent(),
                required String status,
                Value<String> notes = const Value.absent(),
                Value<bool> hasSamples = const Value.absent(),
                Value<int> rowid = const Value.absent(),
              }) => DryingCyclesCompanion.insert(
                id: id,
                knownDeviceId: knownDeviceId,
                profileId: profileId,
                materialName: materialName,
                targetTempC: targetTempC,
                maxDurationMin: maxDurationMin,
                targetHumidityPct: targetHumidityPct,
                startTime: startTime,
                endTime: endTime,
                avgTempC: avgTempC,
                maxTempC: maxTempC,
                avgHumidityPct: avgHumidityPct,
                stopReason: stopReason,
                status: status,
                notes: notes,
                hasSamples: hasSamples,
                rowid: rowid,
              ),
          withReferenceMapper: (p0) => p0
              .map((e) => (e.readTable(table), BaseReferences(db, table, e)))
              .toList(),
          prefetchHooksCallback: null,
        ),
      );
}

typedef $$DryingCyclesTableProcessedTableManager =
    ProcessedTableManager<
      _$AppDatabase,
      $DryingCyclesTable,
      DryingCycle,
      $$DryingCyclesTableFilterComposer,
      $$DryingCyclesTableOrderingComposer,
      $$DryingCyclesTableAnnotationComposer,
      $$DryingCyclesTableCreateCompanionBuilder,
      $$DryingCyclesTableUpdateCompanionBuilder,
      (
        DryingCycle,
        BaseReferences<_$AppDatabase, $DryingCyclesTable, DryingCycle>,
      ),
      DryingCycle,
      PrefetchHooks Function()
    >;
typedef $$CycleSamplesTableCreateCompanionBuilder =
    CycleSamplesCompanion Function({
      Value<int> id,
      required String cycleId,
      required int tSec,
      Value<double?> tempC,
      Value<double?> humidityPct,
      Value<double> heaterPct,
      Value<double> fanPct,
    });
typedef $$CycleSamplesTableUpdateCompanionBuilder =
    CycleSamplesCompanion Function({
      Value<int> id,
      Value<String> cycleId,
      Value<int> tSec,
      Value<double?> tempC,
      Value<double?> humidityPct,
      Value<double> heaterPct,
      Value<double> fanPct,
    });

class $$CycleSamplesTableFilterComposer
    extends Composer<_$AppDatabase, $CycleSamplesTable> {
  $$CycleSamplesTableFilterComposer({
    required super.$db,
    required super.$table,
    super.joinBuilder,
    super.$addJoinBuilderToRootComposer,
    super.$removeJoinBuilderFromRootComposer,
  });
  ColumnFilters<int> get id => $composableBuilder(
    column: $table.id,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<String> get cycleId => $composableBuilder(
    column: $table.cycleId,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<int> get tSec => $composableBuilder(
    column: $table.tSec,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<double> get tempC => $composableBuilder(
    column: $table.tempC,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<double> get humidityPct => $composableBuilder(
    column: $table.humidityPct,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<double> get heaterPct => $composableBuilder(
    column: $table.heaterPct,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<double> get fanPct => $composableBuilder(
    column: $table.fanPct,
    builder: (column) => ColumnFilters(column),
  );
}

class $$CycleSamplesTableOrderingComposer
    extends Composer<_$AppDatabase, $CycleSamplesTable> {
  $$CycleSamplesTableOrderingComposer({
    required super.$db,
    required super.$table,
    super.joinBuilder,
    super.$addJoinBuilderToRootComposer,
    super.$removeJoinBuilderFromRootComposer,
  });
  ColumnOrderings<int> get id => $composableBuilder(
    column: $table.id,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<String> get cycleId => $composableBuilder(
    column: $table.cycleId,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<int> get tSec => $composableBuilder(
    column: $table.tSec,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<double> get tempC => $composableBuilder(
    column: $table.tempC,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<double> get humidityPct => $composableBuilder(
    column: $table.humidityPct,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<double> get heaterPct => $composableBuilder(
    column: $table.heaterPct,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<double> get fanPct => $composableBuilder(
    column: $table.fanPct,
    builder: (column) => ColumnOrderings(column),
  );
}

class $$CycleSamplesTableAnnotationComposer
    extends Composer<_$AppDatabase, $CycleSamplesTable> {
  $$CycleSamplesTableAnnotationComposer({
    required super.$db,
    required super.$table,
    super.joinBuilder,
    super.$addJoinBuilderToRootComposer,
    super.$removeJoinBuilderFromRootComposer,
  });
  GeneratedColumn<int> get id =>
      $composableBuilder(column: $table.id, builder: (column) => column);

  GeneratedColumn<String> get cycleId =>
      $composableBuilder(column: $table.cycleId, builder: (column) => column);

  GeneratedColumn<int> get tSec =>
      $composableBuilder(column: $table.tSec, builder: (column) => column);

  GeneratedColumn<double> get tempC =>
      $composableBuilder(column: $table.tempC, builder: (column) => column);

  GeneratedColumn<double> get humidityPct => $composableBuilder(
    column: $table.humidityPct,
    builder: (column) => column,
  );

  GeneratedColumn<double> get heaterPct =>
      $composableBuilder(column: $table.heaterPct, builder: (column) => column);

  GeneratedColumn<double> get fanPct =>
      $composableBuilder(column: $table.fanPct, builder: (column) => column);
}

class $$CycleSamplesTableTableManager
    extends
        RootTableManager<
          _$AppDatabase,
          $CycleSamplesTable,
          CycleSample,
          $$CycleSamplesTableFilterComposer,
          $$CycleSamplesTableOrderingComposer,
          $$CycleSamplesTableAnnotationComposer,
          $$CycleSamplesTableCreateCompanionBuilder,
          $$CycleSamplesTableUpdateCompanionBuilder,
          (
            CycleSample,
            BaseReferences<_$AppDatabase, $CycleSamplesTable, CycleSample>,
          ),
          CycleSample,
          PrefetchHooks Function()
        > {
  $$CycleSamplesTableTableManager(_$AppDatabase db, $CycleSamplesTable table)
    : super(
        TableManagerState(
          db: db,
          table: table,
          createFilteringComposer: () =>
              $$CycleSamplesTableFilterComposer($db: db, $table: table),
          createOrderingComposer: () =>
              $$CycleSamplesTableOrderingComposer($db: db, $table: table),
          createComputedFieldComposer: () =>
              $$CycleSamplesTableAnnotationComposer($db: db, $table: table),
          updateCompanionCallback:
              ({
                Value<int> id = const Value.absent(),
                Value<String> cycleId = const Value.absent(),
                Value<int> tSec = const Value.absent(),
                Value<double?> tempC = const Value.absent(),
                Value<double?> humidityPct = const Value.absent(),
                Value<double> heaterPct = const Value.absent(),
                Value<double> fanPct = const Value.absent(),
              }) => CycleSamplesCompanion(
                id: id,
                cycleId: cycleId,
                tSec: tSec,
                tempC: tempC,
                humidityPct: humidityPct,
                heaterPct: heaterPct,
                fanPct: fanPct,
              ),
          createCompanionCallback:
              ({
                Value<int> id = const Value.absent(),
                required String cycleId,
                required int tSec,
                Value<double?> tempC = const Value.absent(),
                Value<double?> humidityPct = const Value.absent(),
                Value<double> heaterPct = const Value.absent(),
                Value<double> fanPct = const Value.absent(),
              }) => CycleSamplesCompanion.insert(
                id: id,
                cycleId: cycleId,
                tSec: tSec,
                tempC: tempC,
                humidityPct: humidityPct,
                heaterPct: heaterPct,
                fanPct: fanPct,
              ),
          withReferenceMapper: (p0) => p0
              .map((e) => (e.readTable(table), BaseReferences(db, table, e)))
              .toList(),
          prefetchHooksCallback: null,
        ),
      );
}

typedef $$CycleSamplesTableProcessedTableManager =
    ProcessedTableManager<
      _$AppDatabase,
      $CycleSamplesTable,
      CycleSample,
      $$CycleSamplesTableFilterComposer,
      $$CycleSamplesTableOrderingComposer,
      $$CycleSamplesTableAnnotationComposer,
      $$CycleSamplesTableCreateCompanionBuilder,
      $$CycleSamplesTableUpdateCompanionBuilder,
      (
        CycleSample,
        BaseReferences<_$AppDatabase, $CycleSamplesTable, CycleSample>,
      ),
      CycleSample,
      PrefetchHooks Function()
    >;
typedef $$PendingCommandsTableCreateCompanionBuilder =
    PendingCommandsCompanion Function({
      required String id,
      required String knownDeviceId,
      required String topic,
      required String payloadJson,
      required DateTime createdAt,
      required String status,
      Value<String?> lastError,
      Value<int> rowid,
    });
typedef $$PendingCommandsTableUpdateCompanionBuilder =
    PendingCommandsCompanion Function({
      Value<String> id,
      Value<String> knownDeviceId,
      Value<String> topic,
      Value<String> payloadJson,
      Value<DateTime> createdAt,
      Value<String> status,
      Value<String?> lastError,
      Value<int> rowid,
    });

class $$PendingCommandsTableFilterComposer
    extends Composer<_$AppDatabase, $PendingCommandsTable> {
  $$PendingCommandsTableFilterComposer({
    required super.$db,
    required super.$table,
    super.joinBuilder,
    super.$addJoinBuilderToRootComposer,
    super.$removeJoinBuilderFromRootComposer,
  });
  ColumnFilters<String> get id => $composableBuilder(
    column: $table.id,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<String> get knownDeviceId => $composableBuilder(
    column: $table.knownDeviceId,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<String> get topic => $composableBuilder(
    column: $table.topic,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<String> get payloadJson => $composableBuilder(
    column: $table.payloadJson,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<DateTime> get createdAt => $composableBuilder(
    column: $table.createdAt,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<String> get status => $composableBuilder(
    column: $table.status,
    builder: (column) => ColumnFilters(column),
  );

  ColumnFilters<String> get lastError => $composableBuilder(
    column: $table.lastError,
    builder: (column) => ColumnFilters(column),
  );
}

class $$PendingCommandsTableOrderingComposer
    extends Composer<_$AppDatabase, $PendingCommandsTable> {
  $$PendingCommandsTableOrderingComposer({
    required super.$db,
    required super.$table,
    super.joinBuilder,
    super.$addJoinBuilderToRootComposer,
    super.$removeJoinBuilderFromRootComposer,
  });
  ColumnOrderings<String> get id => $composableBuilder(
    column: $table.id,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<String> get knownDeviceId => $composableBuilder(
    column: $table.knownDeviceId,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<String> get topic => $composableBuilder(
    column: $table.topic,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<String> get payloadJson => $composableBuilder(
    column: $table.payloadJson,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<DateTime> get createdAt => $composableBuilder(
    column: $table.createdAt,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<String> get status => $composableBuilder(
    column: $table.status,
    builder: (column) => ColumnOrderings(column),
  );

  ColumnOrderings<String> get lastError => $composableBuilder(
    column: $table.lastError,
    builder: (column) => ColumnOrderings(column),
  );
}

class $$PendingCommandsTableAnnotationComposer
    extends Composer<_$AppDatabase, $PendingCommandsTable> {
  $$PendingCommandsTableAnnotationComposer({
    required super.$db,
    required super.$table,
    super.joinBuilder,
    super.$addJoinBuilderToRootComposer,
    super.$removeJoinBuilderFromRootComposer,
  });
  GeneratedColumn<String> get id =>
      $composableBuilder(column: $table.id, builder: (column) => column);

  GeneratedColumn<String> get knownDeviceId => $composableBuilder(
    column: $table.knownDeviceId,
    builder: (column) => column,
  );

  GeneratedColumn<String> get topic =>
      $composableBuilder(column: $table.topic, builder: (column) => column);

  GeneratedColumn<String> get payloadJson => $composableBuilder(
    column: $table.payloadJson,
    builder: (column) => column,
  );

  GeneratedColumn<DateTime> get createdAt =>
      $composableBuilder(column: $table.createdAt, builder: (column) => column);

  GeneratedColumn<String> get status =>
      $composableBuilder(column: $table.status, builder: (column) => column);

  GeneratedColumn<String> get lastError =>
      $composableBuilder(column: $table.lastError, builder: (column) => column);
}

class $$PendingCommandsTableTableManager
    extends
        RootTableManager<
          _$AppDatabase,
          $PendingCommandsTable,
          PendingCommand,
          $$PendingCommandsTableFilterComposer,
          $$PendingCommandsTableOrderingComposer,
          $$PendingCommandsTableAnnotationComposer,
          $$PendingCommandsTableCreateCompanionBuilder,
          $$PendingCommandsTableUpdateCompanionBuilder,
          (
            PendingCommand,
            BaseReferences<
              _$AppDatabase,
              $PendingCommandsTable,
              PendingCommand
            >,
          ),
          PendingCommand,
          PrefetchHooks Function()
        > {
  $$PendingCommandsTableTableManager(
    _$AppDatabase db,
    $PendingCommandsTable table,
  ) : super(
        TableManagerState(
          db: db,
          table: table,
          createFilteringComposer: () =>
              $$PendingCommandsTableFilterComposer($db: db, $table: table),
          createOrderingComposer: () =>
              $$PendingCommandsTableOrderingComposer($db: db, $table: table),
          createComputedFieldComposer: () =>
              $$PendingCommandsTableAnnotationComposer($db: db, $table: table),
          updateCompanionCallback:
              ({
                Value<String> id = const Value.absent(),
                Value<String> knownDeviceId = const Value.absent(),
                Value<String> topic = const Value.absent(),
                Value<String> payloadJson = const Value.absent(),
                Value<DateTime> createdAt = const Value.absent(),
                Value<String> status = const Value.absent(),
                Value<String?> lastError = const Value.absent(),
                Value<int> rowid = const Value.absent(),
              }) => PendingCommandsCompanion(
                id: id,
                knownDeviceId: knownDeviceId,
                topic: topic,
                payloadJson: payloadJson,
                createdAt: createdAt,
                status: status,
                lastError: lastError,
                rowid: rowid,
              ),
          createCompanionCallback:
              ({
                required String id,
                required String knownDeviceId,
                required String topic,
                required String payloadJson,
                required DateTime createdAt,
                required String status,
                Value<String?> lastError = const Value.absent(),
                Value<int> rowid = const Value.absent(),
              }) => PendingCommandsCompanion.insert(
                id: id,
                knownDeviceId: knownDeviceId,
                topic: topic,
                payloadJson: payloadJson,
                createdAt: createdAt,
                status: status,
                lastError: lastError,
                rowid: rowid,
              ),
          withReferenceMapper: (p0) => p0
              .map((e) => (e.readTable(table), BaseReferences(db, table, e)))
              .toList(),
          prefetchHooksCallback: null,
        ),
      );
}

typedef $$PendingCommandsTableProcessedTableManager =
    ProcessedTableManager<
      _$AppDatabase,
      $PendingCommandsTable,
      PendingCommand,
      $$PendingCommandsTableFilterComposer,
      $$PendingCommandsTableOrderingComposer,
      $$PendingCommandsTableAnnotationComposer,
      $$PendingCommandsTableCreateCompanionBuilder,
      $$PendingCommandsTableUpdateCompanionBuilder,
      (
        PendingCommand,
        BaseReferences<_$AppDatabase, $PendingCommandsTable, PendingCommand>,
      ),
      PendingCommand,
      PrefetchHooks Function()
    >;

class $AppDatabaseManager {
  final _$AppDatabase _db;
  $AppDatabaseManager(this._db);
  $$KnownDevicesTableTableManager get knownDevices =>
      $$KnownDevicesTableTableManager(_db, _db.knownDevices);
  $$DryingCyclesTableTableManager get dryingCycles =>
      $$DryingCyclesTableTableManager(_db, _db.dryingCycles);
  $$CycleSamplesTableTableManager get cycleSamples =>
      $$CycleSamplesTableTableManager(_db, _db.cycleSamples);
  $$PendingCommandsTableTableManager get pendingCommands =>
      $$PendingCommandsTableTableManager(_db, _db.pendingCommands);
}
