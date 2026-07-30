// ignore: unused_import
import 'package:intl/intl.dart' as intl;
import 'app_localizations.dart';

// ignore_for_file: type=lint

/// The translations for English (`en`).
class AppLocalizationsEn extends AppLocalizations {
  AppLocalizationsEn([String locale = 'en']) : super(locale);

  @override
  String get appTitle => 'Philarmony Installer';

  @override
  String get next => 'Next';

  @override
  String get back => 'Back';

  @override
  String get install => 'Install';

  @override
  String get retry => 'Retry';

  @override
  String get exportProfile => 'Export profile';

  @override
  String get importProfile => 'Import profile';

  @override
  String get loadLastSession => 'Load last session';

  @override
  String get newSetup => 'New setup';

  @override
  String get flashSuccess => 'Flash Success';

  @override
  String get networkWarn =>
      'Device not reachable on network (optional check). Flash still succeeded.';

  @override
  String get pinConflict => 'Fix pin conflicts before continuing';

  @override
  String get wifiPasswordRequired => 'Re-enter WiFi password before flashing';

  @override
  String get noPorts =>
      'No USB serial ports found. Install CH340/CP210x/FTDI drivers and reconnect.';

  @override
  String get language => 'Language';
}
