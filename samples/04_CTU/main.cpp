// ============================================================================
//  Sample 04 — CTU  (Count Up Counter)
// ============================================================================
//
//  Structured Text equivalent:
//
//    VAR
//      parts   : CTU;
//      sensor  : BOOL := FALSE;
//      display : INT  := 0;
//      full    : BOOL := FALSE;
//    END_VAR
//
//    parts(CU := sensor, R := reset_btn, PV := 5);
//    display := parts.CV;
//    full    := parts.Q;   (* TRUE when 5 parts counted *)
//
//  How to build and run:
//    make sample S=04_CTU
// ============================================================================
#include <iostream>
#include <thread>
#include "plc/myplc.h"

using namespace myplc;
using namespace std::chrono_literals;

int main() {
    CTU  parts;
    BOOL sensor = false;
    INT  pv     = 5;

    parts.PV(pv);
    std::cout << "─── CTU demo: count up to " << pv << " ───\n\n";

    // Simulate 7 pulses (one per loop iteration)
    for (int i = 1; i <= 7; ++i) {

        // Rising edge of sensor
        sensor = true;
        parts(sensor, false, pv);
        std::this_thread::sleep_for(50ms);

        // Falling edge
        sensor = false;
        parts(sensor, false, pv);

        std::cout << "  Pulse #" << i
                  << "  CV=" << parts.CV()
                  << "  Q=" << parts.Q();
        if (parts.Q()) std::cout << "  ← target reached!";
        std::cout << "\n";

        if (i == 6) {
            std::cout << "\n  Resetting...\n";
            parts(false, true, pv);    // R=TRUE resets CV, keeps PV
            parts(false, false, pv);   // R back to FALSE
            std::cout << "  After reset: CV=" << parts.CV()
                      << "  Q=" << parts.Q() << "\n\n";
        }
        std::this_thread::sleep_for(200ms);
    }
    std::cout << "\nDone.\n";
}
