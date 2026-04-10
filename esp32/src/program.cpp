// ============================================================================
//  MyPLC — ESP32 User Program
//
//  This file is the ESP32 equivalent of user/program.cpp.
//  PLC_VAR declarations MUST match those in user/program.cpp so that Modbus
//  register addresses align between the ESP32 server and the RPi client.
//
//  Use T_s(), T_ms(), T_min(), T_h() instead of T(5s) — std::chrono literals
//  are not available on Arduino.
// ============================================================================

#include "plc/myplc.h"
#include "sim/registry.h"

using namespace myplc;

// ── Variable Declarations ────────────────────────────────────────────────────
// Must match user/program.cpp exactly (same order, same types)

PLC_VAR(BOOL, start_button, false)
PLC_VAR(BOOL, motor_run,    false)
PLC_VAR(INT,  cycle_time_ms, 0)

myplc::TON delay;

// ── Initialisation ───────────────────────────────────────────────────────────
void INIT() {
    delay.PT(T_s(5));    // 5 seconds — use T_s() on ESP32, not T(5s)
}

// ── Main Scan Loop ───────────────────────────────────────────────────────────
void LOOP() {
    delay(start_button, T_s(5));

    motor_run     = delay.Q();
    cycle_time_ms = static_cast<INT>(delay.ET());
}
