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
}
