# Philarmony — PlatformIO façade (DEC-009) + desktop installer (DEC-011 / FVM)
# Requires: PlatformIO Core (`pio`) on PATH.
# Flutter: FVM pin 3.44.0 (`.fvmrc`) — all installer targets use `fvm flutter` / `fvm dart`.

ENV          ?= esp32devkitc
PORT         ?=
UPLOAD_PORT  ?= $(PORT)

INSTALLER_APP := apps/esp32-desktop-installer
CONTROL_APP := apps/filament-dryer-control
INSTALLER_CORE := packages/philarmony_core
INSTALLER_DIST := $(INSTALLER_APP)/dist
FIRMWARE_ASSET_DIR := $(INSTALLER_APP)/assets/firmware

# Default Flutter device: host desktop OS
UNAME_S := $(shell uname -s 2>/dev/null)
ifeq ($(UNAME_S),Darwin)
DEVICE ?= macos
else ifeq ($(UNAME_S),Linux)
DEVICE ?= linux
else
DEVICE ?= windows
endif

# Resolve pio if only installed under ~/.platformio/penv
PIO := $(shell command -v pio 2>/dev/null || echo "$(HOME)/.platformio/penv/bin/pio")
# Prefer project FVM Flutter 3.44.0; fall back to PATH flutter
FVM := $(shell command -v fvm 2>/dev/null)
ifeq ($(FVM),)
FLUTTER := $(shell command -v flutter 2>/dev/null)
DART := $(shell command -v dart 2>/dev/null)
else
FLUTTER := $(FVM) flutter
DART := $(FVM) dart
endif

UPLOAD_FLAGS :=
ifneq ($(UPLOAD_PORT),)
UPLOAD_FLAGS += --upload-port $(UPLOAD_PORT)
endif

MONITOR_FLAGS :=
ifneq ($(UPLOAD_PORT),)
MONITOR_FLAGS += --port $(UPLOAD_PORT)
endif

define require_flutter
	@test -n "$(FLUTTER)" || (echo "fvm/flutter not on PATH — install FVM and run: fvm use 3.44.0"; exit 1)
endef

.PHONY: help all build compile test flash upload install uploadfs monitor clean \
	sync-installer-firmware bundle-esptool package-installer package-installer-linux \
	package-installer-linux-appimage package-installer-linux-deb package-installer-linux-rpm \
	package-installer-windows package-installer-macos \
	run run-installer run-control run-macos run-linux run-windows \
	devices doctor pub-get create-platforms \
	test-core test-installer test-control test-flutter \
	clean-installer clean-control

.DEFAULT_GOAL := all

help:
	@echo "Philarmony Make targets"
	@echo ""
	@echo "Firmware (PlatformIO):"
	@echo "  make / make all / make build|compile     Build firmware for ENV (default: $(ENV))"
	@echo "  make test              Host Unity tests (pio test -e native)"
	@echo "  make flash|upload|install  Flash app to ESP32 (needs device)"
	@echo "  make uploadfs          Upload LittleFS image"
	@echo "  make monitor           Serial monitor (115200)"
	@echo "  make clean             Clean ENV (+ native) build artifacts"
	@echo ""
	@echo "Desktop installer — run / dev (Flutter via FVM 3.44.0):"
	@echo "  make run | run-installer   Run app (DEVICE=$(DEVICE); override: DEVICE=macos)"
	@echo "  make run-macos | run-linux | run-windows"
	@echo "  make devices               List Flutter devices"
	@echo "  make doctor                fvm flutter doctor"
	@echo "  make pub-get               Pub get (core + installer)"
	@echo "  make create-platforms      flutter create --platforms=windows,macos,linux"
	@echo "  make test-core             dart test in packages/philarmony_core"
	@echo "  make test-installer        flutter test in installer app"
	@echo "  make test-control            flutter test in control app"
	@echo "  make test-flutter          test-core + test-installer + test-control"
	@echo "  make run-control           Run control app (DEVICE=$(DEVICE))"
	@echo "  make package-control-linux Build linux release for control app"
	@echo "  make package-control-macos / package-control-windows  (host OS)"
	@echo "  make package-control          Alias → package-control-linux"
	@echo "  make clean-installer       flutter clean"
	@echo ""
	@echo "Desktop installer — packaging:"
	@echo "  make sync-installer-firmware   Copy .pio firmware.bin into installer assets"
	@echo "  make bundle-esptool            Download standalone esptool into assets/tools"
	@echo "  make package-installer-linux   AppImage + deb + rpm"
	@echo "  make package-installer-linux-appimage|deb|rpm"
	@echo "  make package-installer-windows / package-installer-macos  (host OS required)"
	@echo "  (uses \`fvm flutter\` when fvm is on PATH; pin: .fvmrc → 3.44.0)"
	@echo ""
	@echo "Variables: ENV=$(ENV)  PORT / UPLOAD_PORT  DEVICE=$(DEVICE)"

all: build

build compile:
	$(PIO) run -e $(ENV)

test:
	$(PIO) test -e native

flash upload install:
	$(PIO) run -e $(ENV) -t upload $(UPLOAD_FLAGS)

uploadfs:
	$(PIO) run -e $(ENV) -t uploadfs $(UPLOAD_FLAGS)

monitor:
	$(PIO) device monitor -b 115200 -e $(ENV) $(MONITOR_FLAGS)

clean:
	$(PIO) run -e $(ENV) -t clean
	-$(PIO) run -e native -t clean

# --- Desktop installer: run / test / tooling (FVM) ---

devices:
	$(call require_flutter)
	$(FLUTTER) devices

doctor:
	$(call require_flutter)
	$(FLUTTER) doctor -v

pub-get:
	$(call require_flutter)
	cd $(INSTALLER_CORE) && $(DART) pub get
	cd $(INSTALLER_APP) && $(FLUTTER) pub get
	cd $(CONTROL_APP) && $(FLUTTER) pub get

create-platforms:
	$(call require_flutter)
	cd $(INSTALLER_APP) && $(FLUTTER) create --platforms=windows,macos,linux .

run run-installer: pub-get
	$(call require_flutter)
	cd $(INSTALLER_APP) && $(FLUTTER) run -d $(DEVICE)

run-macos:
	@$(MAKE) run DEVICE=macos

run-linux:
	@$(MAKE) run DEVICE=linux

run-windows:
	@$(MAKE) run DEVICE=windows

test-core:
	$(call require_flutter)
	cd $(INSTALLER_CORE) && $(DART) pub get && $(DART) test

test-installer:
	$(call require_flutter)
	cd $(INSTALLER_APP) && $(FLUTTER) pub get && $(FLUTTER) test

test-control:
	$(call require_flutter)
	cd $(CONTROL_APP) && $(FLUTTER) pub get && $(FLUTTER) gen-l10n && $(FLUTTER) test

test-flutter: test-core test-installer test-control

clean-control:
	$(call require_flutter)
	cd $(CONTROL_APP) && $(FLUTTER) clean

run-control: pub-get
	$(call require_flutter)
	cd $(CONTROL_APP) && $(FLUTTER) gen-l10n && $(FLUTTER) run -d $(DEVICE)

package-control-linux:
	$(call require_flutter)
	@mkdir -p dist/control-linux
	cd $(CONTROL_APP) && $(FLUTTER) build linux --release
	@bundle=$$(ls -d $(CONTROL_APP)/build/linux/*/release/bundle 2>/dev/null | head -1); \
	  if [ -n "$$bundle" ]; then cp -a "$$bundle/." dist/control-linux/; \
	  echo "Packed → dist/control-linux/"; else echo "Linux bundle not found"; exit 1; fi

package-control-linux-appimage: package-control-linux
	@chmod +x $(CONTROL_APP)/packaging/build-appimage.sh
	$(CONTROL_APP)/packaging/build-appimage.sh

package-control-macos:
	$(call require_flutter)
	@mkdir -p dist/control-macos
	cd $(CONTROL_APP) && $(FLUTTER) build macos --release
	@chmod +x $(CONTROL_APP)/packaging/build-dmg.sh
	$(CONTROL_APP)/packaging/build-dmg.sh

package-control-windows:
	$(call require_flutter)
	@mkdir -p dist/control-windows
	cd $(CONTROL_APP) && $(FLUTTER) build windows --release
	@chmod +x $(CONTROL_APP)/packaging/build-msix.sh
	cd $(CONTROL_APP) && $(DART) pub get && bash packaging/build-msix.sh

package-control: package-control-linux

clean-installer:
	$(call require_flutter)
	cd $(INSTALLER_APP) && $(FLUTTER) clean
	cd $(INSTALLER_CORE) && rm -rf .dart_tool

# --- Desktop installer: packaging ---

sync-installer-firmware: build
	@mkdir -p $(FIRMWARE_ASSET_DIR)
	@src=".pio/build/$(ENV)"; \
	for f in firmware.bin bootloader.bin partitions.bin; do \
	  if [ -f "$$src/$$f" ]; then cp "$$src/$$f" "$(FIRMWARE_ASSET_DIR)/$$f"; \
	    echo "Copied $$src/$$f -> $(FIRMWARE_ASSET_DIR)/$$f"; \
	  else echo "Missing $$src/$$f — run make build first"; exit 1; fi; \
	done

bundle-esptool:
	@chmod +x scripts/bundle-esptool.sh
	./scripts/bundle-esptool.sh

package-installer: package-installer-linux

package-installer-linux: package-installer-linux-appimage package-installer-linux-deb package-installer-linux-rpm

package-installer-linux-appimage:
	$(call require_flutter)
	@mkdir -p $(INSTALLER_DIST)
	cd $(INSTALLER_APP) && $(FLUTTER) build linux --release
	@chmod +x $(INSTALLER_APP)/packaging/linux/scripts/build_appimage.sh
	$(INSTALLER_APP)/packaging/linux/scripts/build_appimage.sh

package-installer-linux-deb:
	$(call require_flutter)
	@mkdir -p $(INSTALLER_DIST)
	cd $(INSTALLER_APP) && $(FLUTTER) build linux --release
	@chmod +x $(INSTALLER_APP)/packaging/linux/scripts/build_deb.sh
	$(INSTALLER_APP)/packaging/linux/scripts/build_deb.sh

package-installer-linux-rpm:
	$(call require_flutter)
	@mkdir -p $(INSTALLER_DIST)
	cd $(INSTALLER_APP) && $(FLUTTER) build linux --release
	@chmod +x $(INSTALLER_APP)/packaging/linux/scripts/build_rpm.sh
	$(INSTALLER_APP)/packaging/linux/scripts/build_rpm.sh

package-installer-windows:
	$(call require_flutter)
	cd $(INSTALLER_APP) && $(FLUTTER) build windows --release && $(DART) run msix:create
	@echo "MSIX artifact naming: see $(INSTALLER_APP)/packaging/README.md (unsigned OK for MVP)"

package-installer-macos:
	$(call require_flutter)
	cd $(INSTALLER_APP) && $(FLUTTER) build macos --release
	@chmod +x $(INSTALLER_APP)/packaging/macos/build_dmg.sh
	$(INSTALLER_APP)/packaging/macos/build_dmg.sh
	@echo "Public release REQUIRES Developer ID sign + Apple notarization before publishing DMG"
