/*
 * Philarmony Filament Dryer — Desktop Installer / Shared Core
 * Copyright (C) 2026 Philarmony Contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/// Philarmony shared domain for desktop installer and control app.
library philarmony_core;

export 'src/models/device_profile.dart';
export 'src/models/filament_profile.dart';
export 'src/models/flash_job.dart';
export 'src/models/firmware_package.dart';
export 'src/models/installer_session.dart';
export 'src/models/known_device.dart';
export 'src/models/hardware_config.dart';
export 'src/models/ws_models.dart';
export 'src/validation/pin_validator.dart';
export 'src/validation/filament_profile_validator.dart';
export 'src/validation/cycle_command_validator.dart';
export 'src/serialization/profile_codec.dart';
export 'src/serialization/profile_store.dart';
export 'src/mapping/nvs_config_mapper.dart';
export 'src/mapping/nvs_binary_writer.dart';
export 'src/device/device_interfaces.dart';
export 'src/ws/ws_codec.dart';
