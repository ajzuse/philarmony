/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/// Runtime metadata parsed from WS status/config envelopes (FR-004).
class DeviceRuntimeInfo {
  const DeviceRuntimeInfo({
    this.wifiSsid,
    this.wifiSignalDbm,
    this.apMode,
    this.firmwareVersion,
    this.deviceName,
    this.ntpTimezone,
    this.deviceModel,
  });

  final String? wifiSsid;
  final int? wifiSignalDbm;
  final bool? apMode;
  final String? firmwareVersion;
  final String? deviceName;
  final String? ntpTimezone;
  final String? deviceModel;

  DeviceRuntimeInfo merge(DeviceRuntimeInfo? other) {
    if (other == null) return this;
    return DeviceRuntimeInfo(
      wifiSsid: other.wifiSsid ?? wifiSsid,
      wifiSignalDbm: other.wifiSignalDbm ?? wifiSignalDbm,
      apMode: other.apMode ?? apMode,
      firmwareVersion: other.firmwareVersion ?? firmwareVersion,
      deviceName: other.deviceName ?? deviceName,
      ntpTimezone: other.ntpTimezone ?? ntpTimezone,
      deviceModel: other.deviceModel ?? deviceModel,
    );
  }

  static DeviceRuntimeInfo? fromPayload(Map<String, dynamic> payload) {
    final wifi = payload['wifi'] as Map<String, dynamic>?;
    final network = payload['network'] as Map<String, dynamic>?;
    final ssid = payload['wifi_ssid'] as String? ??
        wifi?['ssid'] as String? ??
        network?['ssid'] as String?;
    final rssi = payload['wifi_rssi'] as num? ??
        payload['wifi_signal_dbm'] as num? ??
        wifi?['rssi'] as num? ??
        wifi?['signal_dbm'] as num? ??
        network?['rssi'] as num?;
    final apMode = payload['ap_mode'] as bool? ??
        payload['hotspot_mode'] as bool? ??
        wifi?['ap_mode'] as bool? ??
        network?['ap_mode'] as bool?;
    final firmware = payload['firmware_version'] as String?;
    final name = payload['device_name'] as String? ?? payload['hostname'] as String?;
    final ntp = payload['ntp_timezone'] as String? ?? payload['timezone'] as String?;
    final model = payload['device_model'] as String? ?? payload['chip_model'] as String?;

    if (ssid == null &&
        rssi == null &&
        apMode == null &&
        firmware == null &&
        name == null &&
        ntp == null &&
        model == null) {
      return null;
    }

    return DeviceRuntimeInfo(
      wifiSsid: ssid,
      wifiSignalDbm: rssi?.toInt(),
      apMode: apMode,
      firmwareVersion: firmware,
      deviceName: name,
      ntpTimezone: ntp,
      deviceModel: model,
    );
  }
}

/// Resolved WiFi presentation for the config screen.
class WifiPresentation {
  const WifiPresentation({
    required this.hotspotMode,
    required this.ssid,
    this.signalDbm,
  });

  final bool hotspotMode;
  final String ssid;
  final int? signalDbm;

  String get signalLabel {
    if (signalDbm == null) return '—';
    return '$signalDbm dBm';
  }
}

WifiPresentation resolveWifiPresentation({
  required String? host,
  DeviceRuntimeInfo? runtime,
}) {
  final hotspot = host == '192.168.4.1' || runtime?.apMode == true;
  final ssid = hotspot
      ? 'philarmony'
      : (runtime?.wifiSsid ?? host ?? '—');
  return WifiPresentation(
    hotspotMode: hotspot,
    ssid: ssid,
    signalDbm: runtime?.wifiSignalDbm,
  );
}
