/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';

enum AppBreakpoint { compact, medium, expanded }

class AppBreakpointData {
  const AppBreakpointData(this.width);

  final double width;

  AppBreakpoint get breakpoint {
    if (width < 600) return AppBreakpoint.compact;
    if (width < 1024) return AppBreakpoint.medium;
    return AppBreakpoint.expanded;
  }

  bool get useBottomNav => breakpoint == AppBreakpoint.compact;
  bool get useMasterDetail => breakpoint == AppBreakpoint.expanded;
}

AppBreakpointData appBreakpointOf(BuildContext context) {
  return AppBreakpointData(MediaQuery.sizeOf(context).width);
}
