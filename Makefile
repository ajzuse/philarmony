# Philarmony — PlatformIO façade (DEC-009 / makefile-targets.md)
# Requires: PlatformIO Core (`pio`) on PATH.

ENV          ?= esp32devkitc
PORT         ?=
UPLOAD_PORT  ?= $(PORT)

# Resolve pio if only installed under ~/.platformio/penv
PIO := $(shell command -v pio 2>/dev/null || echo "$(HOME)/.platformio/penv/bin/pio")

UPLOAD_FLAGS :=
ifneq ($(UPLOAD_PORT),)
UPLOAD_FLAGS += --upload-port $(UPLOAD_PORT)
endif

MONITOR_FLAGS :=
ifneq ($(UPLOAD_PORT),)
MONITOR_FLAGS += --port $(UPLOAD_PORT)
endif

.PHONY: help all build compile test flash upload install uploadfs monitor clean

.DEFAULT_GOAL := help

help:
	@echo "Philarmony Make targets (wraps PlatformIO)"
	@echo ""
	@echo "  make build|compile     Build firmware for ENV (default: $(ENV))"
	@echo "  make test              Host Unity tests (pio test -e native)"
	@echo "  make flash|upload|install  Flash app to ESP32 (needs device)"
	@echo "  make uploadfs          Upload LittleFS image"
	@echo "  make monitor           Serial monitor (115200)"
	@echo "  make clean             Clean ENV (+ native) build artifacts"
	@echo "  make help              This message"
	@echo ""
	@echo "Variables:"
	@echo "  ENV=$(ENV)   Allowed: esp32devkitc, lilygo_tdisplay_v1, esp32_2432s028"
	@echo "  PORT / UPLOAD_PORT   Optional serial device for flash/monitor"
	@echo ""
	@echo "Examples:"
	@echo "  make build ENV=lilygo_tdisplay_v1"
	@echo "  make flash PORT=/dev/ttyUSB0"
	@echo "  make test"

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
