/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/widgets.dart';
import 'package:philarmony_core/philarmony_core.dart';

String profileDisplayName(FilamentProfile profile, Locale locale) {
  if (locale.languageCode == 'pt') {
    return profile.namePt.isNotEmpty ? profile.namePt : profile.nameEn;
  }
  return profile.nameEn.isNotEmpty ? profile.nameEn : profile.namePt;
}
