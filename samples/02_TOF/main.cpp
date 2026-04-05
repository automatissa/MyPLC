// ============================================================================
//  Sample 02 — TOF  (Timer Off Delay)
// ============================================================================
//
//  Structured Text equivalent:
//
//    VAR
//      hold  : TOF;
//      run   : BOOL := FALSE;
//      fan   : BOOL := FALSE;
//    END_VAR
//
//    hold(IN := run, PT := T#3s);
//    fan := hold.Q;
//    (* fan is ON while run is ON *)
//    (* fan stays ON 3 s after run goes FALSE, then turns OFF *)
//
//  How to build and run:
//    make sample S=02_TOF
// ============================================================================
#include <iostream>
#include <iomanip>
#include <thread>
#include "plc/myplc.h"

using namespace myplc;
using namespace std::chrono_literals;

int main() {
    TOF  hold;
    BOOL run = false;
    BOOL fan = false;

    std::cout << "─── TOF demo: fan runs while signal ON, then stays on 3 s after OFF ───\n\n";

    int  step = 0;
    auto t0   = std::chrono::steady_clock::now();
    auto ms   = [&]() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - t0).count();
    };

    while (step < 4) {
        // ── PLC logic ─────────────────────────────────────────────────────
        hold(run, T(3s));
        fan = hold.Q();
        // ─────────────────────────────────────────────────────────────────

        std::cout << "\r  run=" << run
                  << "  ET=" << std::setw(5) << hold.ET() << " ms"
                  << "  Q=" << hold.Q()
                  << "  fan=" << fan
                  << "        " << std::flush;

        switch (step) {
            case 0:
                std::cout << "\n[step 1] Signal → ON\n";
                run = true; t0 = std::chrono::steady_clock::now(); step = 1;
                break;
            case 1:
                if (ms() > 1500) {
                    std::cout << "\n[step 2] Signal → OFF. Fan should hold for 3 s...\n";
                    run = false; t0 = std::chrono::steady_clock::now(); step = 2;
                }
                break;
            case 2:
                if (!fan) {
                    std::cout << "\n[step 3] Fan OFF after " << ms() << " ms\n";
                    t0 = std::chrono::steady_clock::now(); step = 3;
                }
                break;
            case 3:
                if (ms() > 300) { std::cout << "\nDone.\n"; step = 4; }
                break;
        }
        std::this_thread::sleep_for(10ms);
    }
}
