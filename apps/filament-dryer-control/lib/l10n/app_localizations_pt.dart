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
  String get reconnecting => 'Reconectando';

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

  @override
  String get backgroundNotAllowed =>
      'Monitoramento em segundo plano indisponível. Mantenha o app aberto durante a secagem.';

  @override
  String get dismiss => 'Dispensar';

  @override
  String get sensorError => 'Erro de sensor';

  @override
  String get statusLabel => 'Status';

  @override
  String get chamberTemp => 'Temp. câmara';

  @override
  String get targetTemp => 'Temp. alvo';

  @override
  String get humidity => 'Umidade';

  @override
  String get targetHumidity => 'Umidade alvo';

  @override
  String get heater => 'Aquecedor';

  @override
  String get exhaustFan => 'Ventoinha';

  @override
  String get elapsedTime => 'Decorrido';

  @override
  String get remainingTime => 'Restante';

  @override
  String get cpuUsage => 'Uso CPU';

  @override
  String get memoryFree => 'Memória livre';

  @override
  String get uptime => 'Tempo ativo';

  @override
  String get heatingBadge => 'Aquecendo';

  @override
  String get coolingBadge => 'Resfriando';

  @override
  String get idleBadge => 'Ocioso';

  @override
  String get startWithProfile => 'Iniciar com perfil';

  @override
  String get useCelsius => 'Usar Celsius';

  @override
  String get useCelsiusSubtitle => 'Desligado mostra Fahrenheit no painel';

  @override
  String get timeFormat => 'Formato de hora';

  @override
  String get timeFormat24h => '24 horas';

  @override
  String get timeFormat12h => '12 horas';

  @override
  String get chartWindow => 'Janela dos gráficos';

  @override
  String get chartWindowSubtitle =>
      'Histórico de telemetria nos gráficos do painel';

  @override
  String get maxConcurrentSessions => 'Máx. sessões simultâneas';

  @override
  String get pinLock => 'Bloqueio PIN';

  @override
  String get pinLockSubtitle => 'Opcional — stub para MVP';

  @override
  String get deviceConfigSubtitle => 'Sensores, pinos, display, WiFi';

  @override
  String get testNotification => 'Testar notificação';

  @override
  String get testNotificationSubtitle =>
      'Stub do adaptador de notificações locais';

  @override
  String get systemDefault => 'Padrão do sistema';

  @override
  String get languageEnUs => 'Inglês (EUA)';

  @override
  String get languagePtBr => 'Português (Brasil)';

  @override
  String get highContrast => 'Alto contraste';

  @override
  String get highContrastSubtitle => 'Cores mais fortes para melhor leitura';

  @override
  String get configReloadTooltip => 'Recarregar do dispositivo';

  @override
  String get configSavedBanner => 'Configuração salva no dispositivo.';

  @override
  String get configSaveButton => 'Salvar configuração de hardware';

  @override
  String get configSensorsTitle => 'Sensores';

  @override
  String get configTypeLabel => 'Tipo';

  @override
  String get configGpioLabel => 'GPIO';

  @override
  String get configActuatorsTitle => 'Pinos / atuadores';

  @override
  String get configPwmGpioLabel => 'GPIO PWM';

  @override
  String get configDisplayTitle => 'Display';

  @override
  String get configDisplayEnabled => 'Ativado';

  @override
  String get configDriverLabel => 'Driver';

  @override
  String get configWidthLabel => 'Largura';

  @override
  String get configHeightLabel => 'Altura';

  @override
  String get wifiSsidLabel => 'SSID';

  @override
  String get notConnected => 'Não conectado';

  @override
  String get wifiSignalLabel => 'Sinal';

  @override
  String get notReportedMvp => 'Não informado pelo dispositivo (MVP)';

  @override
  String get wifiReconfigureButton => 'Reconfigurar WiFi (modo hotspot)';

  @override
  String get wifiReconfigureNote =>
      'A reconfiguração de WiFi ativa o modo hotspot no dispositivo. Use o instalador desktop para configuração completa de WiFi na v1.';

  @override
  String get advancedDeviceName => 'Nome do dispositivo';

  @override
  String get advancedFirmwareVersion => 'Versão do firmware';

  @override
  String get advancedNtpTimezone => 'Fuso horário NTP';

  @override
  String get advancedHardTempLimit => 'Limite rígido de temp. (°C)';

  @override
  String get advancedUptimeSec => 'Tempo ativo (seg)';

  @override
  String get advancedCpuUsage => 'Uso CPU (%)';

  @override
  String get advancedFreeMemory => 'Memória livre (bytes)';

  @override
  String get historyFilterAll => 'Todos';

  @override
  String get historyFilterCompleted => 'Concluídos';

  @override
  String get historyFilterInProgress => 'Em andamento';

  @override
  String get historyCycleDefault => 'Ciclo';

  @override
  String get historyEmpty => 'Nenhum ciclo ainda';

  @override
  String get selectDevice => 'Selecione um dispositivo';

  @override
  String chartWindowMinutes(int minutes) {
    return '$minutes min';
  }

  @override
  String wifiHotspotModeBody(String ssid) {
    return 'Dispositivo em modo hotspot. Conecte-se à rede WiFi \"$ssid\" para configurar.';
  }

  @override
  String get wifiReconfigureTitle => 'Reconfigurar WiFi';

  @override
  String wifiReconfigureSteps(String ssid, String host) {
    return '1. Conecte o celular ou PC ao WiFi \"$ssid\".\n2. Abra http://$host/ no navegador.\n3. Informe as credenciais da sua rede.\n4. Reconecte o app no novo IP.';
  }

  @override
  String pendingCommandsTooltip(int count) {
    String _temp0 = intl.Intl.pluralLogic(
      count,
      locale: localeName,
      other: '$count alterações pendentes',
      one: '1 alteração pendente',
    );
    return '$_temp0';
  }

  @override
  String get resetProfilesDefaults => 'Restaurar padrões';

  @override
  String get resetProfilesDefaultsTitle => 'Restaurar perfis padrão?';

  @override
  String get resetProfilesDefaultsBody =>
      'Restaura os perfis integrados do dispositivo. Perfis customizados serão removidos.';

  @override
  String get maxConcurrentSessionsSubtitle => 'Máximo de conexões simultâneas';

  @override
  String get autoConnectLabel => 'Conectar automaticamente';

  @override
  String get autoConnectSubtitle => 'Preferir este dispositivo ao abrir o app';

  @override
  String get lastSeenLabel => 'Visto por último';

  @override
  String get firmwareVersionLabel => 'Versão do firmware';

  @override
  String get notReported => 'Não informado';
}
