// ============================================================================
//  Sample 07 — R_TRIG and F_TRIG  (Rising / Falling Edge Detectors)
// ============================================================================
//
//  Structured Text equivalent:
//
//    VAR
//      re       : R_TRIG;
//      fe       : F_TRIG;
//      button   : BOOL := FALSE;
//      on_press : BOOL := FALSE;   (* TRUE for exactly 1 scan when button pressed *)
//      on_release: BOOL := FALSE;  (* TRUE for exactly 1 scan when button released *)
//    END_VAR
//
//    re(CLK := button);
//    fe(CLK := button);
//    on_press   := re.Q;
//    on_release := fe.Q;
//
//  How to build and run:
//    make sample S=07_R_TRIG_F_TRIG
// ============================================================================
#include <iostream>
#include <iomanip>
#include <thread>
#include "plc/myplc.h"

using namespace myplc;
using namespace std::chrono_literals;

int main() {
    R_TRIG re;
    F_TRIG fe;
    BOOL   button = false;

    std::cout << "─── Edge detector demo: R_TRIG + F_TRIG ───\n\n";
    std::cout << "  (Q is TRUE for exactly ONE scan cycle on each edge)\n\n";

    // Simulate: OFF → ON → ON → OFF → OFF sequence
    struct Step { BOOL sig; const char* label; };
    Step sequence[] = {
        { false, "idle (button=0)" },
        { false, "idle (button=0)" },
        { true,  "PRESS  (button=1)" },   // ← rising edge
        { true,  "hold   (button=1)" },
        { true,  "hold   (button=1)" },
        { false, "RELEASE(button=0)" },   // ← falling edge
        { false, "idle   (button=0)" },
        { false, "idle   (button=0)" },
    };

    std::cout << "  Scan │ button │ R_TRIG.Q │ F_TRIG.Q │ Event\n";
    std::cout << "  ─────┼────────┼──────────┼──────────┼───────────────────\n";

    for (int scan = 0; scan < (int)(sizeof(sequence)/sizeof(sequence[0])); ++scan) {
        button = sequence[scan].sig;

        // ── PLC logic ─────────────────────────────────────────────────────
        re(button);
        fe(button);
        // ─────────────────────────────────────────────────────────────────

        std::cout << "  " << std::setw(4) << scan
                  << " │   " << button
                  << "    │    " << re.Q()
                  << "     │    " << fe.Q()
                  << "     │ " << sequence[scan].label << "\n";

        std::this_thread::sleep_for(200ms);
    }
    std::cout << "\nDone.\n";
}
