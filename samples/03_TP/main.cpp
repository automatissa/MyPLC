// ============================================================================
//  Sample 03 — TP  (Timer Pulse — non-retriggerable one-shot)
// ============================================================================
//
//  Structured Text equivalent:
//
//    VAR
//      pulse  : TP;
//      trigger : BOOL := FALSE;
//      signal  : BOOL := FALSE;
//    END_VAR
//
//    pulse(IN := trigger, PT := T#2s);
//    signal := pulse.Q;
//    (* signal is TRUE for exactly 2 s after trigger rising edge *)
//    (* trigger going FALSE while timing has NO effect *)
//    (* a second trigger while timing is running is IGNORED *)
//
//  How to build and run:
//    make sample S=03_TP
// ============================================================================
#include <iostream>
#include <iomanip>
#include <thread>
#include "plc/myplc.h"

using namespace myplc;
using namespace std::chrono_literals;

int main() {
    TP   pulse;
    BOOL trigger = false;
    BOOL signal  = false;

    std::cout << "─── TP demo: 2 s one-shot pulse, non-retriggerable ───\n\n";

    int  step = 0;
    auto t0   = std::chrono::steady_clock::now();
    auto ms   = [&]() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - t0).count();
    };

    while (step < 6) {
        // ── PLC logic ─────────────────────────────────────────────────────
        pulse(trigger, T(2s));
        signal = pulse.Q();
        // ─────────────────────────────────────────────────────────────────

        std::cout << "\r  trigger=" << trigger
                  << "  ET=" << std::setw(5) << pulse.ET() << " ms"
                  << "  Q=" << pulse.Q()
                  << "  signal=" << signal
                  << "        " << std::flush;

        switch (step) {
            case 0:
                std::cout << "\n[step 1] Trigger → ON, pulse starts\n";
                trigger = true; t0 = std::chrono::steady_clock::now(); step = 1;
                break;
            case 1:
                if (ms() > 500) {
                    std::cout << "\n[step 2] Trigger → OFF after 0.5 s. Pulse should continue...\n";
                    trigger = false; t0 = std::chrono::steady_clock::now(); step = 2;
                }
                break;
            case 2:
                if (ms() > 300) {
                    std::cout << "\n[step 3] Trigger → ON again while pulse running. Should be IGNORED.\n";
                    trigger = true; step = 3;
                }
                break;
            case 3:
                if (!signal) {
                    std::cout << "\n[step 4] Pulse finished naturally after 2 s\n";
                    trigger = false; t0 = std::chrono::steady_clock::now(); step = 4;
                }
                break;
            case 4:
                if (ms() > 200) {
                    std::cout << "\n[step 5] Trigger → ON again (fresh pulse)\n";
                    trigger = true; t0 = std::chrono::steady_clock::now(); step = 5;
                }
                break;
            case 5:
                if (!signal && ms() > 100) {
                    std::cout << "\n\nDone.\n"; step = 6;
                }
                break;
        }
        std::this_thread::sleep_for(10ms);
    }
}
