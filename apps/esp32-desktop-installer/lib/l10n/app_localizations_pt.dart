// ignore: unused_import
import 'package:intl/intl.dart' as intl;
import 'app_localizations.dart';

// ignore_for_file: type=lint

/// The translations for Portuguese (`pt`).
class AppLocalizationsPt extends AppLocalizations {
  AppLocalizationsPt([String locale = 'pt']) : super(locale);

  @override
  String get appTitle => 'Instalador Philarmony';

  @override
  String get next => 'Próximo';

  @override
  String get back => 'Voltar';

  @override
  String get install => 'Instalar';

  @override
  String get retry => 'Tentar novamente';

  @override
  String get exportProfile => 'Exportar perfil';

  @override
  String get importProfile => 'Importar perfil';

  @override
  String get loadLastSession => 'Carregar última sessão';

  @override
  String get newSetup => 'Nova configuração';

  @override
  String get flashSuccess => 'Flash concluído';

  @override
  String get networkWarn =>
      'Dispositivo inacessível na rede (verificação opcional). O flash ainda foi bem-sucedido.';

  @override
  String get pinConflict => 'Corrija conflitos de pinos antes de continuar';

  @override
  String get wifiPasswordRequired => 'Redigite a senha WiFi antes do flash';

  @override
  String get noPorts =>
      'Nenhuma porta USB serial encontrada. Instale drivers CH340/CP210x/FTDI e reconecte.';

  @override
  String get language => 'Idioma';
}
