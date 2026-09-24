/*
 * Philarmony Filament Dryer ESP32 Firmware
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

/*
 * Host-compiler shims for PlatformIO `platform = native` (host tests).
 *
 * PlatformIO's native environment invokes `gcc`, `g++`, `ar` and `ranlib`.
 * On machines without a host GNU toolchain we build these tiny launchers from
 * this single source and forward to the equivalent `zig` subcommand, so
 * `pio test -e native` works with just the `ziglang` pip package.
 *
 * Build (see scripts/setup-native-toolchain.bat):
 *   zig cc -w -o gcc.exe   zig-shim.c
 *   zig cc -w -o g++.exe   zig-shim.c
 *   zig cc -w -o ar.exe    zig-shim.c
 *   zig cc -w -o ranlib.exe zig-shim.c
 *
 * The executable's own name selects the zig subcommand:
 *   gcc / cc        -> zig cc
 *   g++ / c++       -> zig c++
 *   ar              -> zig ar
 *   ranlib          -> zig ranlib
 *
 * The zig executable is taken from the PHILARMONY_ZIG environment variable
 * (absolute path), falling back to `zig` resolved via PATH.
 */
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* base_name(const char* path) {
    const char* last = path;
    const char* p;
    for (p = path; *p; ++p) {
        if (*p == '\\' || *p == '/') last = p + 1;
    }
    return last;
}

int main(int argc, char** argv) {
    const char* exe = base_name(argv[0]);
    const char* mode = "cc";
    if (strstr(exe, "++") != NULL) {
        mode = "c++";
    } else if (strncmp(exe, "ranlib", 6) == 0) {
        mode = "ranlib";
    } else if (strncmp(exe, "ar", 2) == 0) {
        mode = "ar";
    }

    const char* zig = getenv("PHILARMONY_ZIG");
    if (zig == NULL || zig[0] == '\0') {
        zig = "zig";
    }

    char** newargv = (char**)malloc(sizeof(char*) * (size_t)(argc + 2));
    if (newargv == NULL) {
        return 127;
    }
    newargv[0] = (char*)zig;
    newargv[1] = (char*)mode;
    for (int i = 1; i < argc; ++i) {
        newargv[i + 1] = argv[i];
    }
    newargv[argc + 1] = NULL;

    _execvp(zig, newargv);
    perror("zig");
    return 127;
}
