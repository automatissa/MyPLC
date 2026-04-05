// ============================================================================
//  Sample 01 — TON  (Timer On Delay)
// ============================================================================
//
//  Structured Text equivalent:
//
//    VAR
//      delay : TON;
//      sensor : BOOL := FALSE;
//      lamp   : BOOL := FALSE;
//    END_VAR
//
//    delay(IN := sensor, PT := T#3s);
//    lamp := delay.Q;
//    (* lamp turns ON 3 s after sensor becomes TRUE *)
//    (* lamp turns OFF immediately when sensor goes FALSE *)
//
//  How to build and run:
//    make sample S=01_TON
// ============================================================================
#include <iostream>
#include <iomanip>
#include <thread>
#include "plc/myplc.h"

using namespace myplc;
using namespace std::chrono_literals;

int main() {
    TON  delay;
    BOOL sensor = false;
    BOOL lamp   = false;

    std::cout << "─── TON demo: lamp turns ON 3 s after sensor, OFF when sensor released ───\n\n";

    // Step 0 → 1: sensor ON, wait for lamp
    // Step 1 → 2: sensor OFF, lamp should reset immediately
    // Step 2    : done
    int step = 0;
    auto step_start = std::chrono::steady_clock::now();

    auto elapsed_ms = [&]() -> long {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - step_start).count();
    };

    while (step < 3) {
        // ── PLC logic (mirrors ST body) ───────────────────────────────────
        delay(sensor, T(3s));
        lamp = delay.Q();
        // ─────────────────────────────────────────────────────────────────

        std::cout << "\r  sensor=" << sensor
                  << "  ET=" << std::setw(5) << delay.ET() << " ms"
                  << "  Q=" << delay.Q()
                  << "  lamp=" << lamp
                  << "        " << std::flush;

        switch (step) {
            case 0:
                std::cout << "\n[step 1] Sensor → ON. Waiting 3 s...\n";
                sensor = true;
                step_start = std::chrono::steady_clock::now();
                step = 1;
                break;
            case 1:
                if (lamp) {
                    std::cout << "\n[step 2] Lamp ON after " << elapsed_ms() << " ms. Releasing sensor.\n";
                    sensor = false;
                    step_start = std::chrono::steady_clock::now();
                    step = 2;
                }
                break;
            case 2:
                std::cout << "\r  sensor=" << sensor
                          << "  ET=" << std::setw(5) << delay.ET() << " ms"
                          << "  Q=" << delay.Q()
                          << "  lamp=" << lamp << "  ← reset immediately  " << std::flush;
                if (elapsed_ms() > 500) {
                    std::cout << "\n\nDone.\n";
                    step = 3;
                }
                break;
        }
        std::this_thread::sleep_for(10ms);
    }
}
