// ============================================================================
//  Sample 06 — CTUD  (Count Up / Down Counter)
// ============================================================================
//
//  Structured Text equivalent:
//
//    VAR
//      tank     : CTUD;
//      fill_btn : BOOL := FALSE;
//      drain_btn: BOOL := FALSE;
//      full     : BOOL := FALSE;
//      empty    : BOOL := FALSE;
//      level    : INT  := 0;
//    END_VAR
//
//    tank(CU := fill_btn, CD := drain_btn,
//         R := reset_btn, LD := load_btn, PV := 5);
//    level := tank.CV;
//    full  := tank.QU;   (* CV >= PV *)
//    empty := tank.QD;   (* CV <= 0  *)
//
//  How to build and run:
//    make sample S=06_CTUD
// ============================================================================
#include <iostream>
#include <thread>
#include "plc/myplc.h"

using namespace myplc;
using namespace std::chrono_literals;

// Helper: generate one rising-edge pulse on a CTUD input
static void up_pulse(CTUD& ctu, INT pv) {
    ctu(true,  false, false, false, pv);
    ctu(false, false, false, false, pv);
}

static void down_pulse(CTUD& ctu, INT pv) {
    ctu(false, true,  false, false, pv);
    ctu(false, false, false, false, pv);
}

int main() {
    CTUD tank;
    INT  pv = 5;

    tank.PV(pv);
    std::cout << "─── CTUD demo: fill/drain a tank (limit = " << pv << ") ───\n\n";

    auto print = [&](const char* label) {
        std::cout << "  " << label
                  << "  CV=" << tank.CV()
                  << "  QU=" << tank.QU() << "(full)"
                  << "  QD=" << tank.QD() << "(empty)\n";
    };

    std::cout << "Filling:\n";
    for (int i = 0; i < 7; ++i) {
        up_pulse(tank, pv);
        print("fill");
        std::this_thread::sleep_for(150ms);
    }

    std::cout << "\nDraining:\n";
    for (int i = 0; i < 8; ++i) {
        down_pulse(tank, pv);
        print("drain");
        std::this_thread::sleep_for(150ms);
    }

    std::cout << "\nReset:\n";
    tank(false, false, true, false, pv);   // R=TRUE
    tank(false, false, false, false, pv);
    print("reset");

    std::cout << "\nDone.\n";
}
