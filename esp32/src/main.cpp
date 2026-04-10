// ============================================================================
//  MyPLC — ESP32 Arduino Entry Point
//
//  Wires together:
//    1. WiFi connection
//    2. Modbus TCP server (maps all PLC_VAR to holding registers)
//    3. PLC scan loop (INIT + LOOP from program.cpp)
// ============================================================================

#include <Arduino.h>
#include <WiFi.h>
#include "config/network.h"
#include "mb_server.h"

// Defined in program.cpp
extern void INIT();
extern void LOOP();

static unsigned long _last_cycle = 0;

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("============================================");
    Serial.println("  MyPLC ESP32  |  IEC 61131-3 PLC");
    Serial.println("============================================");

    // ── WiFi ──────────────────────────────────────────────────────────────────
    Serial.printf("[wifi] Connecting to %s ...\n", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\n[wifi] Connected — IP: %s\n", WiFi.localIP().toString().c_str());

    // ── Modbus TCP Server ─────────────────────────────────────────────────────
    myplc::esp32::mb_server_start(MODBUS_PORT);

    // ── PLC Initialisation ────────────────────────────────────────────────────
    INIT();
    Serial.printf("[plc] Scan loop started (cycle = %d ms)\n", SCAN_CYCLE_MS);

    _last_cycle = millis();
}

void loop() {
    // Poll Modbus server (non-blocking — handles one request per call)
    myplc::esp32::mb_server_poll();

    // Run PLC scan cycle at fixed interval
    unsigned long now = millis();
    if (now - _last_cycle >= SCAN_CYCLE_MS) {
        _last_cycle = now;
        LOOP();
    }
}
