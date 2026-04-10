#pragma once

// ============================================================================
//  Network Configuration
//  Edit this file to match your deployment.
// ============================================================================

// ── WiFi (ESP32 only) ─────────────────────────────────────────────────────
#define WIFI_SSID   "YourNetwork"
#define WIFI_PASS   "YourPassword"

// ── ESP32 Modbus TCP server ───────────────────────────────────────────────
// Used by the RPi HMI client to reach the ESP32.
// Either set a static IP on the ESP32 router, or check Serial output for DHCP IP.
#define ESP32_IP    "192.168.1.100"
#define MODBUS_PORT  502

// ── Web dashboard ─────────────────────────────────────────────────────────
#define HMI_HTTP_PORT 8080

// ── PLC scan cycle (ms) ───────────────────────────────────────────────────
#define SCAN_CYCLE_MS 10
