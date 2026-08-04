import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:flutter/widgets.dart';
import 'package:flutter_localizations/flutter_localizations.dart';
import 'package:intl/intl.dart' as intl;

import 'app_localizations_en.dart';
import 'app_localizations_pt.dart';

// ignore_for_file: type=lint

/// Callers can lookup localized strings with an instance of AppLocalizations
/// returned by `AppLocalizations.of(context)`.
///
/// Applications need to include `AppLocalizations.delegate()` in their app's
/// `localizationDelegates` list, and the locales they support in the app's
/// `supportedLocales` list. For example:
///
/// ```dart
/// import 'l10n/app_localizations.dart';
///
/// return MaterialApp(
///   localizationsDelegates: AppLocalizations.localizationsDelegates,
///   supportedLocales: AppLocalizations.supportedLocales,
///   home: MyApplicationHome(),
/// );
/// ```
///
/// ## Update pubspec.yaml
///
/// Please make sure to update your pubspec.yaml to include the following
/// packages:
///
/// ```yaml
/// dependencies:
///   # Internationalization support.
///   flutter_localizations:
///     sdk: flutter
///   intl: any # Use the pinned version from flutter_localizations
///
///   # Rest of dependencies
/// ```
///
/// ## iOS Applications
///
/// iOS applications define key application metadata, including supported
/// locales, in an Info.plist file that is built into the application bundle.
/// To configure the locales supported by your app, you’ll need to edit this
/// file.
///
/// First, open your project’s ios/Runner.xcworkspace Xcode workspace file.
/// Then, in the Project Navigator, open the Info.plist file under the Runner
/// project’s Runner folder.
///
/// Next, select the Information Property List item, select Add Item from the
/// Editor menu, then select Localizations from the pop-up menu.
///
/// Select and expand the newly-created Localizations item then, for each
/// locale your application supports, add a new item and select the locale
/// you wish to add from the pop-up menu in the Value field. This list should
/// be consistent with the languages listed in the AppLocalizations.supportedLocales
/// property.
abstract class AppLocalizations {
  AppLocalizations(String locale)
    : localeName = intl.Intl.canonicalizedLocale(locale.toString());

  final String localeName;

  static AppLocalizations? of(BuildContext context) {
    return Localizations.of<AppLocalizations>(context, AppLocalizations);
  }

  static const LocalizationsDelegate<AppLocalizations> delegate =
      _AppLocalizationsDelegate();

  /// A list of this localizations delegate along with the default localizations
  /// delegates.
  ///
  /// Returns a list of localizations delegates containing this delegate along with
  /// GlobalMaterialLocalizations.delegate, GlobalCupertinoLocalizations.delegate,
  /// and GlobalWidgetsLocalizations.delegate.
  ///
  /// Additional delegates can be added by appending to this list in
  /// MaterialApp. This list does not have to be used at all if a custom list
  /// of delegates is preferred or required.
  static const List<LocalizationsDelegate<dynamic>> localizationsDelegates =
      <LocalizationsDelegate<dynamic>>[
        delegate,
        GlobalMaterialLocalizations.delegate,
        GlobalCupertinoLocalizations.delegate,
        GlobalWidgetsLocalizations.delegate,
      ];

  /// A list of this localizations delegate's supported locales.
  static const List<Locale> supportedLocales = <Locale>[
    Locale('en'),
    Locale('pt'),
  ];

  /// No description provided for @appTitle.
  ///
  /// In en, this message translates to:
  /// **'Philarmony Installer'**
  String get appTitle;

  /// No description provided for @next.
  ///
  /// In en, this message translates to:
  /// **'Next'**
  String get next;

  /// No description provided for @back.
  ///
  /// In en, this message translates to:
  /// **'Back'**
  String get back;

  /// No description provided for @install.
  ///
  /// In en, this message translates to:
  /// **'Install'**
  String get install;

  /// No description provided for @retry.
  ///
  /// In en, this message translates to:
  /// **'Retry'**
  String get retry;

  /// No description provided for @exportProfile.
  ///
  /// In en, this message translates to:
  /// **'Export profile'**
  String get exportProfile;

  /// No description provided for @importProfile.
  ///
  /// In en, this message translates to:
  /// **'Import profile'**
  String get importProfile;

  /// No description provided for @loadLastSession.
  ///
  /// In en, this message translates to:
  /// **'Load last session'**
  String get loadLastSession;

  /// No description provided for @newSetup.
  ///
  /// In en, this message translates to:
  /// **'New setup'**
  String get newSetup;

  /// No description provided for @flashSuccess.
  ///
  /// In en, this message translates to:
  /// **'Flash Success'**
  String get flashSuccess;

  /// No description provided for @networkWarn.
  ///
  /// In en, this message translates to:
  /// **'Device not reachable on network (optional check). Flash still succeeded.'**
  String get networkWarn;

  /// No description provided for @pinConflict.
  ///
  /// In en, this message translates to:
  /// **'Fix pin conflicts before continuing'**
  String get pinConflict;

  /// No description provided for @wifiPasswordRequired.
  ///
  /// In en, this message translates to:
  /// **'Re-enter WiFi password before flashing'**
  String get wifiPasswordRequired;

  /// No description provided for @noPorts.
  ///
  /// In en, this message translates to:
  /// **'No USB serial ports found. Install CH340/CP210x/FTDI drivers and reconnect.'**
  String get noPorts;

  /// No description provided for @language.
  ///
  /// In en, this message translates to:
  /// **'Language'**
  String get language;

  /// No description provided for @languagePortuguese.
  ///
  /// In en, this message translates to:
  /// **'Português'**
  String get languagePortuguese;

  /// No description provided for @languageEnglish.
  ///
  /// In en, this message translates to:
  /// **'English'**
  String get languageEnglish;

  /// No description provided for @requiredField.
  ///
  /// In en, this message translates to:
  /// **'Required'**
  String get requiredField;

  /// No description provided for @optionalField.
  ///
  /// In en, this message translates to:
  /// **'Optional'**
  String get optionalField;

  /// No description provided for @optionalAdvanced.
  ///
  /// In en, this message translates to:
  /// **'Default 921600 (configurable)'**
  String get optionalAdvanced;

  /// No description provided for @deviceModelLabel.
  ///
  /// In en, this message translates to:
  /// **'ESP32 model'**
  String get deviceModelLabel;

  /// No description provided for @flashSizeLabel.
  ///
  /// In en, this message translates to:
  /// **'Flash size (MB)'**
  String get flashSizeLabel;

  /// No description provided for @baudLabel.
  ///
  /// In en, this message translates to:
  /// **'Baud rate'**
  String get baudLabel;

  /// No description provided for @usbPortLabel.
  ///
  /// In en, this message translates to:
  /// **'USB port'**
  String get usbPortLabel;

  /// No description provided for @serialPortLabel.
  ///
  /// In en, this message translates to:
  /// **'Serial port'**
  String get serialPortLabel;

  /// No description provided for @customPartitionLabel.
  ///
  /// In en, this message translates to:
  /// **'Custom partition table'**
  String get customPartitionLabel;

  /// No description provided for @clearLabel.
  ///
  /// In en, this message translates to:
  /// **'Clear'**
  String get clearLabel;

  /// No description provided for @selectPartitionsTitle.
  ///
  /// In en, this message translates to:
  /// **'Select custom partitions.bin'**
  String get selectPartitionsTitle;

  /// No description provided for @unsupportedChip.
  ///
  /// In en, this message translates to:
  /// **'Unsupported chip {chip}. Supported variants: {supported}. Select a supported model manually.'**
  String unsupportedChip(String chip, String supported);

  /// No description provided for @chipDetectFailed.
  ///
  /// In en, this message translates to:
  /// **'Could not auto-detect chip (select model manually)'**
  String get chipDetectFailed;

  /// No description provided for @partialFlashWarn.
  ///
  /// In en, this message translates to:
  /// **'USB may have disconnected mid-flash. The device may be in a partial state — reconnect and Retry a full erase→write→verify.'**
  String get partialFlashWarn;

  /// No description provided for @retryHelp.
  ///
  /// In en, this message translates to:
  /// **'Retry re-runs a full erase→write→verify of the current package. No previous firmware image is restored automatically.'**
  String get retryHelp;

  /// No description provided for @tempSensorLabel.
  ///
  /// In en, this message translates to:
  /// **'Temperature sensor'**
  String get tempSensorLabel;

  /// No description provided for @humiditySensorLabel.
  ///
  /// In en, this message translates to:
  /// **'Humidity sensor'**
  String get humiditySensorLabel;

  /// No description provided for @tempGpioLabel.
  ///
  /// In en, this message translates to:
  /// **'Temp GPIO'**
  String get tempGpioLabel;

  /// No description provided for @humidityGpioLabel.
  ///
  /// In en, this message translates to:
  /// **'Humidity GPIO'**
  String get humidityGpioLabel;

  /// No description provided for @advancedDriverTitle.
  ///
  /// In en, this message translates to:
  /// **'Advanced: custom JSON driver'**
  String get advancedDriverTitle;

  /// No description provided for @advancedDriverSubtitle.
  ///
  /// In en, this message translates to:
  /// **'Optional — merges into sensor parameters'**
  String get advancedDriverSubtitle;

  /// No description provided for @driverTargetLabel.
  ///
  /// In en, this message translates to:
  /// **'Apply JSON to'**
  String get driverTargetLabel;

  /// No description provided for @driverJsonLabel.
  ///
  /// In en, this message translates to:
  /// **'Driver JSON'**
  String get driverJsonLabel;

  /// No description provided for @pinoutTitle.
  ///
  /// In en, this message translates to:
  /// **'Pinout'**
  String get pinoutTitle;

  /// No description provided for @heaterPwmLabel.
  ///
  /// In en, this message translates to:
  /// **'Heater PWM'**
  String get heaterPwmLabel;

  /// No description provided for @exhaustFanPwmLabel.
  ///
  /// In en, this message translates to:
  /// **'Exhaust fan PWM'**
  String get exhaustFanPwmLabel;

  /// No description provided for @exhaustFanDigitalLabel.
  ///
  /// In en, this message translates to:
  /// **'Exhaust fan digital'**
  String get exhaustFanDigitalLabel;

  /// No description provided for @pinLegendStrapping.
  ///
  /// In en, this message translates to:
  /// **'Strapping'**
  String get pinLegendStrapping;

  /// No description provided for @pinLegendFlash.
  ///
  /// In en, this message translates to:
  /// **'Flash reserved'**
  String get pinLegendFlash;

  /// No description provided for @pinLegendPsram.
  ///
  /// In en, this message translates to:
  /// **'PSRAM reserved'**
  String get pinLegendPsram;

  /// No description provided for @pinLegendInputOnly.
  ///
  /// In en, this message translates to:
  /// **'Input-only'**
  String get pinLegendInputOnly;

  /// No description provided for @pinLegendAssigned.
  ///
  /// In en, this message translates to:
  /// **'Assigned'**
  String get pinLegendAssigned;

  /// No description provided for @pinLegendFlashPsram.
  ///
  /// In en, this message translates to:
  /// **'Flash/PSRAM'**
  String get pinLegendFlashPsram;

  /// No description provided for @pinoutPackageOutline.
  ///
  /// In en, this message translates to:
  /// **'{model} package outline (GPIO map)'**
  String pinoutPackageOutline(String model);

  /// No description provided for @enableDisplayLabel.
  ///
  /// In en, this message translates to:
  /// **'Enable display'**
  String get enableDisplayLabel;

  /// No description provided for @displayDriverLabel.
  ///
  /// In en, this message translates to:
  /// **'Driver'**
  String get displayDriverLabel;

  /// No description provided for @resolutionPresetLabel.
  ///
  /// In en, this message translates to:
  /// **'Resolution preset'**
  String get resolutionPresetLabel;

  /// No description provided for @customLabel.
  ///
  /// In en, this message translates to:
  /// **'Custom'**
  String get customLabel;

  /// No description provided for @widthLabel.
  ///
  /// In en, this message translates to:
  /// **'Width'**
  String get widthLabel;

  /// No description provided for @heightLabel.
  ///
  /// In en, this message translates to:
  /// **'Height'**
  String get heightLabel;

  /// No description provided for @statusFieldsLabel.
  ///
  /// In en, this message translates to:
  /// **'Status fields'**
  String get statusFieldsLabel;

  /// No description provided for @layoutPreviewLabel.
  ///
  /// In en, this message translates to:
  /// **'Layout preview'**
  String get layoutPreviewLabel;

  /// No description provided for @noFieldsLabel.
  ///
  /// In en, this message translates to:
  /// **'(no fields)'**
  String get noFieldsLabel;

  /// No description provided for @spiDcLabel.
  ///
  /// In en, this message translates to:
  /// **'SPI DC pin'**
  String get spiDcLabel;

  /// No description provided for @spiResetLabel.
  ///
  /// In en, this message translates to:
  /// **'SPI RESET pin'**
  String get spiResetLabel;

  /// No description provided for @addCustomProfile.
  ///
  /// In en, this message translates to:
  /// **'Add custom profile'**
  String get addCustomProfile;

  /// No description provided for @editCustomProfile.
  ///
  /// In en, this message translates to:
  /// **'Edit custom profile'**
  String get editCustomProfile;

  /// No description provided for @overrideBuiltinTitle.
  ///
  /// In en, this message translates to:
  /// **'Override builtin'**
  String get overrideBuiltinTitle;

  /// No description provided for @namePtLabel.
  ///
  /// In en, this message translates to:
  /// **'Name PT'**
  String get namePtLabel;

  /// No description provided for @nameEnLabel.
  ///
  /// In en, this message translates to:
  /// **'Name EN'**
  String get nameEnLabel;

  /// No description provided for @tempCLabel.
  ///
  /// In en, this message translates to:
  /// **'Temp °C'**
  String get tempCLabel;

  /// No description provided for @durationMinLabel.
  ///
  /// In en, this message translates to:
  /// **'Duration min'**
  String get durationMinLabel;

  /// No description provided for @humidityPctLabel.
  ///
  /// In en, this message translates to:
  /// **'Humidity %'**
  String get humidityPctLabel;

  /// No description provided for @cancelLabel.
  ///
  /// In en, this message translates to:
  /// **'Cancel'**
  String get cancelLabel;

  /// No description provided for @saveLabel.
  ///
  /// In en, this message translates to:
  /// **'Save'**
  String get saveLabel;

  /// No description provided for @ssidLabel.
  ///
  /// In en, this message translates to:
  /// **'SSID'**
  String get ssidLabel;

  /// No description provided for @passwordLabel.
  ///
  /// In en, this message translates to:
  /// **'Password'**
  String get passwordLabel;

  /// No description provided for @staticIpTitle.
  ///
  /// In en, this message translates to:
  /// **'Static IP (optional)'**
  String get staticIpTitle;

  /// No description provided for @wifiScanLabel.
  ///
  /// In en, this message translates to:
  /// **'Scan nearby WiFi'**
  String get wifiScanLabel;

  /// No description provided for @wifiScanUnavailable.
  ///
  /// In en, this message translates to:
  /// **'No networks found (scan unsupported or permission denied). Enter SSID manually.'**
  String get wifiScanUnavailable;

  /// No description provided for @dismissLabel.
  ///
  /// In en, this message translates to:
  /// **'Dismiss'**
  String get dismissLabel;

  /// No description provided for @stepLabel.
  ///
  /// In en, this message translates to:
  /// **'Step'**
  String get stepLabel;

  /// No description provided for @reviewModel.
  ///
  /// In en, this message translates to:
  /// **'Model: {model} ({flashMb} MB)'**
  String reviewModel(String model, int flashMb);

  /// No description provided for @reviewHeaterPwm.
  ///
  /// In en, this message translates to:
  /// **'Heater PWM: {pin}'**
  String reviewHeaterPwm(int pin);

  /// No description provided for @reviewWifiSsid.
  ///
  /// In en, this message translates to:
  /// **'WiFi SSID: {ssid}'**
  String reviewWifiSsid(String ssid);

  /// No description provided for @reviewProfiles.
  ///
  /// In en, this message translates to:
  /// **'Profiles: {count}'**
  String reviewProfiles(int count);

  /// No description provided for @reviewFirmware.
  ///
  /// In en, this message translates to:
  /// **'Firmware: {version}'**
  String reviewFirmware(String version);

  /// No description provided for @reviewEraseWarning.
  ///
  /// In en, this message translates to:
  /// **'Install will erase and reflash the device with this configuration. There is no automatic restore of a previous firmware image.'**
  String get reviewEraseWarning;

  /// No description provided for @flashLogVerbose.
  ///
  /// In en, this message translates to:
  /// **'Verbose'**
  String get flashLogVerbose;

  /// No description provided for @flashLogQuiet.
  ///
  /// In en, this message translates to:
  /// **'Quiet'**
  String get flashLogQuiet;

  /// No description provided for @flashLogExport.
  ///
  /// In en, this message translates to:
  /// **'Export log'**
  String get flashLogExport;

  /// No description provided for @flashLogExportTitle.
  ///
  /// In en, this message translates to:
  /// **'Export flash log'**
  String get flashLogExportTitle;

  /// No description provided for @exportProfileTitle.
  ///
  /// In en, this message translates to:
  /// **'Export Philarmony profile'**
  String get exportProfileTitle;

  /// No description provided for @exportSavedSnack.
  ///
  /// In en, this message translates to:
  /// **'Saved {path} (password redacted)'**
  String exportSavedSnack(String path);

  /// No description provided for @exportClipboardSnack.
  ///
  /// In en, this message translates to:
  /// **'Save cancelled — JSON copied (password redacted)'**
  String get exportClipboardSnack;

  /// No description provided for @importFailed.
  ///
  /// In en, this message translates to:
  /// **'Import failed: {error}'**
  String importFailed(String error);
}

class _AppLocalizationsDelegate
    extends LocalizationsDelegate<AppLocalizations> {
  const _AppLocalizationsDelegate();

  @override
  Future<AppLocalizations> load(Locale locale) {
    return SynchronousFuture<AppLocalizations>(lookupAppLocalizations(locale));
  }

  @override
  bool isSupported(Locale locale) =>
      <String>['en', 'pt'].contains(locale.languageCode);

  @override
  bool shouldReload(_AppLocalizationsDelegate old) => false;
}

AppLocalizations lookupAppLocalizations(Locale locale) {
  // Lookup logic when only language code is specified.
  switch (locale.languageCode) {
    case 'en':
      return AppLocalizationsEn();
    case 'pt':
      return AppLocalizationsPt();
  }

  throw FlutterError(
    'AppLocalizations.delegate failed to load unsupported locale "$locale". This is likely '
    'an issue with the localizations generation tool. Please file an issue '
    'on GitHub with a reproducible sample app and the gen-l10n configuration '
    'that was used.',
  );
}
