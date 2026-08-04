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

  @override
  String get languagePortuguese => 'Português';

  @override
  String get languageEnglish => 'English';

  @override
  String get requiredField => 'Required';

  @override
  String get optionalField => 'Optional';

  @override
  String get optionalAdvanced => 'Default 921600 (configurable)';

  @override
  String get deviceModelLabel => 'ESP32 model';

  @override
  String get flashSizeLabel => 'Flash size (MB)';

  @override
  String get baudLabel => 'Baud rate';

  @override
  String get usbPortLabel => 'USB port';

  @override
  String get serialPortLabel => 'Serial port';

  @override
  String get customPartitionLabel => 'Custom partition table';

  @override
  String get clearLabel => 'Clear';

  @override
  String get selectPartitionsTitle => 'Select custom partitions.bin';

  @override
  String unsupportedChip(String chip, String supported) {
    return 'Unsupported chip $chip. Supported variants: $supported. Select a supported model manually.';
  }

  @override
  String get chipDetectFailed =>
      'Could not auto-detect chip (select model manually)';

  @override
  String get partialFlashWarn =>
      'USB may have disconnected mid-flash. The device may be in a partial state — reconnect and Retry a full erase→write→verify.';

  @override
  String get retryHelp =>
      'Retry re-runs a full erase→write→verify of the current package. No previous firmware image is restored automatically.';

  @override
  String get tempSensorLabel => 'Temperature sensor';

  @override
  String get humiditySensorLabel => 'Humidity sensor';

  @override
  String get tempGpioLabel => 'Temp GPIO';

  @override
  String get humidityGpioLabel => 'Humidity GPIO';

  @override
  String get advancedDriverTitle => 'Advanced: custom JSON driver';

  @override
  String get advancedDriverSubtitle =>
      'Optional — merges into sensor parameters';

  @override
  String get driverTargetLabel => 'Apply JSON to';

  @override
  String get driverJsonLabel => 'Driver JSON';

  @override
  String get pinoutTitle => 'Pinout';

  @override
  String get heaterPwmLabel => 'Heater PWM';

  @override
  String get exhaustFanPwmLabel => 'Exhaust fan PWM';

  @override
  String get exhaustFanDigitalLabel => 'Exhaust fan digital';

  @override
  String get pinLegendStrapping => 'Strapping';

  @override
  String get pinLegendFlash => 'Flash reserved';

  @override
  String get pinLegendPsram => 'PSRAM reserved';

  @override
  String get pinLegendInputOnly => 'Input-only';

  @override
  String get pinLegendAssigned => 'Assigned';

  @override
  String get pinLegendFlashPsram => 'Flash/PSRAM';

  @override
  String pinoutPackageOutline(String model) {
    return '$model package outline (GPIO map)';
  }

  @override
  String get enableDisplayLabel => 'Enable display';

  @override
  String get displayDriverLabel => 'Driver';

  @override
  String get resolutionPresetLabel => 'Resolution preset';

  @override
  String get customLabel => 'Custom';

  @override
  String get widthLabel => 'Width';

  @override
  String get heightLabel => 'Height';

  @override
  String get statusFieldsLabel => 'Status fields';

  @override
  String get layoutPreviewLabel => 'Layout preview';

  @override
  String get noFieldsLabel => '(no fields)';

  @override
  String get spiDcLabel => 'SPI DC pin';

  @override
  String get spiResetLabel => 'SPI RESET pin';

  @override
  String get addCustomProfile => 'Add custom profile';

  @override
  String get editCustomProfile => 'Edit custom profile';

  @override
  String get overrideBuiltinTitle => 'Override builtin';

  @override
  String get namePtLabel => 'Name PT';

  @override
  String get nameEnLabel => 'Name EN';

  @override
  String get tempCLabel => 'Temp °C';

  @override
  String get durationMinLabel => 'Duration min';

  @override
  String get humidityPctLabel => 'Humidity %';

  @override
  String get cancelLabel => 'Cancel';

  @override
  String get saveLabel => 'Save';

  @override
  String get ssidLabel => 'SSID';

  @override
  String get passwordLabel => 'Password';

  @override
  String get staticIpTitle => 'Static IP (optional)';

  @override
  String get wifiScanLabel => 'Scan nearby WiFi';

  @override
  String get wifiScanUnavailable =>
      'No networks found (scan unsupported or permission denied). Enter SSID manually.';

  @override
  String get dismissLabel => 'Dismiss';

  @override
  String get stepLabel => 'Step';

  @override
  String reviewModel(String model, int flashMb) {
    return 'Model: $model ($flashMb MB)';
  }

  @override
  String reviewHeaterPwm(int pin) {
    return 'Heater PWM: $pin';
  }

  @override
  String reviewWifiSsid(String ssid) {
    return 'WiFi SSID: $ssid';
  }

  @override
  String reviewProfiles(int count) {
    return 'Profiles: $count';
  }

  @override
  String reviewFirmware(String version) {
    return 'Firmware: $version';
  }

  @override
  String get reviewEraseWarning =>
      'Install will erase and reflash the device with this configuration. There is no automatic restore of a previous firmware image.';

  @override
  String get flashLogVerbose => 'Verbose';

  @override
  String get flashLogQuiet => 'Quiet';

  @override
  String get flashLogExport => 'Export log';

  @override
  String get flashLogExportTitle => 'Export flash log';

  @override
  String get exportProfileTitle => 'Export Philarmony profile';

  @override
  String exportSavedSnack(String path) {
    return 'Saved $path (password redacted)';
  }

  @override
  String get exportClipboardSnack =>
      'Save cancelled — JSON copied (password redacted)';

  @override
  String importFailed(String error) {
    return 'Import failed: $error';
  }
}
