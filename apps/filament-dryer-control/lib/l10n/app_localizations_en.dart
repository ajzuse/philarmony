// ignore: unused_import
import 'package:intl/intl.dart' as intl;
import 'app_localizations.dart';

// ignore_for_file: type=lint

/// The translations for English (`en`).
class AppLocalizationsEn extends AppLocalizations {
  AppLocalizationsEn([String locale = 'en']) : super(locale);

  @override
  String get appTitle => 'Philarmony Control';

  @override
  String get navDashboard => 'Dashboard';

  @override
  String get navHistory => 'History';

  @override
  String get navDevices => 'Devices';

  @override
  String get navSettings => 'Settings';

  @override
  String get scan => 'Scan';

  @override
  String get connect => 'Connect';

  @override
  String get connected => 'Connected';

  @override
  String get connecting => 'Connecting';

  @override
  String get reconnecting => 'Reconnecting';

  @override
  String get disconnected => 'Disconnected';

  @override
  String get manualHost => 'Host';

  @override
  String get manualPort => 'Port';

  @override
  String get startCycle => 'Start drying';

  @override
  String get stopCycle => 'Stop';

  @override
  String get stopConfirmTitle => 'Stop drying?';

  @override
  String get stopConfirmBody => 'Heater and fan will turn off immediately.';

  @override
  String get cancel => 'Cancel';

  @override
  String get confirm => 'Confirm';

  @override
  String get exportCsv => 'Export CSV';

  @override
  String get exportPdf => 'Export PDF';

  @override
  String get noDevices => 'No devices found. Try manual connect.';

  @override
  String get language => 'Language';

  @override
  String get switchDevice => 'Switch device';

  @override
  String get deviceSettings => 'Device settings';

  @override
  String get savedDevices => 'Saved devices';

  @override
  String get deviceNotFound => 'Device not found';

  @override
  String get notificationSettings => 'Notifications';

  @override
  String get notifyCycleComplete => 'Cycle complete';

  @override
  String get notifyCycleError => 'Cycle error';

  @override
  String get notifySafetyFault => 'Safety fault';

  @override
  String get notifyConnectionLost => 'Connection lost';

  @override
  String sessionCount(int active, int max) {
    return '$active of $max sessions';
  }

  @override
  String get materialProfiles => 'Material profiles';

  @override
  String get materialProfilesSubtitle =>
      'Built-in presets and custom materials';

  @override
  String get addCustomProfile => 'Add custom profile';

  @override
  String get editCustomProfile => 'Edit custom profile';

  @override
  String get builtinProfile => 'Built-in';

  @override
  String get noProfiles => 'No profiles available';

  @override
  String get maxCustomProfiles => 'Maximum custom profiles reached';

  @override
  String get deleteProfileTitle => 'Delete profile?';

  @override
  String deleteProfileBody(String name) {
    return 'Delete $name?';
  }

  @override
  String get selectProfile => 'Material profile';

  @override
  String get manualParameters => 'Manual parameters';

  @override
  String get namePtLabel => 'Name (PT)';

  @override
  String get nameEnLabel => 'Name (EN)';

  @override
  String get tempCLabel => 'Temp °C';

  @override
  String get durationMinLabel => 'Duration (min)';

  @override
  String get humidityPctLabel => 'Humidity %';

  @override
  String get saveLabel => 'Save';

  @override
  String get deviceConfig => 'Device configuration';

  @override
  String get wifiSettings => 'Wi‑Fi';

  @override
  String get advancedSettings => 'Advanced (read-only)';

  @override
  String get backgroundNotAllowed =>
      'Background monitoring unavailable. Keep the app open during drying.';

  @override
  String get dismiss => 'Dismiss';

  @override
  String get sensorError => 'Sensor Error';

  @override
  String get statusLabel => 'Status';

  @override
  String get chamberTemp => 'Chamber temp';

  @override
  String get targetTemp => 'Target temp';

  @override
  String get humidity => 'Humidity';

  @override
  String get targetHumidity => 'Target humidity';

  @override
  String get heater => 'Heater';

  @override
  String get exhaustFan => 'Exhaust fan';

  @override
  String get elapsedTime => 'Elapsed';

  @override
  String get remainingTime => 'Remaining';

  @override
  String get cpuUsage => 'CPU usage';

  @override
  String get memoryFree => 'Free memory';

  @override
  String get uptime => 'Uptime';

  @override
  String get heatingBadge => 'Heating';

  @override
  String get coolingBadge => 'Cooling';

  @override
  String get idleBadge => 'Idle';

  @override
  String get startWithProfile => 'Start with profile';

  @override
  String get useCelsius => 'Use Celsius';

  @override
  String get useCelsiusSubtitle => 'Off shows Fahrenheit on dashboard';

  @override
  String get timeFormat => 'Time format';

  @override
  String get timeFormat24h => '24-hour';

  @override
  String get timeFormat12h => '12-hour';

  @override
  String get chartWindow => 'Chart window';

  @override
  String get chartWindowSubtitle =>
      'Telemetry history shown on dashboard charts';

  @override
  String get maxConcurrentSessions => 'Max concurrent sessions';

  @override
  String get pinLock => 'PIN lock';

  @override
  String get pinLockSubtitle => 'Optional — stub for MVP';

  @override
  String get deviceConfigSubtitle => 'Sensors, pins, display, WiFi';

  @override
  String get testNotification => 'Test notification';

  @override
  String get testNotificationSubtitle => 'Local notifications adapter stub';

  @override
  String get systemDefault => 'System default';

  @override
  String get languageEnUs => 'English (US)';

  @override
  String get languagePtBr => 'Portuguese (Brazil)';

  @override
  String get highContrast => 'High contrast';

  @override
  String get highContrastSubtitle => 'Stronger colors for readability';

  @override
  String get configReloadTooltip => 'Reload from device';

  @override
  String get configSavedBanner => 'Configuration saved on device.';

  @override
  String get configSaveButton => 'Save hardware configuration';

  @override
  String get configSensorsTitle => 'Sensors';

  @override
  String get configTypeLabel => 'Type';

  @override
  String get configGpioLabel => 'GPIO';

  @override
  String get configActuatorsTitle => 'Pins / actuators';

  @override
  String get configPwmGpioLabel => 'PWM GPIO';

  @override
  String get configDisplayTitle => 'Display';

  @override
  String get configDisplayEnabled => 'Enabled';

  @override
  String get configDriverLabel => 'Driver';

  @override
  String get configWidthLabel => 'Width';

  @override
  String get configHeightLabel => 'Height';

  @override
  String get wifiSsidLabel => 'SSID';

  @override
  String get notConnected => 'Not connected';

  @override
  String get wifiSignalLabel => 'Signal';

  @override
  String get notReportedMvp => 'Not reported by device (MVP)';

  @override
  String get wifiReconfigureButton => 'Reconfigure WiFi (hotspot mode)';

  @override
  String get wifiReconfigureNote =>
      'WiFi reconfiguration triggers hotspot mode on the device. Use the desktop installer for full WiFi setup in v1.';

  @override
  String get advancedDeviceName => 'Device name';

  @override
  String get advancedFirmwareVersion => 'Firmware version';

  @override
  String get advancedNtpTimezone => 'NTP timezone';

  @override
  String get advancedHardTempLimit => 'Hard temp limit (°C)';

  @override
  String get advancedUptimeSec => 'Uptime (sec)';

  @override
  String get advancedCpuUsage => 'CPU usage (%)';

  @override
  String get advancedFreeMemory => 'Free memory (bytes)';

  @override
  String get historyFilterAll => 'All';

  @override
  String get historyFilterCompleted => 'Completed';

  @override
  String get historyFilterInProgress => 'In progress';

  @override
  String get historyCycleDefault => 'Cycle';

  @override
  String get historyEmpty => 'No cycles yet';

  @override
  String get selectDevice => 'Select a device';

  @override
  String chartWindowMinutes(int minutes) {
    return '$minutes min';
  }

  @override
  String wifiHotspotModeBody(String ssid) {
    return 'Device is in hotspot mode. Join WiFi network \"$ssid\" to configure.';
  }

  @override
  String get wifiReconfigureTitle => 'Reconfigure WiFi';

  @override
  String wifiReconfigureSteps(String ssid, String host) {
    return '1. Connect your phone or PC to WiFi \"$ssid\".\n2. Open http://$host/ in a browser.\n3. Enter your home WiFi credentials.\n4. Reconnect the control app on the new IP.';
  }

  @override
  String pendingCommandsTooltip(int count) {
    String _temp0 = intl.Intl.pluralLogic(
      count,
      locale: localeName,
      other: '$count pending changes',
      one: '1 pending change',
    );
    return '$_temp0';
  }

  @override
  String get resetProfilesDefaults => 'Reset defaults';

  @override
  String get resetProfilesDefaultsTitle => 'Reset profile defaults?';

  @override
  String get resetProfilesDefaultsBody =>
      'Restore built-in material profiles from the device. Custom profiles are removed.';

  @override
  String get maxConcurrentSessionsSubtitle =>
      'Maximum simultaneous device connections';

  @override
  String get autoConnectLabel => 'Auto-connect';

  @override
  String get autoConnectSubtitle => 'Prefer this device on app launch';

  @override
  String get lastSeenLabel => 'Last seen';

  @override
  String get firmwareVersionLabel => 'Firmware version';

  @override
  String get notReported => 'Not reported';
}
