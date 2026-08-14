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
