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
  /// **'Philarmony Control'**
  String get appTitle;

  /// No description provided for @navDashboard.
  ///
  /// In en, this message translates to:
  /// **'Dashboard'**
  String get navDashboard;

  /// No description provided for @navHistory.
  ///
  /// In en, this message translates to:
  /// **'History'**
  String get navHistory;

  /// No description provided for @navDevices.
  ///
  /// In en, this message translates to:
  /// **'Devices'**
  String get navDevices;

  /// No description provided for @navSettings.
  ///
  /// In en, this message translates to:
  /// **'Settings'**
  String get navSettings;

  /// No description provided for @scan.
  ///
  /// In en, this message translates to:
  /// **'Scan'**
  String get scan;

  /// No description provided for @connect.
  ///
  /// In en, this message translates to:
  /// **'Connect'**
  String get connect;

  /// No description provided for @connected.
  ///
  /// In en, this message translates to:
  /// **'Connected'**
  String get connected;

  /// No description provided for @connecting.
  ///
  /// In en, this message translates to:
  /// **'Connecting'**
  String get connecting;

  /// No description provided for @reconnecting.
  ///
  /// In en, this message translates to:
  /// **'Reconnecting'**
  String get reconnecting;

  /// No description provided for @disconnected.
  ///
  /// In en, this message translates to:
  /// **'Disconnected'**
  String get disconnected;

  /// No description provided for @manualHost.
  ///
  /// In en, this message translates to:
  /// **'Host'**
  String get manualHost;

  /// No description provided for @manualPort.
  ///
  /// In en, this message translates to:
  /// **'Port'**
  String get manualPort;

  /// No description provided for @startCycle.
  ///
  /// In en, this message translates to:
  /// **'Start drying'**
  String get startCycle;

  /// No description provided for @stopCycle.
  ///
  /// In en, this message translates to:
  /// **'Stop'**
  String get stopCycle;

  /// No description provided for @stopConfirmTitle.
  ///
  /// In en, this message translates to:
  /// **'Stop drying?'**
  String get stopConfirmTitle;

  /// No description provided for @stopConfirmBody.
  ///
  /// In en, this message translates to:
  /// **'Heater and fan will turn off immediately.'**
  String get stopConfirmBody;

  /// No description provided for @cancel.
  ///
  /// In en, this message translates to:
  /// **'Cancel'**
  String get cancel;

  /// No description provided for @confirm.
  ///
  /// In en, this message translates to:
  /// **'Confirm'**
  String get confirm;

  /// No description provided for @exportCsv.
  ///
  /// In en, this message translates to:
  /// **'Export CSV'**
  String get exportCsv;

  /// No description provided for @exportPdf.
  ///
  /// In en, this message translates to:
  /// **'Export PDF'**
  String get exportPdf;

  /// No description provided for @noDevices.
  ///
  /// In en, this message translates to:
  /// **'No devices found. Try manual connect.'**
  String get noDevices;

  /// No description provided for @language.
  ///
  /// In en, this message translates to:
  /// **'Language'**
  String get language;

  /// No description provided for @switchDevice.
  ///
  /// In en, this message translates to:
  /// **'Switch device'**
  String get switchDevice;

  /// No description provided for @deviceSettings.
  ///
  /// In en, this message translates to:
  /// **'Device settings'**
  String get deviceSettings;

  /// No description provided for @savedDevices.
  ///
  /// In en, this message translates to:
  /// **'Saved devices'**
  String get savedDevices;

  /// No description provided for @deviceNotFound.
  ///
  /// In en, this message translates to:
  /// **'Device not found'**
  String get deviceNotFound;

  /// No description provided for @notificationSettings.
  ///
  /// In en, this message translates to:
  /// **'Notifications'**
  String get notificationSettings;

  /// No description provided for @notifyCycleComplete.
  ///
  /// In en, this message translates to:
  /// **'Cycle complete'**
  String get notifyCycleComplete;

  /// No description provided for @notifyCycleError.
  ///
  /// In en, this message translates to:
  /// **'Cycle error'**
  String get notifyCycleError;

  /// No description provided for @notifySafetyFault.
  ///
  /// In en, this message translates to:
  /// **'Safety fault'**
  String get notifySafetyFault;

  /// No description provided for @notifyConnectionLost.
  ///
  /// In en, this message translates to:
  /// **'Connection lost'**
  String get notifyConnectionLost;

  /// No description provided for @sessionCount.
  ///
  /// In en, this message translates to:
  /// **'{active} of {max} sessions'**
  String sessionCount(int active, int max);

  /// No description provided for @materialProfiles.
  ///
  /// In en, this message translates to:
  /// **'Material profiles'**
  String get materialProfiles;

  /// No description provided for @materialProfilesSubtitle.
  ///
  /// In en, this message translates to:
  /// **'Built-in presets and custom materials'**
  String get materialProfilesSubtitle;

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

  /// No description provided for @builtinProfile.
  ///
  /// In en, this message translates to:
  /// **'Built-in'**
  String get builtinProfile;

  /// No description provided for @noProfiles.
  ///
  /// In en, this message translates to:
  /// **'No profiles available'**
  String get noProfiles;

  /// No description provided for @maxCustomProfiles.
  ///
  /// In en, this message translates to:
  /// **'Maximum custom profiles reached'**
  String get maxCustomProfiles;

  /// No description provided for @deleteProfileTitle.
  ///
  /// In en, this message translates to:
  /// **'Delete profile?'**
  String get deleteProfileTitle;

  /// No description provided for @deleteProfileBody.
  ///
  /// In en, this message translates to:
  /// **'Delete {name}?'**
  String deleteProfileBody(String name);

  /// No description provided for @selectProfile.
  ///
  /// In en, this message translates to:
  /// **'Material profile'**
  String get selectProfile;

  /// No description provided for @manualParameters.
  ///
  /// In en, this message translates to:
  /// **'Manual parameters'**
  String get manualParameters;

  /// No description provided for @namePtLabel.
  ///
  /// In en, this message translates to:
  /// **'Name (PT)'**
  String get namePtLabel;

  /// No description provided for @nameEnLabel.
  ///
  /// In en, this message translates to:
  /// **'Name (EN)'**
  String get nameEnLabel;

  /// No description provided for @tempCLabel.
  ///
  /// In en, this message translates to:
  /// **'Temp °C'**
  String get tempCLabel;

  /// No description provided for @durationMinLabel.
  ///
  /// In en, this message translates to:
  /// **'Duration (min)'**
  String get durationMinLabel;

  /// No description provided for @humidityPctLabel.
  ///
  /// In en, this message translates to:
  /// **'Humidity %'**
  String get humidityPctLabel;

  /// No description provided for @saveLabel.
  ///
  /// In en, this message translates to:
  /// **'Save'**
  String get saveLabel;

  /// No description provided for @deviceConfig.
  ///
  /// In en, this message translates to:
  /// **'Device configuration'**
  String get deviceConfig;

  /// No description provided for @wifiSettings.
  ///
  /// In en, this message translates to:
  /// **'Wi‑Fi'**
  String get wifiSettings;

  /// No description provided for @advancedSettings.
  ///
  /// In en, this message translates to:
  /// **'Advanced (read-only)'**
  String get advancedSettings;

  /// No description provided for @backgroundNotAllowed.
  ///
  /// In en, this message translates to:
  /// **'Background monitoring unavailable. Keep the app open during drying.'**
  String get backgroundNotAllowed;

  /// No description provided for @dismiss.
  ///
  /// In en, this message translates to:
  /// **'Dismiss'**
  String get dismiss;

  /// No description provided for @sensorError.
  ///
  /// In en, this message translates to:
  /// **'Sensor Error'**
  String get sensorError;

  /// No description provided for @statusLabel.
  ///
  /// In en, this message translates to:
  /// **'Status'**
  String get statusLabel;

  /// No description provided for @chamberTemp.
  ///
  /// In en, this message translates to:
  /// **'Chamber temp'**
  String get chamberTemp;

  /// No description provided for @targetTemp.
  ///
  /// In en, this message translates to:
  /// **'Target temp'**
  String get targetTemp;

  /// No description provided for @humidity.
  ///
  /// In en, this message translates to:
  /// **'Humidity'**
  String get humidity;

  /// No description provided for @targetHumidity.
  ///
  /// In en, this message translates to:
  /// **'Target humidity'**
  String get targetHumidity;

  /// No description provided for @heater.
  ///
  /// In en, this message translates to:
  /// **'Heater'**
  String get heater;

  /// No description provided for @exhaustFan.
  ///
  /// In en, this message translates to:
  /// **'Exhaust fan'**
  String get exhaustFan;

  /// No description provided for @elapsedTime.
  ///
  /// In en, this message translates to:
  /// **'Elapsed'**
  String get elapsedTime;

  /// No description provided for @remainingTime.
  ///
  /// In en, this message translates to:
  /// **'Remaining'**
  String get remainingTime;

  /// No description provided for @cpuUsage.
  ///
  /// In en, this message translates to:
  /// **'CPU usage'**
  String get cpuUsage;

  /// No description provided for @memoryFree.
  ///
  /// In en, this message translates to:
  /// **'Free memory'**
  String get memoryFree;

  /// No description provided for @uptime.
  ///
  /// In en, this message translates to:
  /// **'Uptime'**
  String get uptime;

  /// No description provided for @heatingBadge.
  ///
  /// In en, this message translates to:
  /// **'Heating'**
  String get heatingBadge;

  /// No description provided for @coolingBadge.
  ///
  /// In en, this message translates to:
  /// **'Cooling'**
  String get coolingBadge;

  /// No description provided for @idleBadge.
  ///
  /// In en, this message translates to:
  /// **'Idle'**
  String get idleBadge;

  /// No description provided for @startWithProfile.
  ///
  /// In en, this message translates to:
  /// **'Start with profile'**
  String get startWithProfile;

  /// No description provided for @useCelsius.
  ///
  /// In en, this message translates to:
  /// **'Use Celsius'**
  String get useCelsius;

  /// No description provided for @useCelsiusSubtitle.
  ///
  /// In en, this message translates to:
  /// **'Off shows Fahrenheit on dashboard'**
  String get useCelsiusSubtitle;

  /// No description provided for @timeFormat.
  ///
  /// In en, this message translates to:
  /// **'Time format'**
  String get timeFormat;

  /// No description provided for @timeFormat24h.
  ///
  /// In en, this message translates to:
  /// **'24-hour'**
  String get timeFormat24h;

  /// No description provided for @timeFormat12h.
  ///
  /// In en, this message translates to:
  /// **'12-hour'**
  String get timeFormat12h;

  /// No description provided for @chartWindow.
  ///
  /// In en, this message translates to:
  /// **'Chart window'**
  String get chartWindow;

  /// No description provided for @chartWindowSubtitle.
  ///
  /// In en, this message translates to:
  /// **'Telemetry history shown on dashboard charts'**
  String get chartWindowSubtitle;

  /// No description provided for @maxConcurrentSessions.
  ///
  /// In en, this message translates to:
  /// **'Max concurrent sessions'**
  String get maxConcurrentSessions;

  /// No description provided for @pinLock.
  ///
  /// In en, this message translates to:
  /// **'PIN lock'**
  String get pinLock;

  /// No description provided for @pinLockSubtitle.
  ///
  /// In en, this message translates to:
  /// **'Optional — stub for MVP'**
  String get pinLockSubtitle;

  /// No description provided for @deviceConfigSubtitle.
  ///
  /// In en, this message translates to:
  /// **'Sensors, pins, display, WiFi'**
  String get deviceConfigSubtitle;

  /// No description provided for @testNotification.
  ///
  /// In en, this message translates to:
  /// **'Test notification'**
  String get testNotification;

  /// No description provided for @testNotificationSubtitle.
  ///
  /// In en, this message translates to:
  /// **'Local notifications adapter stub'**
  String get testNotificationSubtitle;

  /// No description provided for @systemDefault.
  ///
  /// In en, this message translates to:
  /// **'System default'**
  String get systemDefault;

  /// No description provided for @languageEnUs.
  ///
  /// In en, this message translates to:
  /// **'English (US)'**
  String get languageEnUs;

  /// No description provided for @languagePtBr.
  ///
  /// In en, this message translates to:
  /// **'Portuguese (Brazil)'**
  String get languagePtBr;

  /// No description provided for @highContrast.
  ///
  /// In en, this message translates to:
  /// **'High contrast'**
  String get highContrast;

  /// No description provided for @highContrastSubtitle.
  ///
  /// In en, this message translates to:
  /// **'Stronger colors for readability'**
  String get highContrastSubtitle;

  /// No description provided for @configReloadTooltip.
  ///
  /// In en, this message translates to:
  /// **'Reload from device'**
  String get configReloadTooltip;

  /// No description provided for @configSavedBanner.
  ///
  /// In en, this message translates to:
  /// **'Configuration saved on device.'**
  String get configSavedBanner;

  /// No description provided for @configSaveButton.
  ///
  /// In en, this message translates to:
  /// **'Save hardware configuration'**
  String get configSaveButton;

  /// No description provided for @configSensorsTitle.
  ///
  /// In en, this message translates to:
  /// **'Sensors'**
  String get configSensorsTitle;

  /// No description provided for @configTypeLabel.
  ///
  /// In en, this message translates to:
  /// **'Type'**
  String get configTypeLabel;

  /// No description provided for @configGpioLabel.
  ///
  /// In en, this message translates to:
  /// **'GPIO'**
  String get configGpioLabel;

  /// No description provided for @configActuatorsTitle.
  ///
  /// In en, this message translates to:
  /// **'Pins / actuators'**
  String get configActuatorsTitle;

  /// No description provided for @configPwmGpioLabel.
  ///
  /// In en, this message translates to:
  /// **'PWM GPIO'**
  String get configPwmGpioLabel;

  /// No description provided for @configDisplayTitle.
  ///
  /// In en, this message translates to:
  /// **'Display'**
  String get configDisplayTitle;

  /// No description provided for @configDisplayEnabled.
  ///
  /// In en, this message translates to:
  /// **'Enabled'**
  String get configDisplayEnabled;

  /// No description provided for @configDriverLabel.
  ///
  /// In en, this message translates to:
  /// **'Driver'**
  String get configDriverLabel;

  /// No description provided for @configWidthLabel.
  ///
  /// In en, this message translates to:
  /// **'Width'**
  String get configWidthLabel;

  /// No description provided for @configHeightLabel.
  ///
  /// In en, this message translates to:
  /// **'Height'**
  String get configHeightLabel;

  /// No description provided for @wifiSsidLabel.
  ///
  /// In en, this message translates to:
  /// **'SSID'**
  String get wifiSsidLabel;

  /// No description provided for @notConnected.
  ///
  /// In en, this message translates to:
  /// **'Not connected'**
  String get notConnected;

  /// No description provided for @wifiSignalLabel.
  ///
  /// In en, this message translates to:
  /// **'Signal'**
  String get wifiSignalLabel;

  /// No description provided for @notReportedMvp.
  ///
  /// In en, this message translates to:
  /// **'Not reported by device (MVP)'**
  String get notReportedMvp;

  /// No description provided for @wifiReconfigureButton.
  ///
  /// In en, this message translates to:
  /// **'Reconfigure WiFi (hotspot mode)'**
  String get wifiReconfigureButton;

  /// No description provided for @wifiReconfigureNote.
  ///
  /// In en, this message translates to:
  /// **'WiFi reconfiguration triggers hotspot mode on the device. Use the desktop installer for full WiFi setup in v1.'**
  String get wifiReconfigureNote;

  /// No description provided for @advancedDeviceName.
  ///
  /// In en, this message translates to:
  /// **'Device name'**
  String get advancedDeviceName;

  /// No description provided for @advancedFirmwareVersion.
  ///
  /// In en, this message translates to:
  /// **'Firmware version'**
  String get advancedFirmwareVersion;

  /// No description provided for @advancedNtpTimezone.
  ///
  /// In en, this message translates to:
  /// **'NTP timezone'**
  String get advancedNtpTimezone;

  /// No description provided for @advancedHardTempLimit.
  ///
  /// In en, this message translates to:
  /// **'Hard temp limit (°C)'**
  String get advancedHardTempLimit;

  /// No description provided for @advancedUptimeSec.
  ///
  /// In en, this message translates to:
  /// **'Uptime (sec)'**
  String get advancedUptimeSec;

  /// No description provided for @advancedCpuUsage.
  ///
  /// In en, this message translates to:
  /// **'CPU usage (%)'**
  String get advancedCpuUsage;

  /// No description provided for @advancedFreeMemory.
  ///
  /// In en, this message translates to:
  /// **'Free memory (bytes)'**
  String get advancedFreeMemory;

  /// No description provided for @historyFilterAll.
  ///
  /// In en, this message translates to:
  /// **'All'**
  String get historyFilterAll;

  /// No description provided for @historyFilterCompleted.
  ///
  /// In en, this message translates to:
  /// **'Completed'**
  String get historyFilterCompleted;

  /// No description provided for @historyFilterInProgress.
  ///
  /// In en, this message translates to:
  /// **'In progress'**
  String get historyFilterInProgress;

  /// No description provided for @historyCycleDefault.
  ///
  /// In en, this message translates to:
  /// **'Cycle'**
  String get historyCycleDefault;

  /// No description provided for @historyEmpty.
  ///
  /// In en, this message translates to:
  /// **'No cycles yet'**
  String get historyEmpty;

  /// No description provided for @selectDevice.
  ///
  /// In en, this message translates to:
  /// **'Select a device'**
  String get selectDevice;

  /// No description provided for @chartWindowMinutes.
  ///
  /// In en, this message translates to:
  /// **'{minutes} min'**
  String chartWindowMinutes(int minutes);

  /// No description provided for @wifiHotspotModeBody.
  ///
  /// In en, this message translates to:
  /// **'Device is in hotspot mode. Join WiFi network \"{ssid}\" to configure.'**
  String wifiHotspotModeBody(String ssid);

  /// No description provided for @wifiReconfigureTitle.
  ///
  /// In en, this message translates to:
  /// **'Reconfigure WiFi'**
  String get wifiReconfigureTitle;

  /// No description provided for @wifiReconfigureSteps.
  ///
  /// In en, this message translates to:
  /// **'1. Connect your phone or PC to WiFi \"{ssid}\".\n2. Open http://{host}/ in a browser.\n3. Enter your home WiFi credentials.\n4. Reconnect the control app on the new IP.'**
  String wifiReconfigureSteps(String ssid, String host);

  /// No description provided for @pendingCommandsTooltip.
  ///
  /// In en, this message translates to:
  /// **'{count, plural, =1{1 pending change} other{{count} pending changes}}'**
  String pendingCommandsTooltip(int count);

  /// No description provided for @resetProfilesDefaults.
  ///
  /// In en, this message translates to:
  /// **'Reset defaults'**
  String get resetProfilesDefaults;

  /// No description provided for @resetProfilesDefaultsTitle.
  ///
  /// In en, this message translates to:
  /// **'Reset profile defaults?'**
  String get resetProfilesDefaultsTitle;

  /// No description provided for @resetProfilesDefaultsBody.
  ///
  /// In en, this message translates to:
  /// **'Restore built-in material profiles from the device. Custom profiles are removed.'**
  String get resetProfilesDefaultsBody;

  /// No description provided for @maxConcurrentSessionsSubtitle.
  ///
  /// In en, this message translates to:
  /// **'Maximum simultaneous device connections'**
  String get maxConcurrentSessionsSubtitle;

  /// No description provided for @autoConnectLabel.
  ///
  /// In en, this message translates to:
  /// **'Auto-connect'**
  String get autoConnectLabel;

  /// No description provided for @autoConnectSubtitle.
  ///
  /// In en, this message translates to:
  /// **'Prefer this device on app launch'**
  String get autoConnectSubtitle;

  /// No description provided for @lastSeenLabel.
  ///
  /// In en, this message translates to:
  /// **'Last seen'**
  String get lastSeenLabel;

  /// No description provided for @firmwareVersionLabel.
  ///
  /// In en, this message translates to:
  /// **'Firmware version'**
  String get firmwareVersionLabel;

  /// No description provided for @notReported.
  ///
  /// In en, this message translates to:
  /// **'Not reported'**
  String get notReported;
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
