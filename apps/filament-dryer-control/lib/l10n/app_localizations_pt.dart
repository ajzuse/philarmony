// ignore: unused_import
import 'package:intl/intl.dart' as intl;
import 'app_localizations.dart';

// ignore_for_file: type=lint

/// The translations for Portuguese (`pt`).
class AppLocalizationsPt extends AppLocalizations {
  AppLocalizationsPt([String locale = 'pt']) : super(locale);

  @override
  String get appTitle => 'Philarmony Controle';

  @override
  String get navDashboard => 'Painel';

  @override
  String get navHistory => 'Histórico';

  @override
  String get navDevices => 'Dispositivos';

  @override
  String get navSettings => 'Configurações';

  @override
  String get scan => 'Buscar';

  @override
  String get connect => 'Conectar';

  @override
  String get connected => 'Conectado';

  @override
  String get connecting => 'Conectando';

  @override
  String get disconnected => 'Desconectado';

  @override
  String get manualHost => 'Host';

  @override
  String get manualPort => 'Porta';

  @override
  String get startCycle => 'Iniciar secagem';

  @override
  String get stopCycle => 'Parar';

  @override
  String get stopConfirmTitle => 'Parar secagem?';

  @override
  String get stopConfirmBody => 'Aquecedor e ventoinha desligam imediatamente.';

  @override
  String get cancel => 'Cancelar';

  @override
  String get confirm => 'Confirmar';

  @override
  String get exportCsv => 'Exportar CSV';

  @override
  String get exportPdf => 'Exportar PDF';

  @override
  String get noDevices => 'Nenhum dispositivo. Tente conexão manual.';

  @override
  String get language => 'Idioma';

  @override
  String get switchDevice => 'Trocar dispositivo';

  @override
  String get deviceSettings => 'Configurações do dispositivo';

  @override
  String get savedDevices => 'Dispositivos salvos';

  @override
  String get deviceNotFound => 'Dispositivo não encontrado';

  @override
  String get notificationSettings => 'Notificações';

  @override
  String get notifyCycleComplete => 'Ciclo concluído';

  @override
  String get notifyCycleError => 'Erro no ciclo';

  @override
  String get notifySafetyFault => 'Falha de segurança';

  @override
  String get notifyConnectionLost => 'Conexão perdida';

  @override
  String sessionCount(int active, int max) {
    return '$active de $max sessões';
  }

  @override
  String get materialProfiles => 'Perfis de material';

  @override
  String get materialProfilesSubtitle =>
      'Presets integrados e materiais personalizados';

  @override
  String get addCustomProfile => 'Adicionar perfil customizado';

  @override
  String get editCustomProfile => 'Editar perfil customizado';

  @override
  String get builtinProfile => 'Integrado';

  @override
  String get noProfiles => 'Nenhum perfil disponível';

  @override
  String get maxCustomProfiles => 'Máximo de perfis customizados atingido';

  @override
  String get deleteProfileTitle => 'Excluir perfil?';

  @override
  String deleteProfileBody(String name) {
    return 'Excluir $name?';
  }

  @override
  String get selectProfile => 'Perfil de material';

  @override
  String get manualParameters => 'Parâmetros manuais';

  @override
  String get namePtLabel => 'Nome (PT)';

  @override
  String get nameEnLabel => 'Nome (EN)';

  @override
  String get tempCLabel => 'Temp °C';

  @override
  String get durationMinLabel => 'Duração (min)';

  @override
  String get humidityPctLabel => 'Umidade %';

  @override
  String get saveLabel => 'Salvar';

  @override
  String get deviceConfig => 'Configuração do dispositivo';

  @override
  String get wifiSettings => 'Wi‑Fi';

  @override
  String get advancedSettings => 'Avançado (somente leitura)';
}
