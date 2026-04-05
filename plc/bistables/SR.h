#pragma once
#include "plc/types.h"

namespace myplc {

// ============================================================================
//  SR — Set Dominant Bistable (Latch)
// ============================================================================
//  Structured Text equivalent:
//
//    myLatch : SR;
//    myLatch(S1 := set_btn, R := reset_btn);
//    motor := myLatch.Q1;
//
//  Truth table  (S1 dominant — when both inputs TRUE, output stays TRUE):
//    S1  R   Q1
//    0   0   Q1 (holds)
//    0   1   0
//    1   0   1
//    1   1   1  ← S1 wins
//
//  Boolean:  Q1 = S1 OR (Q1 AND NOT R)
// ============================================================================
class SR {
    BOOL _Q1 = false;
public:
    void operator()(BOOL s1, BOOL r);
    BOOL Q1() const { return _Q1; }
};

}  // namespace myplc
