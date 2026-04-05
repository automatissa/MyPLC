// ============================================================================
//  Sample 08 — SR and RS  (Set/Reset Bistable Latches)
// ============================================================================
//
//  Structured Text equivalent:
//
//    VAR
//      sr_latch : SR;    (* SET dominant *)
//      rs_latch : RS;    (* RESET dominant *)
//      set_btn  : BOOL := FALSE;
//      rst_btn  : BOOL := FALSE;
//    END_VAR
//
//    sr_latch(S1 := set_btn, R  := rst_btn);
//    rs_latch(S  := set_btn, R1 := rst_btn);
//    (* When both inputs TRUE: *)
//    (*   sr_latch.Q1 = TRUE  (SET wins)   *)
//    (*   rs_latch.Q1 = FALSE (RESET wins) *)
//
//  How to build and run:
//    make sample S=08_SR_RS
// ============================================================================
#include <iostream>
#include <thread>
#include "plc/myplc.h"

using namespace myplc;
using namespace std::chrono_literals;

int main() {
    SR sr;
    RS rs;

    std::cout << "─── SR vs RS latch demo ───\n\n";
    std::cout << "  S1/S │  R/R1 │ SR.Q1 │ RS.Q1 │ Note\n";
    std::cout << "  ─────┼───────┼───────┼───────┼────────────────────────\n";

    auto test = [&](BOOL s, BOOL r, const char* note) {
        sr(s, r);
        rs(s, r);
        std::cout << "    " << s << "  │   " << r
                  << "   │   " << sr.Q1()
                  << "   │   " << rs.Q1()
                  << "   │ " << note << "\n";
        std::this_thread::sleep_for(300ms);
    };

    test(false, false, "idle — both hold previous state");
    test(true,  false, "SET only  → both latches ON");
    test(false, false, "idle — both hold ON");
    test(false, true,  "RESET only → both latches OFF");
    test(false, false, "idle — both hold OFF");
    test(true,  true,  "BOTH active: SR=ON (set wins), RS=OFF (reset wins)");
    test(false, false, "idle — SR stays ON, RS stays OFF");

    std::cout << "\nDone.\n";
}
