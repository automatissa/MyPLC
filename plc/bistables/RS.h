#pragma once
#include "plc/types.h"

namespace myplc {

// ============================================================================
//  RS — Reset Dominant Bistable (Latch)
// ============================================================================
//  Structured Text equivalent:
//
//    myLatch : RS;
//    myLatch(S := set_btn, R1 := reset_btn);
//    motor := myLatch.Q1;
//
//  Truth table  (R1 dominant — when both inputs TRUE, output goes FALSE):
//    S   R1  Q1
//    0   0   Q1 (holds)
//    0   1   0
//    1   0   1
//    1   1   0  ← R1 wins
//
//  Boolean:  Q1 = (S OR Q1) AND NOT R1
// ============================================================================
class RS {
    BOOL _Q1 = false;
public:
    void operator()(BOOL s, BOOL r1);
    BOOL Q1() const { return _Q1; }
};

}  // namespace myplc
