# HTTP API Contract & Fixed Log Download Paths

## Overview
The HTTP server runs on port 80 when connected to WiFi, or on AP mode fixed IP `192.168.4.1`.

---

## 1. Fixed Log File Download Paths

### 1.1 Download Drying Session Log
- **Endpoint**: `GET /log/drying`
- **Description**: Downloads the current or last drying session log file (`drying.log`). Recreated at session start, preserved post-mortem.
- **Content-Type**: `text/plain; charset=utf-8`
- **Content-Disposition**: `attachment; filename="drying.log"`
- **Response Codes**:
  - `200 OK`: File stream
  - `404 Not Found`: No drying session recorded yet

### 1.2 Download System Log
- **Endpoint**: `GET /log/system`
- **Description**: Downloads the general system boot & operational log (`system.log`).
- **Content-Type**: `text/plain; charset=utf-8`
- **Content-Disposition**: `attachment; filename="system.log"`
- **Response Codes**:
  - `200 OK`: File stream

---

## 2. System Endpoints

### 2.1 WiFi Configuration (Hotspot AP Mode)
- **Endpoint**: `POST /api/wifi/config`
- **Request Body** (Form or JSON):
```json
{
  "ssid": "MinhaRedeWiFi",
  "password": "SenhaSuperSegura"
}
```
- **Response Code**: `200 OK`
```json
{
  "status": "success",
  "message": "Credenciais salvas. Reiniciando conexao..."
}
```

### 2.2 Device Information & Health Check
- **Endpoint**: `GET /api/info`
- **Response Code**: `200 OK`
```json
{
  "firmware_version": "0.1.0",
  "chip_model": "ESP32-D0WD-V3",
  "mac_address": "AA:BB:CC:DD:EE:FF",
  "free_heap_bytes": 224500,
  "system_status": "ready",
  "active_feature": "001-filament-dryer-esp32"
}
```

### 2.3 Hardware Configuration (GET/POST)
- **Endpoint**: `GET /api/hardware/config`
- **Response**: Current hardware configuration as JSON (matching WebSocket config/hardware)

- **Endpoint**: `POST /api/hardware/config`
- **Request Body**: Hardware configuration JSON (same as WebSocket `config/hardware`)
- **Response**: `{ "status": "saved" }` or `{ "error": "..." }`

---

## 3. Captive Portal Endpoints (AP Mode)

| Endpoint | Method | Purpose |
|----------|--------|---------|
| `/` | GET | Captive portal HTML (WiFi config) |
| `/generate_204` | GET | Android captive portal detection |
| `/fwlink` | GET | Windows captive portal detection |
| `/hotspot-detect.html` | GET | iOS/macOS captive portal detection |
| `/ncsi.txt` | GET | Windows NCSI detection |
| `/connecttest.txt` | GET | Windows connect test |

All redirect to `/` (captive portal).