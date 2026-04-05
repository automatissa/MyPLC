// ============================================================================
//  MyPLC — User Program
//  Edit this file to write your PLC program.
//
//  Structure mirrors IEC 61131-3 Structured Text (ST):
//
//    ┌──────────────────────────────────────────────┐
//    │  PROGRAM Main                                │
//    │    VAR                                       │
//    │      start_button : BOOL := FALSE;           │
//    │      motor_run    : BOOL := FALSE;           │
//    │      cycle_time   : INT  := 0;               │
//    │      delay        : TON;                     │
//    │    END_VAR                                   │
//    │                                              │
//    │    delay(IN := start_button, PT := T#5s);    │
//    │    motor_run  := delay.Q;                    │
//    │    cycle_time := delay.ET;                   │
//    │  END_PROGRAM                                 │
//    └──────────────────────────────────────────────┘
//
//  In C++ with MyPLC:
//
//    PLC_VAR(BOOL, start_button, false)  ← mirrors  start_button : BOOL
//    myplc::TON delay;                   ← mirrors  delay : TON
//
//    void LOOP() {
//        delay(start_button, T(5s));     ← same call style as ST
//        motor_run  = delay.Q();
//        cycle_time = delay.ET();
//    }
//
//  Open http://localhost:8080 after running `make run` to see the dashboard.
// ============================================================================

#include "plc/myplc.h"
#include "sim/registry.h"

using namespace myplc;

// ── Variable Declarations ────────────────────────────────────────────────────
// PLC_VAR(TYPE, name, initial_value)
//   → declares the global variable
//   → registers it with the web dashboard (read + write)

PLC_VAR(BOOL, start_button, false)   // Simulated input  — toggle in dashboard
PLC_VAR(BOOL, motor_run,    false)   // Simulated output — visible in dashboard
PLC_VAR(INT,  cycle_time_ms, 0)      // Elapsed timer value (ms)

// Function block instances are declared normally (they're not simple values).
myplc::TON delay;

// ── Initialisation ───────────────────────────────────────────────────────────
// Called once before the scan loop starts.
void INIT() {
    delay.PT(T(5s));     // preset: 5 seconds
}

// ── Main Scan Loop ───────────────────────────────────────────────────────────
// Called every 10 ms by the runtime harness.
void LOOP() {

    // Timer On Delay: motor starts 5 s after start_button is pressed.
    delay(start_button, T(5s));

    motor_run    = delay.Q();    // output follows timer output
    cycle_time_ms = delay.ET();  // expose elapsed time to dashboard
}
