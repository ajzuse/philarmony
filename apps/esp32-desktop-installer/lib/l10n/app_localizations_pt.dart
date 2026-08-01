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

  @override
  String get requiredField => 'Obrigatório';

  @override
  String get optionalField => 'Opcional';

  @override
  String get optionalAdvanced => 'Padrão 921600 (configurável)';

  @override
  String get deviceModelLabel => 'Modelo ESP32';

  @override
  String get flashSizeLabel => 'Tamanho do flash (MB)';

  @override
  String get baudLabel => 'Taxa baud';

  @override
  String get usbPortLabel => 'Porta USB';

  @override
  String get serialPortLabel => 'Porta serial';

  @override
  String get customPartitionLabel => 'Tabela de partições personalizada';

  @override
  String get partialFlashWarn =>
      'O USB pode ter sido desconectado durante o flash. O dispositivo pode estar em estado parcial — reconecte e tente novamente com erase→write→verify completo.';

  @override
  String get retryHelp =>
      'Tentar novamente executa erase→write→verify completo do pacote atual. Nenhuma imagem anterior é restaurada automaticamente.';

  @override
  String get tempSensorLabel => 'Sensor de temperatura';

  @override
  String get humiditySensorLabel => 'Sensor de umidade';

  @override
  String get tempGpioLabel => 'GPIO temperatura';

  @override
  String get humidityGpioLabel => 'GPIO umidade';

  @override
  String get advancedDriverTitle => 'Avançado: driver JSON personalizado';

  @override
  String get advancedDriverSubtitle =>
      'Opcional — mescla nos parâmetros do sensor';

  @override
  String get driverTargetLabel => 'Aplicar JSON em';

  @override
  String get driverJsonLabel => 'JSON do driver';

  @override
  String get pinoutTitle => 'Pinagem';

  @override
  String get heaterPwmLabel => 'PWM aquecedor';

  @override
  String get exhaustFanPwmLabel => 'PWM exaustor';

  @override
  String get exhaustFanDigitalLabel => 'Exaustor digital';

  @override
  String get enableDisplayLabel => 'Ativar display';

  @override
  String get displayDriverLabel => 'Driver';

  @override
  String get resolutionPresetLabel => 'Preset de resolução';

  @override
  String get customLabel => 'Personalizado';

  @override
  String get widthLabel => 'Largura';

  @override
  String get heightLabel => 'Altura';

  @override
  String get statusFieldsLabel => 'Campos de status';

  @override
  String get layoutPreviewLabel => 'Prévia do layout';

  @override
  String get noFieldsLabel => '(sem campos)';

  @override
  String get spiDcLabel => 'Pino SPI DC';

  @override
  String get spiResetLabel => 'Pino SPI RESET';

  @override
  String get addCustomProfile => 'Adicionar perfil personalizado';

  @override
  String get editCustomProfile => 'Editar perfil personalizado';

  @override
  String get overrideBuiltinTitle => 'Sobrescrever embutido';

  @override
  String get namePtLabel => 'Nome PT';

  @override
  String get nameEnLabel => 'Nome EN';

  @override
  String get tempCLabel => 'Temp °C';

  @override
  String get durationMinLabel => 'Duração min';

  @override
  String get humidityPctLabel => 'Umidade %';

  @override
  String get cancelLabel => 'Cancelar';

  @override
  String get saveLabel => 'Salvar';

  @override
  String get ssidLabel => 'SSID';

  @override
  String get passwordLabel => 'Senha';

  @override
  String get staticIpTitle => 'IP estático (opcional)';

  @override
  String get dismissLabel => 'Dispensar';

  @override
  String get stepLabel => 'Etapa';
}
