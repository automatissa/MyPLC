// ============================================================================
//  Sample 05 — CTD  (Count Down Counter)
// ============================================================================
//
//  Structured Text equivalent:
//
//    VAR
//      stock    : CTD;
//      pick     : BOOL := FALSE;
//      load_btn : BOOL := FALSE;
//      empty    : BOOL := FALSE;
//      display  : INT  := 0;
//    END_VAR
//
//    stock(CD := pick, LD := load_btn, PV := 5);
//    display := stock.CV;
//    empty   := stock.Q;   (* TRUE when stock reaches 0 *)
//
//  How to build and run:
//    make sample S=05_CTD
// ============================================================================
#include <iostream>
#include <thread>
#include "plc/myplc.h"

using namespace myplc;
using namespace std::chrono_literals;

int main() {
    CTD  stock;
    BOOL pick = false;
    INT  pv   = 5;

    // Load the counter with preset value
    stock.PV(pv);
    stock.LD(true);
    stock.LD(false);

    std::cout << "─── CTD demo: count down from " << pv << " ───\n\n";
    std::cout << "  After LOAD: CV=" << stock.CV() << "  Q=" << stock.Q() << "\n\n";

    for (int i = 1; i <= 7; ++i) {

        // Rising edge
        pick = true;
        stock(pick, false, pv);
        std::this_thread::sleep_for(50ms);

        // Falling edge
        pick = false;
        stock(pick, false, pv);

        std::cout << "  Pick #" << i
                  << "  CV=" << stock.CV()
                  << "  Q=" << stock.Q();
        if (stock.Q()) std::cout << "  ← empty!";
        std::cout << "\n";

        if (i == 6) {
            std::cout << "\n  Reloading stock to " << pv << "...\n";
            stock(false, true, pv);    // LD=TRUE
            stock(false, false, pv);
            std::cout << "  After LOAD: CV=" << stock.CV()
                      << "  Q=" << stock.Q() << "\n\n";
        }
        std::this_thread::sleep_for(200ms);
    }
    std::cout << "\nDone.\n";
}
